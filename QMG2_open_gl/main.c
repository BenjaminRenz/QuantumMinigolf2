/*TODO list
-compile GLEW as static library
-implement webcam brightest spot detection https://www.pyimagesearch.com/2014/09/29/finding-brightest-spot-image-using-python-opencv/
*/
#define GLEW_STATIC
#include "libraries/GLEW_2.1.0/include/glew.h"
#include "libraries/GLFW_3.2.1/include/glfw3.h"
#include "libraries/FFTW_3.3.5/include/fftw3.h"
#include "libraries/LINMATH/include/linmath.h"

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#define PI 3.14159265358979323846
#include <limits.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button,int action, int mods);
void drop_file_callback(GLFWwindow* window, int count, const char** paths);
void mouse_scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void createPlaneVBO();
void createCube();
void write_bmp(char* filepath, unsigned int width, unsigned int height);
float update_delta_time();
void APIENTRY openglCallbackFunction(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam);
void glfw_error_callback(int error, const char* description);
GLuint CompileShaderFromFile(char FilePath[] ,GLuint shaderType);
//global variables section
float FOV=0.20f;
unsigned int Resolution= 100;
GLFWwindow* MainWindow;


int main(int argc, char* argv[]){
    //GLFW init
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()){
        return -1;
    }

    //Set window creation hints
    //glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GL_TRUE);

    //window creation
    MainWindow = glfwCreateWindow(600, 400, "Quantum Minigolf 2.0", NULL, NULL);
    //GLFWwindow* MainWindow = glfwCreateWindow(1920, 1080, "Quantum Minigolf 2.0", glfwGetPrimaryMonitor(), NULL);
    if (!MainWindow){
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(MainWindow);

    //GLEW init
    glewExperimental=GL_TRUE;
    GLenum err = glewInit();
    if(GLEW_OK != err){
        printf("Error: glewInit() failed.");
    }
    printf("QuantumMinigolf v2 opengl:\n");
    printf("using OpenGl Version: %s\n",glGetString(GL_VERSION));
    //enable v-sync
    glfwSwapInterval(1);
    //Refister Callback for errors (debugging)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(openglCallbackFunction,0);
    glDebugMessageControl(GL_DONT_CARE,GL_DONT_CARE,GL_DONT_CARE,0,NULL,GL_TRUE); //Dont filter messages

    //Register Callbacks for user input
    glfwSetKeyCallback(MainWindow,key_callback);
    glfwSetMouseButtonCallback(MainWindow, mouse_button_callback);
    glfwSetDropCallback(MainWindow,drop_file_callback);
    glfwSetScrollCallback(MainWindow,mouse_scroll_callback);
    //Get window height
    int window_width = 0;
    int window_height = 0;
    glfwGetWindowSize(MainWindow,&window_width,&window_height);

    //Initialize shaders
    //TODO filepath for windows, alter for unix like os
    //GLuint computeShaderId = CompileShaderFromFile(".\\res\\shaders\\compute.glsl",GL_COMPUTE_SHADER);
    GLuint vertexShaderId = CompileShaderFromFile(".\\res\\shaders\\vertex.glsl",GL_VERTEX_SHADER);
    GLuint fragmentShaderId = CompileShaderFromFile(".\\res\\shaders\\fragment.glsl",GL_FRAGMENT_SHADER);
    GLuint ProgrammID = glCreateProgram();              //create program to run on GPU
    glAttachShader(ProgrammID, vertexShaderId);         //attach vertex shader to new program
    glAttachShader(ProgrammID, fragmentShaderId);       //attach fragment shader to new program
    glLinkProgram(ProgrammID);                          //create execuatble
    glUseProgram(ProgrammID);
    GLint MVPmatrix=glGetUniformLocation(ProgrammID,"MVPmatrix");//only callable after glUseProgramm has been called once

    createPlaneVBO();
    glDisable(GL_CULL_FACE);
    glClearColor(0.3f,0.3f,0.3f,0.5f);

/* https://www.seas.upenn.edu/%7Epcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
    glActiveTexture(GL_TEXTURE0);
    GLuint psi_texture=0;
    glGenTextures(1,&psi_texture);
    glBindTexture(GL_TEXTURE_2D,psi_texture);
    //generate pbo
    GLuint PBO1=0;
    GLuint PBO2=0;
    glGenBuffers(1,&PBO1);
    glGenBuffers(1,&PBO2);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,PBO1);
    glBufferData(GL_PIXEL_UNPACK_BUFFER,"sizeof data","datapointer",GL_STREAM_DRAW);
    glTexImage2D(GL_TEXTURE_2D,0,0,0,Resolution,Resolution,GL_BGRA,GL_UNSIGNED_INT_8_8_8_8_REV,NULL); //NULL pointer let opengl fetch data from bound GL_PIXEL_UNPACK_BUFFER
    {//in render loop
        //enable pbo1 to be uesed in the current render
        glBindTexture(GL_TEXTURE_2D, psi_texture);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,PBO1);
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,Resolution,Resolution,GL_BGRA,GL_UNSIGNED_INT_8_8_8_8,NULL);
        //Make PBO2 ready to recieve new data
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,PBO2);
        glBufferData(GL_PIXEL_UNPACK_BUFFER,"bytecount",0,GL_STREAM_DRAW);
        GLuint* data = glMapBuffer(GL_PIXEL_UNPACK_BUFFER,0,"bytecount",GL_MAP_WRITE_BIT);//Map buffer on gpu to client address space : offset 0,data,allow to write to buffer
        //write to data
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); //start upload
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
        {//swap pixel buffers
            GLuint temp=PBO1;
            PBO1=PBO2;
            PBO2=temp;
        }


    }
*/
    //glTexSubImage2D(GL_TEXTURE_2D,,0,0,0,Resolution,Resolution,GL_UNSIGNED_INT_8_8_8_8_REV);//4 upadte every frame
    double rotation_up_down=0;
    double rotation_left_right=0.1f;

    mat4x4 mvp4x4;
    mat4x4 persp4x4;
    vec3 eye_vec={1.0f,1.0f,1.0f};
    vec3 cent_vec={0.0f,0.0f,0.0f};
    vec3 up_vec={0.0f,0.0f,1.0f};
    write_bmp(".\\test.bin",0,0);
    while (!glfwWindowShouldClose(MainWindow)){
        float delta_time = update_delta_time();
        //Camera Movement calculation
        if(glfwGetKey(MainWindow,GLFW_KEY_W)==GLFW_PRESS){
            if(rotation_up_down<(3.0)){
                rotation_up_down=rotation_up_down+delta_time;
            }
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_S)==GLFW_PRESS){
           if(rotation_up_down>(-3.0)){
                rotation_up_down=rotation_up_down-delta_time;
           }
        }
        //atan(rotation_up_down);
        if(glfwGetKey(MainWindow,GLFW_KEY_D)==GLFW_PRESS){
            if(rotation_left_right>(-PI)){
                rotation_left_right=rotation_left_right-delta_time;
            }else{
                rotation_left_right=PI;
            }
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_A)==GLFW_PRESS){
             if(rotation_left_right<PI){
                rotation_left_right=rotation_left_right+delta_time;
            }else{
                rotation_left_right=-PI;
            }
        }
        //camera projection an transformation matrix calculation
        eye_vec[0]=1.5f*sin(rotation_left_right)*cos(atan(rotation_up_down));
        eye_vec[1]=1.5f*cos(rotation_left_right)*cos(atan(rotation_up_down));
        eye_vec[2]=1.5f*sin(atan(rotation_up_down));
        mat4x4_look_at(mvp4x4,eye_vec,cent_vec,up_vec);
        mat4x4_perspective(persp4x4,FOV,16.0f/9.0f,0.5f,10.0f);
        mat4x4_mul(mvp4x4,persp4x4,mvp4x4);
        glUniformMatrix4fv(MVPmatrix,1,GL_FALSE,(GLfloat*)mvp4x4);
        //atan(rotation_up_down)
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_LINES,6*Resolution*Resolution,GL_UNSIGNED_INT,0);
        //Swap Buffers
        glfwSwapBuffers(MainWindow);
        //Process Events
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}



