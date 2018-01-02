//#include "libraries/FFTW_3.3.5/include/fftw3.h"     //Depending on the desired precision use fftw3 (double), fftw3f (single) or fftwl (long double)
#include "fft.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    width = 32;
    height = 32;

    psi = (fftw_complex *) fftw_alloc_complex(width * height);

    fft = fftw_plan_dft_2d (width, height, psi, psi, FFTW_FORWARD, FFTW_MEASURE);

    for(int i=0;i<height*width;i++) {
        psi[i][0]=i%32*1.0;
    }

    for(int i=0;i<height*width;i++) {
        psi[i][1]=0.0;
    }

    for(int i=0;i<height*width;i++) {
        printf("%f;%f\n", psi[i][0], psi[i][1]);
    }

    fftw_execute(fft);

    for(int i=0;i<height*width;i++) {
        printf("%f;%f\n", psi[i][0], psi[i][1]);
    }

    fftw_destroy_plan(fft);
    fftw_free(psi);
}
