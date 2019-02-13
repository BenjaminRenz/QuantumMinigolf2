#ifndef SIMULATION_H_INCLUDED
#define SIMULATION_H_INCLUDED
#include "fftw3.h"
#include <stdint.h>
int sim_res_x;
int sim_res_y;
int sim_res_total;

fftw_complex *psi;

void simulation_alloc(); //Needs to be called before any aanimation can start.
void simulation_dealloc();

void simulation_pause();
void simulation_unpause();

int simulation_redraw_wave(int offset_x,int offset_y,float angle,float momentum,float gauss_width);
int simulation_run(float dt);

enum {meas_hit,meas_no_hit};
int simulation_measurement(double glfwTime);
void simulation_load_potential(uint8_t* pot);

#endif