float update_delta_time(){              //Get the current time with glfwGetTime and subtract last time to return deltatime
    static double last_glfw_time=0.0f;
    static double current_glfw_time;
    current_glfw_time = glfwGetTime();
    float delta = (float) (current_glfw_time-last_glfw_time);
    last_glfw_time = current_glfw_time;
    return delta;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(key==GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(MainWindow,1);
    }
    //TODO: move key checking routines here.
}
void mouse_button_callback(GLFWwindow* window, int button,int action, int mods){
    if(button== GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS){
        printf("LMB Down");
    }
}

GLuint CompileShaderFromFile(char FilePath[] ,GLuint shaderType){
    //read from file into heap memory
    FILE* filepointer=fopen(FilePath,"rb");                  //open specified file in read only mode
    if(filepointer==NULL){
        printf("Error: Filepointer to shaderfile at %s could not be loaded.",FilePath);
        //return;
    }
    fseek(filepointer,0,SEEK_END);                          //shift filePointer to EndOfFile Position to get filelength
    long filelength = ftell(filepointer);                   //get filePointer position
    fseek(filepointer,0,SEEK_SET);                          //move file Pointer back to first line of file
    char* filestring = (char*)malloc(filelength+1);         //
    if(fread(filestring,sizeof(char),filelength,filepointer) != filelength){
        printf("Error: Missing characters in input string");
        //return;
    }
    if(filestring[0]==0xEF&&filestring[1]==0xBB&&filestring[2]==0xBF){   //Detect if file is utf8 with bom
        printf("Error: Remove the bom from your utf8 shader file");
    }
    filestring[filelength]=0;                             //Set end of string
    fclose(filepointer);                                  //Close File
    const char* ConstFilePointer = filestring;            //opengl wants const pointer
    //compile shader with opengl
    GLuint ShaderId= glCreateShader(shaderType);
    glShaderSource(ShaderId,1,&ConstFilePointer,NULL);
    glCompileShader(ShaderId);
    GLint compStatus = 0;
    glGetShaderiv(ShaderId,GL_COMPILE_STATUS,&compStatus);
    if(compStatus!=GL_TRUE){
        printf("Error: Compilation of shader %d failed!\n",ShaderId);
        //TODO free resources
        //return;
    }
    printf("Info: Shader %d sucessfully compiled.\n",ShaderId);
    free(filestring);                                   //Delete Shader string from heap
    fclose(filepointer);
    return ShaderId;
}

