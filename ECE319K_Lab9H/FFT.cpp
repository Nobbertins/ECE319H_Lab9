#include <stdint.h>
#include "FFT.h"
#include <math.h>

#define FS 8000     // sampling frequency
#define N 256

#define PI 3.14159265358979f

void FFT_Process(uint16_t *adc, float *mag){
  static complex_t x[N];

  float mean = 0;
  for(int i = 0; i < N; i++){
      mean += adc[i];
  }
  mean /= N;

  for(int i = 0; i < N; i++){
      x[i].real = (float)adc[i] - mean;
      x[i].imag = 0.0f;
  }

  FFT(x);

  for(int i = 0; i < N/2; i++){
    mag[i] = (x[i].real * x[i].real) + (x[i].imag * x[i].imag);
  }
}

void FFT(complex_t *x){
  bit_reverse(x);

  for(int len = 2; len <= N; len <<= 1){
    float angle = -2.0f * PI / len;
    complex_t wlen = {cosf(angle), sinf(angle)};

    for(int i = 0; i < N; i += len){
      complex_t w = {1.0f, 0.0f};

      for(int j = 0; j < len/2; j++){
        complex_t u = x[i + j];
        complex_t v = x[i + j + len/2];

        // v * w
        complex_t t;
        t.real = v.real * w.real - v.imag * w.imag;
        t.imag = v.real * w.imag + v.imag * w.real;

        // butterfly
        x[i + j].real = u.real + t.real;
        x[i + j].imag = u.imag + t.imag;

        x[i + j + len/2].real = u.real - t.real;
        x[i + j + len/2].imag = u.imag - t.imag;

        // update da twiddle
        float wr = w.real * wlen.real - w.imag * wlen.imag;
        float wi = w.real * wlen.imag + w.imag * wlen.real;
        w.real = wr;
        w.imag = wi;
      }
    }
  }
}

void bit_reverse(complex_t *x){
  int j = 0;

  for(int i = 0; i < N; i++){
    if(i < j){
      complex_t temp = x[i];
      x[i] = x[j];
      x[j] = temp;
    }

    int bit = N >> 1;
    while(j & bit){
      j ^= bit;
      bit >>= 1;
    }
    j ^= bit;
  }
}