#version 430 core
out vec4 frag_color;
uniform float Intensity;
void main(){
    if(Intensity<0.0f){
        frag_color=vec4(1.0f,0.0f,0.0f,abs(Intensity));
    }else{
        frag_color=vec4(0.0f,1.0f,0.0f,Intensity);
    }
}
