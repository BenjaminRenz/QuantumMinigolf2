int simulation_state;
int simulation_resolution_x;
int simulation_resolution_y;
enum {simulation_state_simulate,simulation_state_measurement_animation,simulation_state_created_and_wait_for_start,simulation_state_wait_for_restart};


int simulation_redraw_wave(int offset_x,int offset_y,float angle,float momentum,float gauss_width){
    if(simulation_state==simulation_state_measurement_animation){
        printf("Error: Measurement animation still running, can't redraw wave!\n");
        return 1;
    }else if(simulation_state==simulation_state_simulate){
        printf("Error: Simulation still running, can't redraw wave!\n");
        return 1;
    }
    //TODO make dependent on gauss_width
    memset(&(psi[0][0]),0,simulation_resolution_y*simulation_resolution_y*4*sizeof(float));
    for(int j = 0; j < simulation_resolution_y; j++) {
        for(int i = 0; i < simulation_resolution_y; i++) {
            if((abs(i-((int)wave_offset_x)))<(simulation_resolution_y/10.0f)&&(abs(j-((int)wave_offset_y))<(simulation_resolution_y/10.0f))){
                psi[i + j * simulation_resolution_x][0] = exp(-((i - ((int)wave_offset_x)) * (i - ((int)wave_offset_x)) / wave_proportion + (j - ((int)wave_offset_y)) * (j - ((int)wave_offset_y))) / (gauss_width)) * cos((i - simulation_resolution_x / 2.0f) * cos(angle) + ((j - simulation_resolution_y / 2.0f) * sin(angle)) * momentum_multi);
                psi[i + j * simulation_resolution_x][1] = exp(-((i - ((int)wave_offset_x)) * (i - ((int)wave_offset_x)) / wave_proportion + (j - ((int)wave_offset_y)) * (j - ((int)wave_offset_y))) / (gauss_width)) * sin((i - simulation_resolution_x / 2.0f) * cos(angle) + ((j - simulation_resolution_y / 2.0f) * sin(angle)) * momentum_multi);
            }
        }
    }

    simulation_state=simulation_state_redrawn_and_wait_for_start;
}


int simulation_run(){
    if(simulation_state!=simulation_state_created_and_wait_for_start){
        printf("Error: Wave packet not initialized, won't start simulation!\n");
        return 1;
    }
    //Change to momentum space (same as fourier transform)
    fftw_execute(fft);
    //Complex multiplication of the wave fuction in the momentum space with the sqared momentum propagator e^(1/(i*hbar)*)?? TODO
    //Which applies the -p^2/2*m of the Hamilton Operator H=(-p^2/2*m+V(x))
    for(int i = 0; i < simulation_resolution_x * simulation_resolution_y; i++) {
        double psi_re_temp = psi_transform[i][0];
        psi_transform[i][0] = psi_re_temp * prop[i][0] - psi_transform[i][1] * prop[i][1];
        psi_transform[i][1] = psi_re_temp * prop[i][1] + psi_transform[i][1] * prop[i][0];
    }
    //Change back to position space
    fftw_execute(ifft);
    //apply the potential part of the Hamilton operator
    for(int i = 0; i < simulation_resolution_x * simulation_resolution_y; i++) {
        double psi_re_temp = psi[i][0];
        psi[i][0] = psi_re_temp * cos(potential[i]) - psi[i][1] * sin(potential[i]);
        psi[i][1] = psi_re_temp * sin(potential[i]) + psi[i][1] * cos(potential[i]);
    }
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
        for(pos = 0; pos < simulation_resolution_x * simulation_resolution_y; pos++) {
            sum = sum + ((psi[pos][0] * psi[pos][0] + psi[pos][1] * psi[pos][1]) / norm_sum);
            if(sum > random) {
                printf("Sum:  %f\n",sum);
                printf("Rand: %f\n",random);
                printf("Norm %f\n",norm_sum);
                break;
            }
        }
        if((pos%simulation_resolution_x)>((VertMinX+0.5f)*simulation_resolution_x)&&(pos%simulation_resolution_x)<((VertMaxX+0.5f)*simulation_resolution_x)&&(pos/simulation_resolution_y)>((VertMinY+0.5f)*simulation_resolution_y)&&(pos/simulation_resolution_y)<((VertMaxY+0.5f)*Resolutiony)){
            ColorIntensity=1.99f;
            printf("HIT\n\n\n");
        }
        else{
            ColorIntensity=0.99f;
            printf("NO HIT\n\n\n");
        }
        //Animation for mess
        for(int x = 0; x<(simulation_resolution_x*simulation_resolution_y-1);x++){
            animation_start[x][0]=psi_transform[x][0];
            animation_start[x][1]=psi_transform[x][1];
            if((((pos % simulation_resolution_x) - x%simulation_resolution_x) * ((pos % simulation_resolution_x) - x%simulation_resolution_x) + ((pos / simulation_resolution_y) - x/simulation_resolution_y) * ((pos / simulation_resolution_y) - x/simulation_resolution_y))<10.0f){
                psi[x][0]=1.0f;
            }else{
                psi[x][0]=0.0f;
            }
            psi[x][1]=0.0f;
        }
        fftw_execute(fft);
        for(int x = 0; x<(simulation_resolution_x*simulation_resolution_y-1);x++){
            animation_end[x][0]=psi_transform[x][0];
            animation_end[x][1]=psi_transform[x][1];
        }
        AnimationStep=0;
    }
    for(int x = 0; x<(simulation_resolution_x*simulation_resolution_y-1);x++){
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
