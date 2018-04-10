#version 430 core
layout(location = 0) in vec3 position;
uniform mat4 MVPmatrix;
uniform float Intensity;
void main(){
    gl_Position = MVPmatrix*vec4(position,1.0f);
}
