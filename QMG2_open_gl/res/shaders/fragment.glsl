#version 430 core
in vec2 UV;
in float potential_true_frag;
//uniform sampler2D texture0;
out vec4 frag_color;
uniform sampler2D texture0;
void main(){
    float re=(texture(texture0,UV).b)*2-1;
    float im=(texture(texture0,UV).g)*2-1;
    //float re=(texture(texture0,UV).b)*2-1;
    //float im=(texture(texture0,UV).g)*2-1;
    float pot=potential_true_frag*texture(texture0,UV).r;

    float intens=(re*re+im*im);
    intens=intens*intens; //make it brighter
    frag_color=vec4(sqrt(re*re),pot,sqrt(im*im),1.0f);
}
