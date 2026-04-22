#ifndef game_globals
#define game_globals

#define AMMO_LIMIT 8
#define HEART_LIMIT 5
#define SHOOT_HOLD 4 //in frames
#define SHOOT_COOLDOWN 7 //must be geq than hold
#define RELOAD_HOLD 7
#define START_SPAWN_COOLDOWN 100

extern bool isShooting;
extern int ammo;
extern int hearts;
extern int score;
extern bool isEnglish;
extern int spawn_cooldown;
extern int spawn_timer;

#endif