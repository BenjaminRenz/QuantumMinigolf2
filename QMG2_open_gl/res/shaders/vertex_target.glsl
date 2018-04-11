#version 430 core
layout(location = 0) in vec3 position;
uniform mat4 MVPmatrix;
uniform float Intensity;
varying vec4 ColorIntensity;
void main(){
    if(position.z>0.002f){
        ColorIntensity = vec4(0.0f,0.0f,0.0f,0.0f);
    }else{
        if(Intensity>2.0f){
            ColorIntensity = vec4(0.0f,0.0f,1.0f,Intensity-2.0f);
        }else if(Intensity>1.0f){
            ColorIntensity = vec4(0.0f,1.0f,0.0f,Intensity-1.0f);
        }else{
            ColorIntensity = vec4(1.0f,0.0f,0.0f,Intensity);
        }
    }
    gl_Position = MVPmatrix*vec4(position,1.0f);

}
