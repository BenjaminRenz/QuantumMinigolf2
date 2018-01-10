#version 430 core
in vec2 UV;
//uniform sampler2D texture0;
out vec4 frag_color;
uniform sampler2D texture0;
void main(){
    float re=texture(texture0,UV).b;
    float im=texture(texture0,UV).g;
    frag_color=vec4(re*re+im*im,0.0f,0.0f,1.0f);
}
