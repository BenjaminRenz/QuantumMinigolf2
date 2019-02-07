#include "map_camera_plane.h"
#include <stdio.h>
void main(void){
    mat3x3 CalibMat;
    float CalibPoint[8]={0.f,0.f,50.f,10.f,80.f,90.f,10.f,100.f};
    camera_perspec_calibrating(CalibMat,CalibPoint);
    vec2 testpointOut;
    vec2 testpointIn={50.f,10.f};
    camera_perspec_map_point(testpointOut,CalibMat,testpointOut);
    printf("Debug: %f, %f\n",testpointOut[0],testpointOut[1]);
}
