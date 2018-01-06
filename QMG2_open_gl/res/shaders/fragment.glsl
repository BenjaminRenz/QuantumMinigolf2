#version 430 core
in vec2 UV;
//uniform sampler2D texture0;
out vec4 frag_color;
uniform sampler2D texture0;
void main(){
    frag_color=vec4(texture(texture0,UV).rgb,1.0f);
}
