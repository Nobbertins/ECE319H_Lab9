#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
#include "../inc/ST7735.h"

#define screenWidth 128
#define screenHeight 160
#define mapHeight 10
#define mapWidth 8
#define squareLength 16
#define FOV 70
#define cameraWidth 1.4 //arctan(FOV/2) * 2

#define blue16 ST7735_Color565(27, 161, 234)
#define yellow16 ST7735_Color565(255, 242, 0)
#define red16 ST7735_Color565(245, 97, 92)
#define green16 ST7735_Color565(62, 170, 13)
#define empty16 0
#define playerColor ST7735_Color565(255, 255, 255)

//long side is y direction (height)
const uint16_t gridmap[mapHeight][mapWidth] = {
{blue16, blue16, blue16, blue16, blue16, blue16, blue16, blue16},
{blue16, empty16, empty16, empty16, empty16, empty16, empty16, blue16},
{blue16, red16, empty16, empty16, yellow16, empty16, empty16, blue16},
{blue16, empty16, empty16, empty16, empty16, empty16, empty16, blue16},
{blue16, red16, empty16, empty16, empty16, empty16, empty16, blue16},
{blue16, empty16, empty16, empty16, empty16, green16, empty16, blue16},
{blue16, empty16, empty16, empty16, empty16, empty16, empty16, blue16},
{blue16, empty16, empty16, empty16, empty16, empty16, empty16, blue16},
{blue16, empty16, empty16, empty16, empty16, empty16, empty16, blue16},
{blue16, blue16, blue16, blue16, blue16, blue16, blue16, blue16}
};

struct Pos{
    int x;
    int y;
};

struct Vector2D{
    float x;
    float y;
};


extern Pos camera;
extern Pos cameraMap;
extern Vector2D cameraDirection;

inline Vector2D getPerp(Vector2D v){
    return {-v.y, v.x};
}

//returns normalized raycast direction for x'th pixel on camera screen (from left to right)
Vector2D findRaycastDirection(uint16_t x);

void drawTopDown(void);

void drawPlayer(void);

float drawRaycast(Vector2D r, uint16_t color);

void drawRaycasts(Vector2D facing, uint16_t color);

#endif