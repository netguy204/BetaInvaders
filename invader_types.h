
struct Vector {
  int x;
  int y;
};

struct Enemy {
  Vector pos;
  int state;
  int cooldown;
};

struct Player {
  Vector pos;
  int state;
};

struct Bullet {
  Vector pos;
  int enemy;
};
