#include "World.cpp"

using namespace std;

class Engine {
public:
  virtual void step(World* w, long steps) = 0;
};
