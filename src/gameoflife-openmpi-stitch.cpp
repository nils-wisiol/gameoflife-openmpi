#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>

#define TYPE int
#define MSG_PRINT 10
#define MSG_FROM_BENEATH 11
#define MSG_FROM_ABOVE 12

using namespace std;

int numprocs, rank, namelen;
unsigned long xoffset, yoffset, width, height;
struct config {
  unsigned long width;
  unsigned long height;
  unsigned long steps;
  bool quiet;
} config;
TYPE** world, **buffer;

void readConfig(int argc, char *argv[], struct config* c) {
  c->quiet = false;
  char ch;
  while ((ch = getopt(argc, argv, "h:w:s:q")) != -1) {
    switch (ch) {
    case 'w':
      c->width = strtol(optarg, NULL, 10);
      break;
    case 'h':
      c->height = strtol(optarg, NULL, 10);
      break;
    case 's':
      c->steps = strtol(optarg, NULL, 10);
      break;
    case 'q':
      c->quiet = true;
      break;
    }
  }
}

void print() {
  if (rank != 0) {
    MPI_Recv(NULL, /* message buffer */
        0, /* one data item */
        MPI_INT, /* of type double real */
        MPI_ANY_SOURCE, /* receive from any sender */
        MSG_PRINT, /* any type of message */
        MPI_COMM_WORLD, /* default communicator */
        NULL);
  }

  for (unsigned long j = 0; j < height; j++) {
    printf("%d: ", rank);
    for (unsigned long i = 0; i < width; i++) {
      printf(world[j][i] ? "#" : ".");
    }
    printf("\n");
  }

  if (rank != numprocs - 1) {
    MPI_Send(NULL, /* message buffer */
        0, /* one data item */
        MPI_INT, /* data item is an integer */
        rank + 1, /* destination process rank */
        MSG_PRINT, /* user chosen message tag */
        MPI_COMM_WORLD); /* default communicator */
  } else {
    printf("\n\n");
  }
}

void printMsg(TYPE* msg, int width, char* direction) {
  printf("process' %d %s msg: ", rank, direction);
  for (unsigned long i = 0; i < width; i++) {
    printf("%d ", msg[i]);
  }
  printf("\n");
}

void set(unsigned long glY, unsigned long glX, TYPE val) {
  // check for boundaries
  if (xoffset <= glX && glX < xoffset + width
      && yoffset <= glY && glY < yoffset + height) {
    world[glY - yoffset][glX - xoffset] = val;
  }
}

