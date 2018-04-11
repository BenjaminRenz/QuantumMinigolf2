#version 430 core
out vec4 frag_color;
uniform float Intensity;
varying vec4 ColorIntensity;
void main(){
    frag_color=vec4(ColorIntensity);
}
