// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Your name
// Last Modified: January 12, 2026

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/DAC5.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
#include "graphics.h"
#include "math.h"
#include "Joystick.h"
#include "Switch.h"
#include "game_globals.h"
#include "assets.h"
#include "../inc/ADC.h"
#include "FFT.h"

extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);

#define FS 8000     // sampling frequency
#define N 256

volatile uint16_t buffer1[N];
volatile uint16_t buffer0[N];
volatile uint16_t *capturePtr = buffer0; // ISR writes here
volatile uint16_t *processPtr = buffer1; // Main reads here
volatile int bufferReady = 0;
volatile int counter = 0;
volatile float debugFrequency = 0;
float mag[N];
float freq;
int peak = 0;

extern "C" void SysTick_Handler(void){
    // GPIOB->DOUTSET31_0 = GREEN;
    capturePtr[counter] = ADC0_In();
    counter++;
    if(counter >= N){
        // Swap buffers
        volatile uint16_t *temp = capturePtr;
        capturePtr = processPtr;
        processPtr = temp;
        counter = 0;
        bufferReady = 1;
        // GPIOB->DOUTCLR31_0 = GREEN;
    }
}

// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}

// games  engine runs at 30Hz
uint8_t frameSemaphore = 0;
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    // 1) sample slide pot
    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    frameSemaphore = 1;
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};

void killTargetedEnemies(int shotType){
  Enemy* e = enemyHead;
  while(e != NULL){
    bool correctShot = false;
    switch(shotType){
      case -1: correctShot = true; break;
      case 0: correctShot = (e->spriteInfo == &enemyA); break;
      case 1: correctShot = (e->spriteInfo == &enemyB); break;
      case 2: correctShot = (e->spriteInfo == &enemyC); break;
    }
    if(e->targeted && e->alive && (correctShot)) {
      e->deadFrames = deadAnimLength; 
      e->alive = false; 
      e->currentSprite = DEAD; 
      Sound_Killed(); 
      score += 10;
    }
    e = e->next;
  }
}
int enemiesSpawned = 0;
void updateEnemies(void){
  //spawning
  if(spawn_timer == 0){
    spawn_timer = spawn_cooldown;
    int selectedCorner = Random(4);
    int selectedEnemy = Random(3);
    switch(selectedEnemy){
      case 0: spawnEnemy(corners[selectedCorner], &enemyA); break;
      case 1: spawnEnemy(corners[selectedCorner], &enemyB); break;
      default: spawnEnemy(corners[selectedCorner], &enemyC); break;
    }
    enemiesSpawned++;
    if(enemiesSpawned == 5){
      enemiesSpawned = 0;
      spawn_cooldown /= 2;
      if(spawn_cooldown == 0) spawn_cooldown = 1;
    }
  }
  spawn_timer--;
  //check enemies for updates (movement, killed)
  Enemy* e = enemyHead;
  while(e != NULL){
    if(!e->alive){
      if(e->deadFrames == 0) {e = killEnemy(e); continue;}
      e->deadFrames--;
    }
    else{
      //movement
      int xOffset = camera.x - e->location.x;
      int yOffset = camera.y - e->location.y;
      float playerDistance = sqrtf(xOffset * xOffset + yOffset * yOffset);
      float xMov = xOffset / playerDistance * ENEMY_SPEED;
      float yMov = yOffset / playerDistance * ENEMY_SPEED;
      if(isMusical) {xMov *= 0.8; yMov *= 0.8;}
      int dx = lround(xMov);
      int dy = lround(yMov);
      if(gridmap[e->location.y / squareLength][(e->location.x + dx) / squareLength].color != empty16.color){
        dx = 0;
      }
      if(gridmap[(e->location.y + dy) / squareLength][(e->location.x) / squareLength].color != empty16.color){
        dy = 0;
      }
      e->location.x += dx;
      e->location.y += dy;
      //check attack
      if(playerDistance <= ENEMY_ATTACK_RANGE){
        e->alive = false;
        e->currentSprite = ATTACK;
        e->deadFrames = deadAnimLength - 1;
        hearts--;
        if(hearts == 0) gameOver = true;
        Sound_Explosion();
      }
    }
    e = e->next;
  }
}

// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
  int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_BLACKTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(ST7735_BLACK);
  Joystick_Init(); // PB18 = ADC1 channel 5, slidepot
  Buttons_Init(); // initialize switches (shoot Pa27, reload PB2)
  LED_Init();    // initialize LED
  //Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
  // initialize interrupts on TimerG12 at 30 Hz
  //TimerG12_IntArm(2666666, 2);

  Sound_Init(3636);  // initialize sound
  // TExaS_Init(0,0,&TExaS_LaunchPadLogic()); // PB27 and PB26
  // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(2666667, 0);
  SysTick_Init(80000000 / FS);
  //ADC0_Init(5, ADCVREF_VDDA);
  ADC0_InitAve(5, 2);
  //enable SysTick for performance timing
  // SysTick->LOAD = 0xFFFFFF;    // max
  // SysTick->VAL = 0;            // any write to current clears it
  // SysTick->CTRL = 0x00000005;  // enable SysTick with core clock
  // initialize all data structures
  __enable_irq();
  uint32_t startTime = SysTick->VAL;
  uint32_t stopTime = SysTick->VAL;
  uint32_t Offset = (startTime-stopTime)&0x0FFFFFF; // in bus cycles
  uint32_t rendertime = ((startTime-stopTime)&0x0FFFFFF)-Offset; // in bus cycles
  float cameraAngle = 0;
  int shootCooldown = SHOOT_COOLDOWN;
  int shootHold = SHOOT_HOLD;
  int reloadHold = RELOAD_HOLD;
  bool doublePress = false;
  //A440 (c major)
  const int numNotes = 8;
  const int normalNotes[numNotes] = {440, 494, 523, 587, 659, 687, 784, 880};
  const int recorderNotes[numNotes] = {593, 656, 719, 781, 875, 969, 1062, 1156};
  const int* notes = normalNotes;
  int currentNote = -1;
  //drawTopDown();
  while(1){
    isEnglish = false;
    if(gameOver){
      ST7735_FillScreen(ST7735_BLACK);
      ST7735_SetCursor(0, 0);
      if(isEnglish) ST7735_OutString("Main Menu");
      else ST7735_OutString("El Menu Principal");
      char buf[20];
      ST7735_SetCursor(0, 1);
      if(isEnglish) snprintf(buf, sizeof(buf), "Score: %d", score);
      else snprintf(buf, sizeof(buf), "Puntos: %d", score);
      ST7735_OutString(buf);
      ST7735_SetCursor(0, 2);
      if(isEnglish) ST7735_OutString("Shoot = Normal Mode");
      else ST7735_OutString("Disparar = Normal");
      ST7735_SetCursor(0, 3);
      if(isEnglish) ST7735_OutString("Reload = Hard Mode");
      else ST7735_OutString("Recargar = Dificil");
      bool reloadButton = readReloadButton();
      bool shootButton = readShootButton();
      while(!(reloadButton || shootButton)){
        reloadButton = readReloadButton();
        shootButton = readShootButton();
      }
      if(reloadButton) isMusical = false;
      else isMusical = true;
      gameOver = false;
      ammo = AMMO_LIMIT;
      hearts = HEART_LIMIT;
      while(enemyHead != NULL) enemyHead = killEnemy(enemyHead);
      enemyTail = NULL;
      isShooting = false;
      shotType = -1;
      score = 0;
      if(isMusical) spawn_cooldown = START_SPAWN_COOLDOWN * 2;
      else spawn_cooldown = START_SPAWN_COOLDOWN;
      spawn_timer = spawn_cooldown;
      total_enemies = 0;
      camera = {70, 85};
      cameraMap = {4, 5};
      cameraDirection = {0.0, 1.0};
    }
    if(bufferReady){
      FFT_Process((uint16_t*)processPtr, mag);
      int peak = 1;
      for(int i = 2; i < N/2; i++){ 
        if(mag[i] > mag[peak]){
          peak = i;
        }
      }
      float highest = mag[peak] / 1000000.0;
      if(highest > INPUT_THRESHOLD) freq = (float)peak * FS / N;
      else freq = 0;
      bufferReady = 0;

      bool noteFound = false;
      for(int i = 0; i < numNotes; i++){
        if(freq <= notes[i] + STRICTNESS && freq >= notes[i] - STRICTNESS){
          currentNote = i;
          noteFound = true;
        }
      }
      if(!noteFound) currentNote = -1; 
    }
    // wait for semaphore
    if(frameSemaphore == 1){
      frameSemaphore = 0;
      //startTime = SysTick->VAL;  

      // //top down drawing
      // drawPlayer();
      // drawTopDown();

      //3D
      Joystick_Dir j = Joystick_Read();

      shootCooldown--;
      if(shootCooldown == -1) shootCooldown = 0;

      if(isShooting){
        shootHold--;
        if(shootHold == -1) {shootHold = 0; isShooting = false;}
      }

      bool reloadButton = readReloadButton() || (currentNote == 3);
      bool shootButton = readShootButton() || (currentNote == 0 || currentNote == 1 || currentNote == 2);

      //apply audio input
      
      if(currentNote == 7) j.x = 1.0;
      if(currentNote == 6) j.x = -1.0;
      if(currentNote == 4) j.y = 1.0;
      if(currentNote == 5) j.y = -1.0;

      if(reloadButton && shootButton){
        if(!doublePress){
          isEnglish = !isEnglish;
          doublePress = true;
        }
      }
      else if(reloadButton){
        doublePress = false;
        if(reloadHold == 0) { if(ammo < AMMO_LIMIT) {ammo++; Sound_Reload();} reloadHold = RELOAD_HOLD;}
        else reloadHold--;
      }
      else if(shootButton){
        doublePress = false;
        reloadHold = RELOAD_HOLD;
        if(!isShooting && shootCooldown == 0 && ammo > 0){
          if(currentNote == 0 || currentNote == 1 || currentNote == 2) shotType = currentNote;
          else shotType = -1;
          isShooting = true;
          shootCooldown = SHOOT_COOLDOWN;
          shootHold = SHOOT_HOLD;
          ammo--;
          killTargetedEnemies(shotType);
          Sound_Shoot();
        }
      }
      else {reloadHold = RELOAD_HOLD; doublePress = false;}

      if(fabsf(j.x) > 0.1) cameraAngle += (maxTurnSpeed * j.x);
      if(cameraAngle >= 360) cameraAngle -= 360;

      Vector2D oldDir = cameraDirection;
      
      cameraDirection.x = cosf(cameraAngle * 3.14159 / 180);
      cameraDirection.y = sinf(cameraAngle * 3.14159 / 180);

      moveCamera({j.x, j.y});

      updateEnemies();
      
      drawRaycasts(cameraDirection);
       
      // stopTime = SysTick->VAL;
      // rendertime = ((startTime-stopTime)&0x0FFFFFF)-Offset; // in bus cycles
    }
  }
}
      //joystick debug
      // Joystick_Dir j = Joystick_Read();
      // char buf[24];
      // snprintf(buf, sizeof(buf), "%.2f, %.2f   ", j.x, j.y);
      // ST7735_SetCursor(0, 0);
      // ST7735_OutString(buf);
      // ST7735_SetCursor(0, 1);
      // Joystick_ADC ja = Joystick_In();
      // snprintf(buf, sizeof(buf), "%d, %d   ", ja.x, ja.y);
      // ST7735_OutString(buf);

