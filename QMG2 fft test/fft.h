#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define PI 3.14159265358979323846

#include "libraries/FFTW_3.3.5/include/fftw3.h"
#include <stdio.h>
#include <stdlib.h>

int width, height;

fftwf_complex *psi;
fftw_plan fft;

#endif // MAIN_H_INCLUDED
