#version 430 core
layout(location = 0) in vec3 position;
uniform mat4 MVPmatrix;
varying vec4 ColorIntensity;
void main(){
    gl_Position = MVPmatrix*vec4(position,1.0f);

}
