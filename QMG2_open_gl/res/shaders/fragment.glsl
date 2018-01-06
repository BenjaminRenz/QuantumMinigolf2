#version 430 core
in vec2 uv_in;
//uniform sampler2D texture0;
out vec4 frag_color;

void main(){
    frag_color=vec4(uv_in.xy,1.0f,1.0f);
}
