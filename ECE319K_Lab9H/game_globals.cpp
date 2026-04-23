#include "game_globals.h"

bool gameOver = false;
bool isShooting = false;
int shotType = -1;
int ammo = AMMO_LIMIT;
int hearts = HEART_LIMIT;
int score = 0;
bool isEnglish = true;
int spawn_cooldown = START_SPAWN_COOLDOWN;
int spawn_timer = START_SPAWN_COOLDOWN;
int total_enemies = 0;
