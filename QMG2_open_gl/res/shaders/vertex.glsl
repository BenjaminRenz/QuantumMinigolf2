#version 430 core
layout(location = 0) in vec2 position;
out vec2 UV_out;
uniform mat4 MVPmatrix;
//uniform sampler2D psiTexture;
void main(){
    gl_Position = MVPmatrix*vec4(position,0.0f,1.0f);
    UV_out = position+vec2(0.5f,0.5f);
    //gl_Position = MVPmatrix*(vec4(position,texture(tex,UV_out),1.0f));

}
