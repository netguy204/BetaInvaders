#define JOYX A8
#define JOYY A5
#define BUTTON 1
#define LIGHT A9

#define MAX_ENEMIES 15
#define MAX_BULLETS 15

#define SCREEN_WIDTH 8
#define SCREEN_HEIGHT 8

#define LIVEZONE 256
#define COOLTIME 10

#define GAME_INIT 0
#define GAME_PLAY 1
#define GAME_LOSE 2
#define GAME_WIN 3

#include "invader_types.h"
#include <LedControl.h>

LedControl lc = LedControl(10,11,12,1);

Enemy enemies[MAX_ENEMIES];
Bullet bullets[MAX_BULLETS];
int live_enemies;
int live_bullets;
int init_enemies;
Player player;
int game_state = GAME_INIT;

bool above(Vector* a, Vector* b) {
  return a->x == b->x && a->y > b->y;
}

bool same(Vector* a, Vector* b) {
  return a->x == b->x && a->y == b->y;
}

Enemy* init_enemy(int x, int y) {
  if(live_enemies == MAX_ENEMIES) return NULL;
  
  Enemy* enemy = &enemies[live_enemies++];
  enemy->pos.x = x;
  enemy->pos.y = y;
  enemy->state = 0;
  enemy->cooldown = COOLTIME;
  return enemy;
}

void free_enemy(Enemy* e) {
  int index = (int)(e - enemies);
  for(int ii = index + 1; ii < live_enemies; ++ii) {
    enemies[ii-1] = enemies[ii];
  }
  live_enemies--;
}

Bullet* init_bullet(int x, int y) {
  if(live_bullets == MAX_BULLETS) return NULL;
  
  Bullet* bullet = &bullets[live_bullets++];
  bullet->pos.x = x;
  bullet->pos.y = y;
  bullet->enemy = 0;
  return bullet;
}

void free_bullet(Bullet* b) {
  int index = (int)(b - bullets);
  for(int ii = index + 1; ii < live_bullets; ++ii) {
    bullets[ii-1] = bullets[ii];
  }
  live_bullets--;
}

void fire_enemy(Enemy* enemy) {
  if(enemy->cooldown > 0) return; // wait for the cooldown period before firing
  if(live_bullets >= MAX_BULLETS - 1) return; // don't fire if no memory is available
  if(enemy->pos.y == 0) return; // don't fire from the bottom of the screen
  
  // always fire down
  Bullet* b = init_bullet(enemy->pos.x, enemy->pos.y - 1);
  b->enemy = 1;
  enemy->cooldown = COOLTIME;
}

void fire_player() {
  Bullet* b = init_bullet(player.pos.x, player.pos.y + 1);
  if(!b) return;
}

void update_enemy(Enemy* enemy) {
  if(above(&enemy->pos, &player.pos)) {
    fire_enemy(enemy);
  }
  if(enemy->state == 0) {
    if(enemy->pos.x < SCREEN_WIDTH - 1) {
      enemy->pos.x++;
    } else {
      enemy->state = 1;
    }
  } else {
    if(enemy->pos.x > 0) {
      enemy->pos.x--;
    } else {
      enemy->state = 0;
    }
  }   
  
  if(enemy->cooldown > 0) {
    enemy->cooldown--;
  }
}

void update_bullet(Bullet* bullet) {
  if(bullet->enemy) {
    bullet->pos.y -= 1;
    if(bullet->pos.y < 0) {
      free_bullet(bullet);
      return;
    }
    if(same(&bullet->pos, &player.pos)) {
      game_state = GAME_LOSE;
    }
  } else {
    bullet->pos.y += 1;
    if(bullet->pos.y >= SCREEN_HEIGHT) {
      free_bullet(bullet);
      return;
    }
    // player bullets cheat slightly so that the player feels more powerful
    Vector above;
    above.x = bullet->pos.x;
    above.y = bullet->pos.y + 1;
    for(int ii = 0; ii < live_enemies; ++ii) {
      if(same(&enemies[ii].pos, &bullet->pos) || same(&enemies[ii].pos, &above)) {
        free_enemy(&enemies[ii]);
        free_bullet(bullet);
        if(live_enemies == 0) {
          game_state = GAME_WIN;
        }
        return;
      }
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  init_enemies = 2;
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
}

int update_state = 0;
void loop() {
  if(game_state == GAME_INIT) {
    live_enemies = 0;
    live_bullets = 0;
    for(int i = 0; i < init_enemies; ++i) {
      int y = SCREEN_HEIGHT - 1 - i*2 / SCREEN_WIDTH;
      init_enemy(i*2 % SCREEN_WIDTH, y);
    }
    
    player.pos.x = SCREEN_WIDTH/2;
    player.pos.y = 0;
    player.state = 0;
    game_state = GAME_PLAY;
  } else if(game_state == GAME_PLAY) {
    lc.clearDisplay(0);
    
    long start = millis();
    
    // glitch, one object is skipped when a bullet or enemy is removed by the update function
    // update bullets and enemies once every other frame
    for(int ii = 0; ii < live_bullets; ++ii) {
      if(!&bullets[ii].enemy || update_state & 1 == 1) update_bullet(&bullets[ii]);
      lc.setLed(0, SCREEN_HEIGHT - 1 - bullets[ii].pos.y, bullets[ii].pos.x, true);
    }
    for(int ii = 0; ii < live_enemies; ++ii) {
      if(update_state & 1 == 1) update_enemy(&enemies[ii]);
      lc.setLed(0, SCREEN_HEIGHT - 1 - enemies[ii].pos.y, enemies[ii].pos.x, true);
    }
    
    // process player direction inputs
    long dx = analogRead(JOYX);
    long dy = analogRead(JOYY);
    if(dx < LIVEZONE) {
      player.pos.x += 1;
    } else if(dx > 1023 - LIVEZONE) {
      player.pos.x -= 1;
    }
    if(player.pos.x < 0) player.pos.x = 0;
    if(player.pos.x >= SCREEN_WIDTH) player.pos.x = SCREEN_WIDTH - 1;
    
    // fire only on a button press edge
    int fire = !digitalRead(BUTTON);
    if(fire && player.state == 0) {
      fire_player();
      player.state = 1;
    } else if(!fire) {
      player.state = 0;
    }
    
    lc.setLed(0, SCREEN_HEIGHT - 1 - player.pos.y, player.pos.x, true);
    
    long dt = millis() - start;
    delay(max(0, 80 - dt));
    ++update_state;
  } else if(game_state == GAME_LOSE) {
    lc.clearDisplay(0);
    for(int ii = 0; ii < SCREEN_WIDTH; ++ii) {
      lc.setLed(0, ii, ii, true);
      lc.setLed(0, SCREEN_HEIGHT - 1 -ii, ii, true);
      delay(200);
    }
    delay(300);
    init_enemies = 2;
    
    game_state = GAME_INIT;
  } else if(game_state == GAME_WIN) {
    lc.clearDisplay(0);
    for(int ii = 0; ii < SCREEN_WIDTH; ++ii) {
      for(int jj = 0; jj < SCREEN_HEIGHT; ++jj) {
        lc.setLed(0, ii, jj, true);
        delay(30);
      }
    }
    
    // increase the number of enemies up to the maximum
    if(init_enemies == MAX_ENEMIES) {
      init_enemies = 2;
    } else {
      init_enemies++;
    }
    game_state = GAME_INIT;
  }
}
