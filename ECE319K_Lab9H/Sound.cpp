// Sound.cpp
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// your name
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC5.h"
#include "../inc/Timer.h"
#include "../inc/Clock.h"

uint32_t Index=0;           // Index varies from 0 to 15
volatile const uint8_t *SoundPt;
volatile uint32_t SoundCount;

// void SysTick_IntArm(uint32_t period, uint32_t priority){
//   SysTick->CTRL = 0x00;      // disable SysTick during setup

//   SysTick->LOAD = period-1;  // reload value
//   SCB->SHP[1] = (SCB->SHP[1]&(~0xC0000000))|(priority<<30); // priority 2
//   SysTick->VAL = 0;          // any write to VAL clears COUNT and sets VAL equal to LOAD
//   SysTick->CTRL = 0x07;      // enable SysTick with 80 MHz bus clock and interrupts
// }
// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5 bit DAC
void Sound_Init(uint32_t period){
  DAC5_Init();

  // 1. Reset + power
  TIMG0->GPRCM.RSTCTL = 0xB1000003;
  TIMG0->GPRCM.PWREN  = 0x26000001;
  Clock_Delay(24);

  // 2. Clock = bus (ULPCLK = 40 MHz)
  TIMG0->CLKSEL = 0x08;

  // 3. No divider
  TIMG0->CLKDIV = 0;

  // 4. Prescale = 1
  TIMG0->COMMONREGS.CPS = 0;

  // 5. Set period
  TIMG0->COUNTERREGS.LOAD = period - 1;

  // 6. Enable interrupt
  TIMG0->CPU_INT.IMASK = 0x01;

  // 7. Arm NVIC (from your table → bit 16)
  NVIC->ISER[0] = 1 << 16;

  // Optional: set priority (lower than SysTick if you want)
  NVIC->IP[4] = (NVIC->IP[4] & ~0x000000C0) | 0x00000080;

  // 8. Enable timer (periodic mode)
  TIMG0->COUNTERREGS.CTRCTL = 0x02; // periodic
  TIMG0->COMMONREGS.CCLKCTL = 1;
  TIMG0->COUNTERREGS.CTRCTL |= 0x01; // enable
// write this
 
}
// extern "C" void SysTick_Handler(void);
// void SysTick_Handler(void){ // called at 11 kHz
//   // output one value to DAC if a sound is active
// }

extern "C" void TIMG0_IRQHandler(void){
  if(TIMG0->CPU_INT.IIDX == 1){ // acknowledge
    if(SoundCount){
      DAC5_Out((*SoundPt)>>3);
      SoundPt++;
      SoundCount--;
    } else {
      DAC5_Out(0); // silence
    }
  }
}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count){
  if(SoundCount==0){
    SoundPt = pt;
    SoundCount = count;
  }
}

void Sound_Shoot(void){
// write this
  Sound_Start(shoot, 4080);
}
void Sound_Killed(void){
// write this
  Sound_Start(invaderkilled, 3377);
}
void Sound_Explosion(void){
// write this
  Sound_Start(fastinvader2, 3377);
}

void Sound_Fastinvader1(void){

}
void Sound_Fastinvader2(void){

}
void Sound_Fastinvader3(void){

}
void Sound_Fastinvader4(void){

}
void Sound_Highpitch(void){

}