void ADC0_InitAve(uint32_t channel, uint32_t n){
  ADC0->ULLMEM.GPRCM.RSTCTL = 0xB1000003;  // 1) reset
  ADC0->ULLMEM.GPRCM.PWREN = 0x26000001;   // 2) activate
  Clock_Delay(24);                         // 3) wait
  ADC0->ULLMEM.GPRCM.CLKCFG = 0xA9000000;  // 4) ULPCLK
  ADC0->ULLMEM.CLKFREQ = 7;                // 5) 40-48 MHz
  ADC0->ULLMEM.CTL0 = 0x03010000;          // 6) divide by 8
  ADC0->ULLMEM.CTL1 = 0x00000000|(n<<28)|(n<<24);// 7) mode
  // AVGD = AVEN = n
  ADC0->ULLMEM.CTL2 = 0x00000000;          // 8) MEMRES
  if(n){
    ADC0->ULLMEM.MEMCTL[0] = (1<<16)|channel;  // 9) channel
  }else{
    ADC0->ULLMEM.MEMCTL[0] = channel;      // 9) channel
  }
  ADC0->ULLMEM.SCOMP0 = 0;                 // 10) 8 sample clocks
  ADC0->ULLMEM.GEN_EVENT.IMASK = 0;       // 11) no interrupt
}