void glfw_error_callback(int error, const char* description){
    printf("Error in glfw %d occured\nDescription: %s\n",error,description);
}

void APIENTRY openglCallbackFunction(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam){
    printf("Error in opengl occured!\n");
    printf("Message: %s\n",message);
    printf("type or error: ");
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        printf("ERROR");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        printf("DEPRECATED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        printf("UNDEFINED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        printf("PORTABILITY");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        printf("PERFORMANCE");
        break;
    case GL_DEBUG_TYPE_OTHER:
         printf("OTHER");
        break;
    }
    printf("\nId:%d \n",id);
    printf("Severity:");
    switch (severity){
    case GL_DEBUG_SEVERITY_LOW:
        printf("LOW");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        printf("MEDIUM");
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        printf("HIGH");
        break;
    }
    printf("\nGLerror end\n");
}

void createPlaneVBO(){
    GLuint VaoId=0;
    GLuint VboPositionsId=0;
    GLuint VboIndicesId=0;

    //Generate Vertex Array Object
    glGenVertexArrays(1,&VaoId);
    glBindVertexArray(VaoId);

    //Generate Vertex Positions
    float* plane_vertex_data =malloc(3*Resolution*Resolution*sizeof(float));      //TODO free this pointer ( memory leak )
    long vert_index=0;
    for(int y=0;y<Resolution;y++){
        for(int x=0;x<Resolution;x++){
            //Vector coordinates (x,y,z)
            plane_vertex_data[vert_index++]=(((float)x)/(Resolution-1))-0.5f;
            plane_vertex_data[vert_index++]=(((float)y)/(Resolution-1))-0.5f; //Set height (y) to zero
        }
    }
    glGenBuffers(1, &VboPositionsId);                                                          //create buffer
    glBindBuffer(GL_ARRAY_BUFFER, VboPositionsId);                                            //Link buffer
    glBufferData(GL_ARRAY_BUFFER, Resolution*Resolution*2*sizeof(float),plane_vertex_data,GL_STATIC_DRAW);    //Upload data to Buffer, Vertex data is set only once and drawn regularly, hence we use GL_STATIC_DRAW
    glEnableVertexAttribArray(0);//x,y,z
    glVertexAttribPointer(0,2,GL_FLOAT, GL_FALSE,2*sizeof(float),0);
    //Generate Vertex Indices
    GLuint* plane_indices = malloc((Resolution-1)*(Resolution-1)*6*sizeof(GLuint));         //TODO free allocated Memory
    vert_index=0;
    for(unsigned int y=0;y<(Resolution-1);y++){
        for(unsigned int x=0;x<(Resolution-1);x++){
            //Generate first triangle
            plane_indices[vert_index++]=x+(y*Resolution);   //Vertex lower left first triangle
            plane_indices[vert_index++]=x+1+(y*Resolution);//Vertex upper right first triangle
            plane_indices[vert_index++]=x+((y+1)*Resolution); //Vertex upper left first triangle
            //Generate second triangle
            plane_indices[vert_index++]=(x+1)+(y*Resolution);   //Vertex lower left second triangle
            plane_indices[vert_index++]=(x+1)+((y+1)*Resolution); //Vertex lower right second triangle
            plane_indices[vert_index++]=x+((y+1)*Resolution); //Vertex upper right first triangle
        }
    }
    glGenBuffers(1, &VboIndicesId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VboIndicesId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,(Resolution-1)*(Resolution-1)*6*sizeof(GLuint),plane_indices,GL_STATIC_DRAW);
}



