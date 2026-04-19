// Joystick.h
// Runs on MSPM0
// Provide functions that initialize ADC1 channel 5, PB18 and use a slide pot to measure distance
// Created: July 19, 2025
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#ifndef JoyStick_H
#define JoyStick_H
#include <stdint.h>

struct Joystick_ADC{
  uint32_t x;
  uint32_t y;
};

struct Joystick_Dir{
  float x;
  float y;
};

extern const Joystick_ADC joystick_start; //also is mag of min
extern const Joystick_Dir joystick_max;


void Joystick_Init(void); // initialize slide pot
Joystick_ADC Joystick_In(void); // return last ADC sample value (0 to 4095)
Joystick_Dir Joystick_Read(void); //read float

#endif
