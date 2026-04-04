#include "graphics.h"

//global variable definitions
Pos camera = {70, 85};
Pos cameraMap = {4, 5};
Vector2D cameraDirection = {0.0, 1.0};


//function definitions

//draws the 10x8 gridmap of 16x16 squares onto the screen (top-down view)
void drawTopDown(void){
    //draws lines, less SPI calls ~70ms render time (14Hz)
    //could use setAddrWindow directly to optimize further by rendering squares instead of lines
    //but this was mostly just a performance test for raycasting later anyway
    for(int r = 0; r < mapHeight; r++){
        for(int c = 0; c < mapWidth; c++){
            for(int16_t line = 0; line < squareLength; line++){
            ST7735_DrawFastVLine(c * squareLength + line, r * squareLength, squareLength, gridmap[r][c]);
            }
        }
    }
    // pixel by pixel naive approach (too many SPI calls, >200ms render time)
    // for(int r = 0; r < 10; r++){
    //     for(int c = 0; c < 8; c++){
    //         for(int16_t i = c * 16; i < c * 16 + 16; i++){
    //             for(int16_t j = r * 16; j < r * 16 + 16; j++){
    //                 ST7735_DrawPixel(i, j, gridmap[r][c]);
    //             }
    //         }
    //     }
    // }
}

void drawPlayer(void){
    ST7735_DrawPixel(camera.x, camera.y, playerColor);
}

//draws raycast from camera with raycast direction r
void drawRaycast(Vector2D r, uint16_t color){
    float distance = 0.0;
    Pos rayMap = {cameraMap.x, cameraMap.y};
    Vector2D rem = {(float)(camera.x - cameraMap.x * squareLength), (float)(camera.y - cameraMap.y * squareLength)};
    while(gridmap[rayMap.y][rayMap.x] == empty16){
        Vector2D dt = {10e30, 10e30}; //assume worst for == 0 case
        Pos ind = {0, 0}; //map square index incrementer

        //calculate dt
        //4 possibilities of what we hit first:
        // We cross xrem=0 if r^x<0, at dt=−xrem/r^x
        // We cross xrem=wTILE if r^x>0, at dt=(wTILE–xrem)/r^x
        // We cross yrem=0 if r^y<0, at dt=−yrem/r^y
        // We cross yrem=wTILE if r^y>0, at dt=(wTILE–yrem)/r^y
        if(r.x < 0.0){
            dt.x = -rem.x/r.x;
            ind.x = -1;
        }
        else if(r.x > 0.0){
            dt.x = (squareLength - rem.x)/r.x;
            ind.x = 1;
        }
        if(r.y > 0.0){
            dt.y = (squareLength - rem.y)/r.y;
            ind.y = 1;
        }
        else if(r.y < 0.0){
            dt.y = -rem.y/r.y;
            ind.y = -1;
        }

        //increment ray with shortest hit
        float bestdt = dt.x;
        if(dt.x < dt.y){
            bestdt = dt.x;
            ind.y = 0;
        }
        else{
            bestdt = dt.y;
            ind.x = 0;
        }
        
        rayMap.x += ind.x;
        rayMap.y += ind.y;
        rem.x += r.x * bestdt - squareLength * ind.x;
        rem.y += r.y * bestdt - squareLength * ind.y;
        //draw hit
        ST7735_DrawPixel(rayMap.x * squareLength + rem.x, rayMap.y * squareLength + rem.y, color);
    }
}
