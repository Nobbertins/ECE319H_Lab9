#include "graphics.h"
#include "math.h"

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
            ST7735_DrawFastVLine(c * squareLength + line, r * squareLength, squareLength, gridmap[r][c].color);
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

//raycast from camera with raycast direction r, returns Wall hit
Wall drawRaycast(Vector2D r, float angle){
    Pos rayMap = {cameraMap.x, cameraMap.y};
    Vector2D rem = {(float)(camera.x - cameraMap.x * squareLength), (float)(camera.y - cameraMap.y * squareLength)};

    Pos ind = {0, 0}; //map square index incrementer (used to tell if x-hit or y-hit afterwards)
    while(gridmap[rayMap.y][rayMap.x].color == empty16.color){
        Vector2D dt = {10e30, 10e30}; //assume worst for == 0 case
        ind = {0,0};
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
        //draw hit (need color var)
        //ST7735_DrawPixel(rayMap.x * squareLength + rem.x, rayMap.y * squareLength + rem.y, color);
    }
    Wall w;
    //not actual distance (straight distance from player is better avoids fisheye)
    w.dist = sqrtf(((rayMap.x * squareLength + rem.x) - camera.x) * ((rayMap.x * squareLength + rem.x) - camera.x) + ((rayMap.y * squareLength + rem.y) - camera.y) * ((rayMap.y * squareLength + rem.y) - camera.y));
    //correct with raycasted angle (in rads)
    w.dist *= cosf(angle * 3.14159 / 180);

    //light color for y-hits, normal color for x-hits (lighting trick)
    if(ind.x == 0) w.color = gridmap[rayMap.y][rayMap.x].light_color;
    else w.color = gridmap[rayMap.y][rayMap.x].color;
    
    return w;
}

void drawRaycasts(Vector2D facing){
    Vector2D pb = getPerp(facing);
    Vector2D p;//init
    for(int i = 0; i < screenWidth; i++){
        //p = facing + (screenWidth/2 - i) * pb
        p.x = facing.x + (cameraWidth/2 - (float)i/screenWidth * cameraWidth) * pb.x;
        p.y = facing.y + (cameraWidth/2 - (float)i/screenWidth * cameraWidth) * pb.y;
        //norm
        float mag = sqrtf(p.x * p.x + p.y * p.y);
        p.x /= mag;
        p.y /= mag;
        //drawRaycast
        Wall w = drawRaycast(p, FOV/2 - (float)i/screenWidth * FOV);
        renderColumn(i, w);
    }
}

void renderColumn(int col, Wall w) {
    int16_t ceilingCutoff = screenHeight / 2 - (int)(screenHeight / w.dist);
    int16_t wallCutoff    = screenHeight / 2 + (int)(screenHeight / w.dist);

    //clamp in case
    if (ceilingCutoff < 0) ceilingCutoff = 0;
    if (wallCutoff > screenHeight - 1) wallCutoff = screenHeight - 1;

    int16_t ceilingLen = ceilingCutoff;
    int16_t wallLen    = wallCutoff - ceilingCutoff;
    int16_t floorLen   = screenHeight - wallCutoff;

    ST7735_DrawFastVLine(col, 0,             ceilingLen, ceilingColor);
    ST7735_DrawFastVLine(col, ceilingCutoff, wallLen,    w.color);
    ST7735_DrawFastVLine(col, wallCutoff,    floorLen,   floorColor);
}

void moveCamera(Vector2D j){
    float oldX = camera.x;
      float oldY = camera.y;

      camera.x += (int) (cameraDirection.x * maxMoveSpeed * j.y);
      camera.y += (int) (cameraDirection.y * maxMoveSpeed * j.y);
      
      int lookaheadX = camera.x + (int)(cameraClippingRadius * cameraDirection.x * copysignf(1.0, j.y));
      bool xBlocked = 
          gridmap[cameraMap.y][lookaheadX / squareLength].color != empty16.color ||
          gridmap[(camera.y + cameraClippingRadius) / squareLength][lookaheadX / squareLength].color != empty16.color ||
          gridmap[(camera.y - cameraClippingRadius) / squareLength][lookaheadX / squareLength].color != empty16.color;

      if(xBlocked) camera.x = oldX;
      else cameraMap.x = camera.x/squareLength;

      int lookaheadY = camera.y + (int)(cameraClippingRadius * cameraDirection.y * copysignf(1.0, j.y));
      bool yBlocked = 
          gridmap[lookaheadY/squareLength][cameraMap.x].color != empty16.color ||
          gridmap[lookaheadY/squareLength][(camera.x + cameraClippingRadius) / squareLength].color != empty16.color ||
          gridmap[lookaheadY/squareLength][(camera.x - cameraClippingRadius) / squareLength].color != empty16.color;

      if(yBlocked) camera.y = oldY;
      else cameraMap.y = camera.y/squareLength;   

      // check all 4 sides regardless of movement direction (for weird edge cases)
      bool tooClose =
          gridmap[cameraMap.y][(camera.x + cameraClippingRadius) / squareLength].color != empty16.color ||
          gridmap[cameraMap.y][(camera.x - cameraClippingRadius) / squareLength].color != empty16.color ||
          gridmap[(camera.y + cameraClippingRadius) / squareLength][cameraMap.x].color != empty16.color ||
          gridmap[(camera.y - cameraClippingRadius) / squareLength][cameraMap.x].color != empty16.color;

      if(tooClose) {
          camera.x = oldX;
          camera.y = oldY;
          cameraMap.x = oldX / squareLength;
          cameraMap.y = oldY / squareLength;
      }
}