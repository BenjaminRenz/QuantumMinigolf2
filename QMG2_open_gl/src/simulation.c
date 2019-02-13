#include "simulation.h"
#include <string.h>
#include <math.h>

int simulation_resolution_total;
enum {simulation_state_not_allocated,simulation_state_simulate,simulation_state_measurement_animation,simulation_state_created_and_wait_for_start,simulation_state_wait_for_restart};
int simulation_state=simulation_state_not_allocated;
//typedef double fftw_complex[2];

fftw_complex *psi;
fftw_complex *psi_transform;
fftw_complex *prop;
fftw_complex *animation_start;
fftw_complex *animation_end;
fftw_plan fft;
fftw_plan ifft;

double* potential;


void simulation_alloc(){
    simulation_resolution_total=simulation_resolution_x*simulation_resolution_y;
    psi = fftw_alloc_complex(simulation_resolution_total);
    psi_transform = (fftw_complex*) fftw_alloc_complex(simulation_resolution_total);
    prop = (fftw_complex*) fftw_alloc_complex(simulation_resolution_total);

    animation_start = (fftw_complex*) fftw_alloc_complex(simulation_resolution_total);
    animation_end = (fftw_complex*) fftw_alloc_complex(simulation_resolution_total);

    fft = fftw_plan_dft_2d(simulation_resolution_x, simulation_resolution_y, psi, psi_transform, FFTW_FORWARD, FFTW_MEASURE);
    ifft = fftw_plan_dft_2d(simulation_resolution_x, simulation_resolution_y, psi_transform, psi, FFTW_BACKWARD, FFTW_MEASURE);

    potential = (double*) malloc(Resolutionx * Resolutiony * sizeof(double));
    simulation_state=simulation_state_wait_for_restart;
}

void simulation_dealloc(){
    free(prop);
    free(psi_transform);
    free(psi);

    free(animation_start);
    free(animation_end);

    free(potential);
    simulation_state=simulation_state_not_allocated;
}

int simulation_redraw_wave(int offset_x,int offset_y,float angle,float momentum,float gauss_width){
    if(simulation_state==simulation_state_measurement_animation){
        printf("Warn: Measurement animation still running, can't redraw wave!\n");
        return 1;
    }else if(simulation_state==simulation_state_simulate){
        printf("Warn: Simulation still running, can't redraw wave!\n");
        return 1;
    }else if(simulation_state==simulation_state_not_allocated){
        printf("Error: FFTW arrays not allocated!\n");
        return 2;
    }
    //for gauss function will be cut off to increase performance at redraw
    int cutSquareHalf=(int)(gauss_width*gauss_width*5.0f);
    memset(&(psi[0][0]),0,simulation_resolution_total*4*sizeof(float));
    for(int j = 0; j < simulation_resolution_y; j++) {
        for(int i = 0; i < simulation_resolution_x; i++) {
            int gauss_x_squared=(i-offset_x)*(i-offset_x);
            int gauss_y_squared=(i-offset_y)*(i-offset_y);
            int gauss_r_squared=gauss_x_squared+gauss_y_squared;
            if(gauss_r_squared<cutSquareHalf){
                psi[i+j*simulation_resolution_x][0]=exp(-gauss_r_squared/(2*gauss_width*gauss_width)) * cos((i - simulation_resolution_x / 2.0f) * cos(angle) + ((j - simulation_resolution_y / 2.0f) * sin(angle)) * momentum);
                psi[i+j*simulation_resolution_x][1]=exp(-gauss_r_squared/(2*gauss_width*gauss_width)) * sin((i - simulation_resolution_x / 2.0f) * cos(angle) + ((j - simulation_resolution_y / 2.0f) * sin(angle)) * momentum);
            }
        }
    }
    simulation_state=simulation_state_created_and_wait_for_start;
}

