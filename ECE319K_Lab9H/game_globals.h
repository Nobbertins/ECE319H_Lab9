#ifndef game_globals
#define game_globals

#define AMMO_LIMIT 8
#define HEART_LIMIT 5
#define SHOOT_HOLD 4 //in frames
#define SHOOT_COOLDOWN 10 //must be geq than hold
#define RELOAD_HOLD 10

extern bool isShooting;
extern int ammo;
extern int hearts;
extern int score;
extern bool isEnglish;

#endif