#ifndef game_globals
#define game_globals

#define AMMO_LIMIT 8
#define HEART_LIMIT 5
#define SHOOT_HOLD 4 //in frames
#define SHOOT_COOLDOWN 6 //must be geq than hold
#define RELOAD_HOLD 5
#define START_SPAWN_COOLDOWN 50
#define MAX_ENEMIES 6 //max allowed bc of small heap lmao (malloc actually fails)
#define ENEMY_ATTACK_RANGE 5 //in world units
#define ENEMY_SPEED 1.5 //multiplier in world units

extern bool gameOver;
extern bool isShooting;
extern int shotType;
extern int ammo;
extern int hearts;
extern int score;
extern bool isEnglish;
extern int spawn_cooldown;
extern int spawn_timer;
extern int total_enemies;

#endif