#version 430 core
out vec4 frag_color;
in vec2 uv_pass_to_frag;
uniform sampler2D texture1;
void main(){
    frag_color=vec4(texture(texture1,uv_pass_to_frag));
}
