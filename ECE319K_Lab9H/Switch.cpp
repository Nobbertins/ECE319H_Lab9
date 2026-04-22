/*
 * Switch.cpp
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "Switch.h"

// LaunchPad.h defines all the indices into the PINCM table
void Buttons_Init(void){
    // write this
  IOMUX->SECCFG.PINCM[PA27INDEX] = 0x00050081; // input, pullup
  IOMUX->SECCFG.PINCM[PB7INDEX] = 0x00050081; // input, pullup
}
// return current state of switches
bool readShootButton(void){
    // write this
  uint32_t data = GPIOA->DIN31_0;
  return ((data & (1 << 27)) != 0);
}
bool readReloadButton(void){
  uint32_t data = GPIOB->DIN31_0;
  return ((data & (1 << 7)) != 0);
}
