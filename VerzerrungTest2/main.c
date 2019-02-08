#include "map_camera_plane.h"
#include <stdio.h>
void main(void){
    mat3x3 CalibMat;
    float CalibPoint[8]={0.f,0.f,50.f,0.f,50.f,50.f,0.f,50.f};
    camera_perspec_calibrating(CalibMat,CalibPoint);
    vec2 testpointOut;
    vec2 testpointIn={25.f,25.f};
    camera_perspec_map_point(testpointOut,CalibMat,testpointIn);
    printf("Debug: %f, %f\n",testpointOut[0],testpointOut[1]);
}
