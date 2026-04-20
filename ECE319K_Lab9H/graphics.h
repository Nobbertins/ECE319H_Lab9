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
#define cameraWidth 1.4 //tan(FOV/2) * 2
#define maxTurnSpeed 5
#define maxMoveSpeed 2
#define cameraClippingRadius 4
#define UI_HEIGHT 15

struct wallColor{
    const uint16_t color;
    const uint16_t light_color;
};

const wallColor blue16 = {ST7735_Color565(3, 73, 252), ST7735_Color565(84, 133, 255)};
const wallColor yellow16 = {ST7735_Color565(252, 235, 52), ST7735_Color565(253, 240, 136)};
const wallColor red16 = {ST7735_Color565(242, 71, 65), ST7735_Color565(247, 107, 103)};
const wallColor green16 = {ST7735_Color565(62, 170, 13), ST7735_Color565(137, 186, 114)};
const wallColor empty16 = {ST7735_Color565(255, 0, 255), ST7735_Color565(255, 0, 255)};//magenta = transparent or empty

#define playerColor ST7735_Color565(255, 255, 255)
#define ceilingColor ST7735_Color565(68, 68, 68)
#define floorColor ST7735_Color565(122, 122, 122)
#define heartColor ST7735_Color565(255, 0, 0)
#define ammoColor ST7735_Color565(255, 148, 7)

//long side is y direction (height)
const wallColor gridmap[mapHeight][mapWidth] = {
{blue16, blue16, blue16, blue16, blue16, blue16, blue16, blue16},
{blue16, empty16, empty16, empty16, empty16, empty16, empty16, blue16},
{blue16, red16, empty16, empty16, yellow16, empty16, empty16, blue16},
{blue16, empty16, empty16, empty16, empty16, empty16, empty16, blue16},
{blue16, red16, empty16, empty16, empty16, yellow16, empty16, blue16},
{blue16, empty16, empty16, green16, empty16, green16, empty16, blue16},
{blue16, empty16, red16, empty16, empty16, empty16, empty16, blue16},
{blue16, empty16, empty16, yellow16, empty16, red16, empty16, blue16},
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

struct Wall{
    float dist;
    uint16_t color;
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

void drawPlayer(void);//for top down

void drawCrosshair(void);

Wall drawRaycast(Vector2D r, uint16_t color);

void drawRaycasts(Vector2D facing);

void renderColumn(int col, Wall w);

void renderBufferedColumn(int col, Wall w);

bool moveCamera(Vector2D j);

#endif