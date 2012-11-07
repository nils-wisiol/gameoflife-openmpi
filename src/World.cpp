#include <vector>

using namespace std;

#define TYPE bool
#define DEFAULT false

/**
 * represents a game of life state
 */
class World {
protected:
  /**
   * status of the mesh
   */
  TYPE** mesh;

  /**
   * width and height of the mesh
   */
  unsigned long width, height;

  /**
   * cout of alive cells
   */
  unsigned long alive;

  /**
   * all the worlds in this instance
   */
  static vector<World*> worlds;

public:
  /**
   * create a new world of game of life
   * initializes the game with all cells DEFAULT
   */
  World(unsigned long w, unsigned long h) {
    width = w;
    height = h;
    mesh = new TYPE*[width];
    for(unsigned long i = 0; i < width; i++) {
      mesh[i] = new TYPE[height];
      for(unsigned long j = 0; j < height; j++) {
        mesh[i] = DEFAULT;
      }
    }
    worlds.push_back(this);
  }

  /**
   * sets the cell at (x,y) to the given state
   * returns the count of alive cells
   */
  unsigned long set(unsigned long x, unsigned long y, TYPE state) {
    mesh[x][y] = state;
    return state ? ++alive : --alive;
  }
};
