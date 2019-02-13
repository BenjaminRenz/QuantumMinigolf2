#ifndef SIMULATION_H_INCLUDED
#define SIMULATION_H_INCLUDED
#include "fftw3.h"
#include <stdint.h>
int simulation_resolution_x;
int simulation_resolution_y;

void simulation_alloc(); //Needs to be called before any aanimation can start.
void simulation_dealloc();

int simulation_redraw_wave(int offset_x,int offset_y,float angle,float momentum,float gauss_width);
int simulation_run(float dt);
int simulation_animate_measurement();


#endif
