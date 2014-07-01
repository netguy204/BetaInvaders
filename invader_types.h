
struct Vector {
  int x;
  int y;
};

struct Enemy {
  Vector pos;
  int state : 2;
  int cooldown : 6;
};

struct Player {
  Vector pos;
  int state;
};

struct Bullet {
  Vector pos;
  int enemy;
};
