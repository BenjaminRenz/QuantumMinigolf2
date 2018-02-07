#version 430 core
in vec2 UV;
uniform sampler2D texture1;
void main(){
    frag_color=vec4(texture(texture1,UV));
}
