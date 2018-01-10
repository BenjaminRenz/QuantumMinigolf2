#version 430 core
layout(location = 0) in vec2 position;
uniform mat4 MVPmatrix;
out vec2 UV;
uniform sampler2D texture0;
void main(){
    UV=position.xy+vec2(0.5f,0.5f);
    float re=(texture(texture0,UV).b)*2-1;
    float im=(texture(texture0,UV).g)*2-1;
    gl_Position = MVPmatrix*vec4(position,(re*re+im*im)/10,1.0f);//(re*re+im*im)/10.0f,1.0f);
}