int main(int argc, char *argv[]) {

  //if (rank==0)
    //printf("sizeof(TYPE): %d\n", (int)sizeof(TYPE));

  // gather MPI process data
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  //printf("Process %d on %s out of %d\n", rank, processor_name, numprocs);

  // gather global configuration
  struct config config;
  readConfig(argc, argv, &config);

  // calculate process specific data
  int above = (rank + numprocs - 1) % numprocs;
  int beneath = (rank + 1) % numprocs;
  //printf("Process %d has above %d and beneath %d\n", rank, above, beneath);
  xoffset = 0;
  yoffset = config.height / numprocs * rank;
  width = config.width;
  height = config.height / numprocs;
  if (!config.quiet)
    printf("Process %d has range %d,%d X %d,%d\n", rank, xoffset, yoffset,
        xoffset + width, yoffset + height);

  // initialize world and buffer
  world = new TYPE*[height];
  buffer = new TYPE*[height];
  for (unsigned long j = 0; j < height; j++) {
    buffer[j] = new TYPE[width];
    world[j] = new TYPE[width];
    for (unsigned long i = 0; i < width; i++) {
      world[j][i] = 0;
      buffer[j][i] = 0;
    }
  }

  // set f-pentomino
  // X##
  // ##
  //  #
  // in the middle of the field
  // such that X marks cell floor(config.height/2), floor(config.width/2)
  //// first row
  set(config.height/2, config.height/2 + 1, 1);
  set(config.height/2, config.height/2 + 2, 1);
  //// second row
  set(config.height/2 + 1, config.height/2, 1);
  set(config.height/2 + 1, config.height/2 + 1, 1);
  //// third row
  set(config.height/2 + 2, config.height/2 + 1, 1);

  // calculate!
  TYPE* sentFromAbove = new TYPE[width];
  TYPE* sentFromBeneath = new TYPE[width];
  for (unsigned long g = 0; g < config.steps; g++) {
    // send last generations results to the neighbours
    TYPE* sendToAbove = world[0];
    TYPE* sendToBeneath = world[height - 1];
    MPI_Request toAbove, toBeneath;
    MPI_Isend(sendToAbove, width, MPI_INT, above, MSG_FROM_BENEATH, MPI_COMM_WORLD, &toAbove);
    MPI_Isend(sendToBeneath, width, MPI_INT, beneath, MSG_FROM_ABOVE, MPI_COMM_WORLD, &toBeneath);
    //printMsg(sendToAbove, width, "sent to above");
    //printMsg(sendToBeneath, width, "sent to beneath");

    // receive last generations results from the neighbours
    MPI_Request fromAbove, fromBeneath;
    MPI_Irecv(sentFromAbove, width, MPI_INT, above, MSG_FROM_ABOVE, MPI_COMM_WORLD, &fromAbove);
    MPI_Irecv(sentFromBeneath, width, MPI_INT, beneath, MSG_FROM_BENEATH, MPI_COMM_WORLD, &fromBeneath);
    MPI_Wait(&fromAbove, NULL);
    MPI_Wait(&fromBeneath, NULL);
    //printMsg(sentFromAbove, width, "received from above");
    //printMsg(sentFromBeneath, width, "received from beneath");

    // calculate next generation
    //// first row (j = 0)
    for (unsigned long i = 0; i < width; i++) {
      // count neighbourhood
      int c = sentFromAbove[(i + width - 1) % width] // row above
                + sentFromAbove[(i + width - 0) % width]
                + sentFromAbove[(i + width + 1) % width]
            + world[0][(i + width - 1) % width] // row 0
                + world[0][(i + width + 1) % width]
            + world[1][(i + width - 1) % width] // row 1
                + world[1][(i + width + 0) % width]
                + world[1][(i + width + 1) % width];
      buffer[0][i] = world[0][i] ? (c == 2 || c == 3) : c == 3;
    }
    //// middle rows
    for (unsigned long j = 1; j < height - 1; j++) {
      for (unsigned long i = 0; i < width; i++) {
        // count neighbourhood
        int c = world[j-1][(i + width - 1) % width] // row j-1
                  + world[j-1][(i + width + 0) % width]
                  + world[j-1][(i + width + 1) % width]
              + world[j][(i + width - 1) % width] // row j
                  + world[j][(i + width + 1) % width]
              + world[j+1][(i + width - 1) % width] // row j+1
                  + world[j+1][(i + width + 0) % width]
                  + world[j+1][(i + width + 1) % width];
        buffer[j][i] = world[j][i] ? (c == 2 || c == 3) : c == 3;
      }
    }
    //// last row (j = height - 1)
    for (unsigned long i = 0; i < width; i++) {
      // count neighbourhood
      int c = world[height - 2][(i + width - 1) % width] // row height - 2
                + world[height - 2][(i + width - 0) % width]
                + world[height - 2][(i + width + 1) % width]
            + world[height - 1][(i + width - 1) % width] // last row (height - 1)
                + world[height - 1][(i + width + 1) % width]
            + sentFromBeneath[(i + width - 1) % width] // row beneath
                + sentFromBeneath[(i + width + 0) % width]
                + sentFromBeneath[(i + width + 1) % width];
      buffer[height - 1][i] = world[height - 1][i] ? (c == 2 || c == 3) : c == 3;
    }

    // swap world/buffer
    TYPE** swap = world;
    world = buffer;
    buffer = swap;
  }

  // output
  if (!config.quiet)
    print();

  MPI_Finalize();
}
