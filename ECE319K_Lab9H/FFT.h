#ifndef FFT_H
#define FFT_H

#include <stdint.h>

#define INPUT_THRESHOLD 100
#define STRICTNESS 15
struct complex_t{
    float real;
    float imag;
};

void FFT(complex_t *x);
void FFT_Process(uint16_t *adc, float *mag);
void bit_reverse(complex_t *x);

#endif