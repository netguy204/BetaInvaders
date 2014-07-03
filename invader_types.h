#include <stdint.h>

struct Vector {
  int8_t x;
  int8_t y;
};

struct Enemy {
  Vector pos;
  char state : 2;
  char cooldown : 6;
};

struct Player {
  Vector pos;
  uint8_t state;
};

struct Bullet {
  Vector pos;
  uint8_t enemy;
};
