/* Joystick.cpp
 * Students: Ayaan and Henry
 */
#include <ti/devices/msp/msp.h>
#include "../inc/Clock.h"
#include "Joystick.h"
#include "math.h"

#define ADCVREF_VDDA 0x000
#define ADCVREF_INT  0x200

const Joystick_ADC joystick_start = {3033, 3122}; //also is mag of min
const Joystick_Dir joystick_max = {4095 - (float)joystick_start.x, 4095 - (float)joystick_start.y};

void Joystick_Init(void){
  ADC1->ULLMEM.GPRCM.RSTCTL = 0xB1000003; // 1) reset
  ADC1->ULLMEM.GPRCM.PWREN = 0x26000001;  // 2) activate
  Clock_Delay(24);                        // 3) wait
  ADC1->ULLMEM.GPRCM.CLKCFG = 0xA9000000; // 4) ULPCLK
  ADC1->ULLMEM.CLKFREQ = 7;               // 5) 40-48 MHz
  ADC1->ULLMEM.CTL0 = 0x03010000;         // 6) divide by 8
  ADC1->ULLMEM.CTL1 = 0x00010000;         // 7) mode (sequence sample) (bits 16-17)
  ADC1->ULLMEM.CTL2 = 0x01000000;         // 8) MEMRES (start (bits 20-16) and stop (bits 28-24) of sequence sample)
  ADC1->ULLMEM.MEMCTL[0] = 5;             // 9) channel 5 is PB18
  ADC1->ULLMEM.MEMCTL[1] = 6; //channel 6 is PB19
  ADC1->ULLMEM.SCOMP0 = 0;                // 10) 8 sample clocks
  ADC1->ULLMEM.CPU_INT.IMASK = 0;         // 11) no interrupt
}


Joystick_ADC Joystick_In(void){
  ADC1->ULLMEM.CTL0 |= 0x00000001;             // 1) enable conversions
  ADC1->ULLMEM.CTL1 |= 0x00000100;             // 2) start ADC
  uint32_t volatile delay=ADC1->ULLMEM.STATUS; // 3) time to let ADC start
  while((ADC1->ULLMEM.STATUS&0x01)==0x01){}    // 4) wait for completion
  //adcData = ADC1->ULLMEM.MEMRES[0];
  //semaphoreVar=1;
  return {ADC1->ULLMEM.MEMRES[0], ADC1->ULLMEM.MEMRES[1]};               // 5) 12-bit result
}

//hits top value very fast so extremely sensitive (also invert x-axis)
Joystick_Dir Joystick_Read(void){
  Joystick_ADC j = Joystick_In();
  Joystick_Dir out;
  // if(j.x > joystick_start.x) out.x = (j.x - joystick_start.x)/joystick_max.x;
  // else out.x = ((float)j.x - joystick_start.x)/joystick_start.x;
  // if(j.y > joystick_start.y) out.y = (j.y - joystick_start.y)/joystick_max.y;
  // else out.y = ((float)j.y - joystick_start.y)/joystick_start.y;
  return {-fmaxf(((float)j.x - joystick_start.x)/joystick_max.x, -1.0), fmaxf(((float)j.y - joystick_start.y)/joystick_max.y, -1.0)};
}