void ADC0_Init(uint32_t channel, uint32_t reference){
    // Reset ADC and VREF
    // RSTCLR
    //   bits 31-24 unlock key 0xB1
    //   bit 1 is Clear reset sticky bit
    //   bit 0 is reset ADC
  ADC0->ULLMEM.GPRCM.RSTCTL = 0xB1000003;
  if(reference == ADCVREF_INT){
    VREF->GPRCM.RSTCTL = 0xB1000003;
  }
    // Enable power ADC and VREF
    // PWREN
    //   bits 31-24 unlock key 0x26
    //   bit 0 is Enable Power
  ADC0->ULLMEM.GPRCM.PWREN = 0x26000001;
  if(reference == ADCVREF_INT){
    VREF->GPRCM.PWREN = 0x26000001;
  }
  Clock_Delay(24); // time for ADC and VREF to power up
  ADC0->ULLMEM.GPRCM.CLKCFG = 0xA9000000; // ULPCLK
  // bits 31-24 key=0xA9
  // bit 5 CCONSTOP= 0 not continuous clock in stop mode
  // bit 4 CCORUN= 0 not continuous clock in run mode
  // bit 1-0 0=ULPCLK,1=SYSOSC,2=HFCLK
  ADC0->ULLMEM.CLKFREQ = 7; // 40 to 48 MHz
  ADC0->ULLMEM.CTL0 = 0x03010000;
  // bits 26-24 = 011 divide by 8
  // bit 16 PWRDN=1 for manual, =0 power down on completion, if no pending trigger
  // bit 0 ENC=0 disable (1 to 0 will end conversion)
  ADC0->ULLMEM.CTL1 = 0x00000000;
  // bits 30-28 =0  no shift
  // bits 26-24 =0  no averaging
  // bit 20 SAMPMODE=1 software triggers
  // bits 17-16 CONSEQ=0 ADC at start will be sampled once, 10 for repeated sampling
  // bit 8 SC=0 for stop, =1 to software start
  // bit 0 TRIGSRC=0 software trigger
  ADC0->ULLMEM.CTL2 = 0x00000000;
  // bits 28-24 ENDADD (which  MEMCTL to end)
  // bits 20-16 STARTADD (which  MEMCTL to start)
  // bits 15-11 SAMPCNT (for DMA)
  // bit 10 FIFOEN=0 disable FIFO
  // bit 8  DMAEN=0 disable DMA
  // bits 2-1 RES=0 for 12 bit (=1 for 10bit,=2for 8-bit)
  // bit 0 DF=0 unsigned formant (1 for signed, left aligned)
  ADC0->ULLMEM.MEMCTL[0] = 5;
  // bit 28 WINCOMP=0 disable window comparator
  // bit 24 TRIG trigger policy, =0 for auto next, =1 for next requires trigger
  // bit 20 BCSEN=0 disable burn out current
  // bit 16 = AVGEN =0 for no averaging
  // bit 12 = STIME=0 for SCOMP0
  // bits 9-8 VRSEL = 10 for internal VREF,(00 for VDDA)
  // bits 4-0 channel = 0 to 7 available
  ADC0->ULLMEM.SCOMP0 = 0; // 8 sample clocks
//  ADC0->ULLMEM.GEN_EVENT.ICLR |= 0x0100; // clear flag MEMCTL[0]
//  ADC0->ULLMEM.GEN_EVENT.IMASK = 0; // no interrupt
  if(reference == ADCVREF_INT){
    VREF->CLKSEL = 0x00000008; // bus clock
    VREF->CLKDIV = 0; // divide by 1
    VREF->CTL0 = 0x0001;
  // bit 8 SHMODE = off
  // bit 7 BUFCONFIG=0 for 2.4 (=1 for 1.4)
  // bit 0 is enable
    VREF->CTL2 = 0;
  // bits 31-16 HCYCLE=0
    // bits 15-0 SHCYCLE=0
    while((VREF->CTL1&0x01)==0){}; // wait for VREF to be ready
  }
}
// sample 12-bit ADC
uint32_t ADC0_In(void){
  ADC0->ULLMEM.CTL0 |= 0x00000001; // enable conversions
  ADC0->ULLMEM.CTL1 |= 0x00000100; // start ADC
  uint32_t volatile delay=ADC0->ULLMEM.STATUS; // time to let ADC start
  while((ADC0->ULLMEM.STATUS&0x01)==0x01){} // wait for completion
  return ADC0->ULLMEM.MEMRES[0];
}