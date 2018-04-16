#version 430 core
out vec4 frag_color;
uniform float Intensity;
void main(){
    if(Intensity>2.0f){
        frag_color= vec4(Intensity-2.0f,Intensity-2.0f,0.0f,0.0f);
    }else if(Intensity>1.0f){
        frag_color= vec4(0.0f,Intensity-1.0f,0.0f,0.0f);
    }else{
        frag_color= vec4(Intensity,0.0f,0.0f,0.0f);
    }
}
