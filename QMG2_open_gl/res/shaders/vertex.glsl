#version 430 core
layout(location = 0) in vec2 position;
uniform mat4 MVPmatrix;
out vec2 UV_out;
uniform sampler2D texture0;
void main(){
    UV_out=position+vec2(0.5f,0.5f);
    gl_Position = MVPmatrix*vec4(position,texture(texture0,UV_out).a/10.0f,1.0f);

    //UV_out = position+vec2(0.5f,0.5f);
    //gl_Position = MVPmatrix*(vec4(position,texture(texture0,UV_out),1.0f));
}