int simulation_run(float dt){
    static float dt_old=0.f;
    if(dt_old!=dt){ //Generate new momentum prop
        for(int y = 0; y < simulation_resolution_y / 2; y++) {
            for(int x = 0; x < simulation_resolution_x / 2; x++) { //e^(-*p^2/2m) from Hamilton
                prop[y * simulation_resolution_x + x][0] = cos(dt * (-x * x - y * y));
                prop[y * simulation_resolution_x + x][1] = sin(dt * (-x * x - y * y));
            }
            for(int x = simulation_resolution_x / 2; x < simulation_resolution_x; x++) {
                prop[y * simulation_resolution_x + x][0] = cos(dt * (-(x - simulation_resolution_x) * (x - simulation_resolution_x) - y * y));
                prop[y * simulation_resolution_x + x][1] = sin(dt * (-(x - simulation_resolution_x) * (x - simulation_resolution_x) - y * y));
            }
        }
        for(int y = simulation_resolution_y / 2; y < simulation_resolution_y; y++) {
            for(int x = 0; x < simulation_resolution_x / 2; x++) {
                prop[y * simulation_resolution_x + x][0] = cos(dt * (-x * x - (y - simulation_resolution_y) * (y - simulation_resolution_y)));
                prop[y * simulation_resolution_x + x][1] = sin(dt * (-x * x - (y - simulation_resolution_y) * (y - simulation_resolution_y)));
            }
            for(int x = simulation_resolution_x / 2; x < simulation_resolution_x; x++) {
                prop[y * simulation_resolution_x + x][0] = cos(dt * (-(x - simulation_resolution_x) * (x - simulation_resolution_x) - (y - simulation_resolution_y) * (y - simulation_resolution_y)));
                prop[y * simulation_resolution_x + x][1] = sin(dt * (-(x - simulation_resolution_x) * (x - simulation_resolution_x) - (y - simulation_resolution_y) * (y - simulation_resolution_y)));
            }
        }
        dt_old=dt;
    }
    if(simulation_state!=simulation_state_created_and_wait_for_start){
        printf("Error: Wave packet not initialized, won't start simulation!\n");
        return 1;
    }
    //Change to momentum space (same as fourier transform)
    fftw_execute(fft);
    //Complex multiplication of the wave function in the momentum space with the squared momentum propagator e^(1/(i*hbar)*)?? TODO
    //Which applies the -p^2/2*m of the Hamilton Operator H=(-p^2/2*m+V(x))
    for(int i = 0; i < simulation_resolution_total; i++) {
        double psi_re_temp = psi_transform[i][0];
        psi_transform[i][0] = psi_re_temp * prop[i][0] - psi_transform[i][1] * prop[i][1];
        psi_transform[i][1] = psi_re_temp * prop[i][1] + psi_transform[i][1] * prop[i][0];
    }
    //Change back to position space
    fftw_execute(ifft);
    //apply the potential part of the Hamilton operator
    for(int i = 0; i < simulation_resolution_total; i++) {
        double psi_re_temp = psi[i][0];
        psi[i][0] = psi_re_temp * cos(potential[i]) - psi[i][1] * sin(potential[i]);
        psi[i][1] = psi_re_temp * sin(potential[i]) + psi[i][1] * cos(potential[i]);
    }
    return 0;
}

int simulation_animate_measurement(){
    //Internal variable which holds information of what part of the animation will get executed in this frame
    static int AnimationStep=0;
    if(!AnimationStep) { //first frame of animation, will do the measurement
        srand((long)(10000.0f * glfwGetTime()));
        double random = (rand() % 1001) / 1000.0f;
        double sum = 0;
        double norm_sum = 0;
        for(int i = 0; i < simulation_resolution_x * simulation_resolution_y; i++) {
            norm_sum = norm_sum + (psi[i][0] * psi[i][0] + psi[i][1] * psi[i][1]);
        }
        int measurement_pos=0;
        for(; measurement_pos < simulation_resolution_total; measurement_pos++) {
            sum = sum + ((psi[measurement_pos][0] * psi[measurement_pos][0] + psi[measurement_pos][1] * psi[measurement_pos][1]) / norm_sum);
            if(sum > random) {
                printf("Sum:  %f\n",sum);
                printf("Rand: %f\n",random);
                printf("Norm %f\n",norm_sum);
                break;
            }
        }
        if((measurement_pos%simulation_resolution_x)>((VertMinX+0.5f)*simulation_resolution_x)&&(measurement_pos%simulation_resolution_x)<((VertMaxX+0.5f)*simulation_resolution_x)&&(measurement_pos/simulation_resolution_y)>((VertMinY+0.5f)*simulation_resolution_y)&&(measurement_pos/simulation_resolution_y)<((VertMaxY+0.5f)*Resolutiony)){
            ColorIntensity=1.99f;
            printf("HIT\n\n\n");
        }
        else{
            ColorIntensity=0.99f;
            printf("NO HIT\n\n\n");
        }
        //Animation for mess
        for(int x = 0; x<simulation_resolution_total;x++){
            animation_start[x][0]=psi_transform[x][0];
            animation_start[x][1]=psi_transform[x][1];
            if((((measurement_pos % simulation_resolution_x) - x%simulation_resolution_x) * ((measurement_pos % simulation_resolution_x) - x%simulation_resolution_x) + ((measurement_pos / simulation_resolution_y) - x/simulation_resolution_y) * ((measurement_pos / simulation_resolution_y) - x/simulation_resolution_y))<10.0f){
                psi[x][0]=1.0f;
            }else{
                psi[x][0]=0.0f;
            }
            psi[x][1]=0.0f;
        }
        fftw_execute(fft);
        for(int x = 0; x<simulation_resolution_total;x++){
            animation_end[x][0]=psi_transform[x][0];
            animation_end[x][1]=psi_transform[x][1];
        }
        AnimationStep=0;
    }
    for(int x = 0; x<simulation_resolution_total;x++){
        psi_transform[x][0]=animation_start[x][0]*(1.0f-AnimationStep/30.0f)+animation_end[x][0]*(AnimationStep/30.0f);
        psi_transform[x][1]=animation_start[x][1]*(1.0f-AnimationStep/30.0f)+animation_end[x][1]*(AnimationStep/30.0f);
    }
    fftw_execute(ifft);
    AnimationStep++;
    if(AnimationStep==30){
        AnimationStep=0;
        simulation_state=simulation_state_wait_for_restart;
    }
}

void simulation_load_potential(uint8_t* pot){
    for(int i = 0; i < simulation_resolution_total; i++) {
        potential[i] = (255 - pot[4 * i + 1]) / 255.0f;
    }
}