void drop_file_callback(GLFWwindow* window, int count, const char** paths){
    for(int i=0; i<count; i++){
        printf("Dropped File Path: %s\n",paths[i]);
    }
}

void mouse_scroll_callback(GLFWwindow* window, double xOffset, double yOffset){
    float temp_mouse_scroll= -0.04f*(float)yOffset;
    if((FOV+temp_mouse_scroll<1.0f)&&(FOV+temp_mouse_scroll>0.1f)){
        FOV+=temp_mouse_scroll;
    }else{
    }
}

//Functions for bitmap read and write
/*
typedef struct bmp_header{
    //header
    uint8_t bmp_magic_num[2];
    uint32_t bmp_filesize;
    unsigned int bmp_offset_to_pixelarray;
    //v5 header
    unsigned int bmp_info_header_size;
    unsigned int bmp_width;
    unsigned int height;
    short bmp_planecount:
    short bmp_bits_per_pixel;
    unsigned int bmp_compression;
    unsigned int bmp_image_size;
    unsigned int bmp_X_PpMeter;
    unsigned int bmp_Y_PpMeter;
    unsigned int bmp_col_num;
    unsigned int bmp_col_important;
}bmp_header;
*/
typedef struct Image{
    unsigned int sizeX;
    unsigned int sizeY;
    void* data;
}Image;




unsigned int read_uint_from_endian_file(FILE* file){
    unsigned char data[4];
    unsigned int data_return_int;
    if(fread(data,1,4,file)<4){ //total number of read elements is less than 4
        return 0;
    }
    //little endian
    data_return_int=(b[3]<<24)|(b[2]<<16)|(b[1]<<8)|b[0];
    //big endian (comment out and comment little endian if needed)
    //data_return_int=(b[0]<<24)|(b[1]<<16)|(b[2]<<8)|b[3];
    return data_return_int;
}

unsigned short read_short_from_endian_file(FILE* file){
    unsigned char data[2];
    unsigned short data_return_short;
    if(fread(data,1,2,file)<2){ //total number of read elements is less than 4
        return 0;
    }
    //little endian
    data_return_short=(b[1]<<8)|b[0];
    //big endian (comment out and comment little endian if needed)
    //data_return_int=(b[0]<<8)|b[1];
    return data_return_short;
}

void read_bmp(char* filepath,Image* bmp_image){
    FILE *filepointer=fopen(filepath,"rb");
    if(filepointer==NULL){
        printf("File :%d could not be found\n",filepath);
        return;
    }
    if(read_short_from_endian_file(filepointer)!=0x424D){// (equals bm)
        printf("File :%d is not an BMP\n");
        return;
    }
    fseek(filepointer,14 ,SEEK_SET);
    unsigned int BitmapInfoHeaderSize = read_uint_from_endian_file(filepointer);
    unsigned int BitmapWidth=read_uint_from_endian_file(filepointer);
    printf("BitmapWidth is %d.\n",BitmapWidth);
    unsigned int BitmapHeight=read_uint_from_endian_file(filepointer);
    printf("BitmapHeight is %d.\n",BitmapHeight);
    printf("Calculated Image Size.\n")
    if(read_short_from_endian_file(filepointer)!=1){
        printf("Unsupported plane count\n");
        return;
    }
    if(read_short_from_endian_file(filepointer)!=24){
        printf("Unsupported color depth, should be 24.\n");
        return;
    }
    if(read_uint_from_endian_file(filepointer)!=0){
        printf("Does not support compressed bmp files.\n");
        return;
    }
    unsigned int BitmapImageSize=read_uint_from_endian_file(filepointer);
    unsigned int BitmapXPpM=read_uint_from_endian_file(filepointer);
    unsigned int BitmapYPpM=read_uint_from_endian_file(filepointer);
    void* imageData=malloc()
    fclose(filepointer);
}




void write_bmp(char* filepath, unsigned int width, unsigned int height){
    FILE* filepointer = fopen(filepath,"wb");
    //bytes_per_line=(3*(width+1)/4)*4;
    const char* String_to_write="BMP";
    fwrite(&String_to_write,sizeof(char),3,filepointer);
    return;
}
