#include "graphics.h"
#include "math.h"
#include "assets.h"
#include "game_globals.h"

//global variable definitions
Pos camera = {70, 85};
Pos cameraMap = {4, 5};
Vector2D cameraDirection = {0.0, 1.0};
const float FOV_rads = FOV * 3.14159f / 180.0f;

Enemy* enemyHead = NULL;
Enemy* enemyTail = NULL;

EnemySpriteInfo enemyA = {A_sprite_idle, A_sprite_attack, A_sprite_dead};

//function definitions
void spawnEnemy(Vector2D pos, EnemySpriteInfo* spriteInfo){
    //empty
    if(enemyHead == NULL){
        enemyHead = (Enemy*) malloc(sizeof(Enemy));
        *enemyHead = {pos, spriteInfo, IDLE, 4, NULL, NULL, -1, -1, -1, -1, -1, -1, -1, -1, -1.0, false, true, deadAnimLength};
        enemyTail = enemyHead;
        return;
    }
    //not empty
    enemyTail->next = (Enemy*) malloc(sizeof(Enemy));
    *(enemyTail->next) = {pos, spriteInfo, IDLE, 4, NULL, enemyTail, -1, -1, -1, -1, -1, -1, -1, -1, -1.0, false, true, deadAnimLength};
    enemyTail = enemyTail->next;
}
//kills enemy and returns next
Enemy* killEnemy(Enemy* enemy){
    if(enemy->prev == NULL && enemy->next == NULL) {enemyHead = enemyTail = NULL;}
    else if(enemy->prev == NULL) {enemyHead = enemy->next; enemyHead->prev = NULL;}
    else if(enemy->next == NULL) {enemyTail = enemy->prev; enemyTail->next = NULL;}
    else{
        (enemy->prev)->next = enemy->next;
        (enemy->next)->prev = enemy->prev;
    }
    Enemy* n = enemy->next;
    free(enemy);
    return n;
}

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
//raycast from camera with raycast direction r, ret
}

void drawCrosshair(void){
    ST7735_DrawFastVLine(63, 77, 6, playerColor);
    ST7735_DrawFastVLine(64, 77, 6, playerColor);
    ST7735_DrawFastHLine(61, 80, 6, playerColor);
    ST7735_DrawFastHLine(61, 79, 6, playerColor);
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
    

    //find all enemies on screen
    //float ang = atan2f(cameraDirection.y, cameraDirection.x);
    float sang = cameraDirection.y;
    float cang = cameraDirection.x;
    Enemy* e = enemyHead;
    while(e != NULL){
        //also reset targeted flag set by graphics engine
        e->targeted = false;
        float camera_ex = cang * (e->location.x - camera.x) + sang * (e->location.y - camera.y);
        float camera_ey = -sang * (e->location.x - camera.x) + cang * (e->location.y - camera.y);
        //skip rendering if out of view (negative, zero will crash, give some room)
        if(camera_ex < 0.2){
             e->x_hi = e->x_lo = e->y_hi = e->y_lo = -1;
             e = e->next;
             continue;
        }
        e->x_lo = (int) ((0.5 - (camera_ey + e->spriteWidth/2)/(FOV_rads * (camera_ex))) * screenWidth);
        e->x_hi = (int) ((0.5 - (camera_ey - e->spriteWidth/2)/(FOV_rads * (camera_ex))) * screenWidth);
        e->y_lo = screenHeight / 2 - (int)(screenHeight / (camera_ex));
        e->y_hi = screenHeight / 2 + (int)(screenHeight / (camera_ex));
        e->cx_lo = clamp(e->x_lo, 0, screenWidth-1);
        e->cx_hi = clamp(e->x_hi, 0, screenWidth-1);
        e->cy_lo = clamp(e->y_lo, 0, screenHeight-1);
        e->cy_hi = clamp(e->y_hi, 0, screenHeight-1);
        //edge column issue
        if(e->cx_lo == screenWidth - 1 || e->cx_hi == 0) {e->x_hi = e->x_lo = e->y_lo = e->y_hi = -1;}
        e->dist = camera_ex;
        e = e->next;
    }

    //draw rays
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
        //renderColumn(i, w);
        renderBufferedColumn(i, w);
    }
}

void renderColumn(int col, Wall w) {
    int16_t ceilingCutoff = screenHeight / 2 - (int)(screenHeight / w.dist);
    int16_t wallCutoff    = screenHeight / 2 + (int)(screenHeight / w.dist);

    int16_t ceilingLen = ceilingCutoff;
    int16_t wallLen    = wallCutoff - ceilingCutoff;
    int16_t floorLen   = screenHeight - wallCutoff;

    ST7735_DrawFastVLine(col, 0,             ceilingLen, ceilingColor);
    ST7735_DrawFastVLine(col, ceilingCutoff, wallLen,    w.color);
    ST7735_DrawFastVLine(col, wallCutoff,    floorLen,   floorColor);
}

void renderBufferedColumn(int col, Wall w){
    uint16_t col_buf[160];

    int16_t ceilingCutoff = screenHeight / 2 - (int)(screenHeight / w.dist);
    int16_t wallCutoff    = screenHeight / 2 + (int)(screenHeight / w.dist);
    //clamp in case
    if (ceilingCutoff < 0) ceilingCutoff = 0;
    if (wallCutoff > screenHeight - 1) wallCutoff = screenHeight - 1;

    //fill array (bottom to top)
    uint16_t color = ceilingColor;
    for(int i = 159; i >= UI_HEIGHT; i--){
        if(i == wallCutoff) color = w.color;
        if(i == ceilingCutoff) color = floorColor;
        col_buf[i] = color;
    }

    //fill enemies
    Enemy* e = enemyHead;
    while(e != NULL){
        if(e->x_hi == -1 || e->dist > w.dist) {e = e->next; continue;}
        if(col >= e->cx_lo && col <= e->cx_hi){
            for(int i = e->cy_lo; i <= e->cy_hi; i++){
                    uint16_t imgColor;
                    switch(e->currentSprite){
                        case IDLE: 
                            imgColor = e->spriteInfo->idle_sprite[(int)((float) (enemySize - 1) / (e->x_hi - e-> x_lo) * (col - e->x_lo))][(int)((float)(enemySize - 1) / (e->y_hi - e->y_lo) * (i - e->y_lo))];
                            break;
                        case ATTACK: 
                            imgColor = e->spriteInfo->attack_sprite[(int)((float) (enemySize - 1) / (e->x_hi - e-> x_lo) * (col - e->x_lo))][(int)((float)(enemySize - 1) / (e->y_hi - e->y_lo) * (i - e->y_lo))];
                            break;
                        case DEAD: 
                            imgColor = e->spriteInfo->dead_sprite[(int)((float) (enemySize - 1) / (e->x_hi - e-> x_lo) * (col - e->x_lo))][(int)((float)(enemySize - 1) / (e->y_hi - e->y_lo) * (i - e->y_lo))];
                            break;
                    }
                    if(imgColor != empty16.color) col_buf[e->cy_hi - i + e->cy_lo] = imgColor;
                    if((i == screenHeight/2 || i == screenHeight/2-1) && (col == screenWidth/2 ||col == screenWidth/2-1)) e->targeted = true;
            }
        }
        e = e->next;
    }

    //fill crosshair
    if(col >= 61 && col <= 66) col_buf[79] = col_buf[80] = playerColor;
    if(col == 63 || col == 64) col_buf[77] = col_buf[78] = col_buf[81] = col_buf[82] = playerColor;

    //fill hand and gun
    if(col >= screenWidth/2 - 12 && col <= screenWidth/2 + 11){
        for(int i = 27; i >= 0; i--){
            uint16_t pixel;
            if(isShooting) pixel = gunshot_sprite[col - (screenWidth/2 - 12)][i];
            else pixel = gun_sprite[col - (screenWidth/2 - 12)][i];
            if(pixel != empty16.color) col_buf[27 - i + UI_HEIGHT] = pixel;
        }
    }
    
    //fill UI
    for(int i = 0; i < UI_HEIGHT; i++){
        if(isEnglish) col_buf[i] = english_UI_sprite[col][UI_HEIGHT - 1 - i];
        else col_buf[i] = spanish_UI_sprite[col][UI_HEIGHT - 1 - i];
    }

    //fill hearts
    for(int i = 0; i < hearts; i++){
        if(col == 35 + i * 9 || col == 39 + i * 9) col_buf[5] = heartColor;
        else if(col == 36 + i * 9 || col == 38 + i * 9) col_buf[5] = col_buf[4] = heartColor;
        else if(col == 37 + i * 9) col_buf[3] = col_buf[4] = heartColor;
    }

    //fill ammo
    for(int i = 0; i < ammo; i++){
        if(col == 86 + 5 * i) col_buf[3] = col_buf[4] = ammoColor;
        else if(col == 87 + 5 * i) col_buf[3] = col_buf[4] = ammoColor;
    }

    //fill score
    int numbers[5];
    numbers[4] = score / 10000;
    numbers[3] = (score % 10000) / 1000;
    numbers[2] = (score % 1000) / 100;
    numbers[1] = (score % 100) / 10;
    numbers[0] = score % 10;

    for(int i = 0; i < 5; i++){
        if(col == 24 - 5 * i) {for(int j = 0; j < 5; j++) if(num_sprite[numbers[i]][0][j] == yc) col_buf[j + 2] = num_sprite[numbers[i]][0][j];}
        else if(col == 25 - 5 * i) {for(int j = 0; j < 5; j++) if(num_sprite[numbers[i]][1][j] == yc) col_buf[j + 2] = num_sprite[numbers[i]][1][j];}
        else if(col == 26 - 5 * i) {for(int j = 0; j < 5; j++) if(num_sprite[numbers[i]][2][j] == yc) col_buf[j + 2] = num_sprite[numbers[i]][2][j];}
    }

    //render array
    ST7735_DrawBitmap(col, 159, (const uint16_t*) col_buf, 1, 160);
}


//moves camera and returns whether or not has moved
bool moveCamera(Vector2D j){
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

      return !((oldX == camera.x) && (oldY == camera.y));
}