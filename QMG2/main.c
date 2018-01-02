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
void mouse_button_callback();
void createPlaneVBO();
void createCube();
float update_delta_time();
void APIENTRY openglCallbackFunction(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam);
void glfw_error_callback(int error, const char* description);;
GLuint CompileShaderFromFile(char FilePath[] ,GLuint shaderType);

int main(int argc, char* argv[]){
    //GLFW init
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()){
        return -1;
    }

    //Set window creation hints
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GL_TRUE);

    //window creation
    GLFWwindow* MainWindow = glfwCreateWindow(600, 400, "Quantum Minigolf 2.0", NULL, NULL);

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
    printf("QuantumMinigolf v2:\n");
    printf("using OpenGl Version: %s\n",glGetString(GL_VERSION));

    //Refister Callback for errors (debugging)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(openglCallbackFunction,0);
    glDebugMessageControl(GL_DONT_CARE,GL_DONT_CARE,GL_DONT_CARE,0,NULL,GL_TRUE); //Dont filter messages

    //Register Callbacks for user input
    glfwSetKeyCallback(MainWindow,key_callback);
    glfwSetMouseButtonCallback(MainWindow, mouse_button_callback);

    //Get window height
    int window_width = 0;
    int window_height = 0;
    glfwGetWindowSize(MainWindow,&window_width,&window_height);

    //Initialize shaders
    //TODO filepath for windows, alter for unix like os
    GLuint vertexShaderId = CompileShaderFromFile(".\\res\\shaders\\vertex.glsl",GL_VERTEX_SHADER);
    GLuint fragmentShaderId = CompileShaderFromFile(".\\res\\shaders\\fragment.glsl",GL_FRAGMENT_SHADER);
    GLuint ProgrammID = glCreateProgram();              //create program to run on GPU
    glAttachShader(ProgrammID, vertexShaderId);         //attach vertex shader to new program
    glAttachShader(ProgrammID, fragmentShaderId);       //attach fragment shader to new program
    glLinkProgram(ProgrammID);                          //create execuatble
    glUseProgram(ProgrammID);
    //GLint MVPmatrix=glGetUniformLocation(ProgrammID,"MVPmatrix");//only callable after glUseProgramm has been called once


    /*
    //create plane
    GLuint VertexArrayID = 0;
    glGenVertexArrays(1,&VertexArrayID);
    glBindVertexArray(VertexArrayID);
    //Generate Vertex Positions
    float plane_vertices[9]=
    {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
    };
    GLuint vertexBufferId=0;
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices,GL_STATIC_DRAW);
*/
    createPlaneVBO();
    glDisable(GL_CULL_FACE);
    glClearColor(1.0f,1.0f,0.0f,0.5f);

/* https://www.seas.upenn.edu/%7Epcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
    //Generate PBO for fft result upload for gpu
    //Double Buffering indexing
    PBO_index=(index+1)%2;
    PBO_next_index=(intex+1)%2;
    //void* FFTData = ;

    GLuint Texture_ID=0;
    glBindTexture(GL_TEXTURE_2D, Texture_ID);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pboIds[PBO_index]);
    glBufferSubData(GL_UNPACK_BUFFER, )
    //Copy from PixelBufferObject to texture object
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,FFT_width,FFT_height,GL_BGRA,GL_UNSIGNED_BYTE,0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pboIds[PBO_next_index]);

*/
    double rotation_up_down=0;
    double rotation_left_right=0.1f;


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
        if(glfwGetKey(MainWindow,GLFW_KEY_A)==GLFW_PRESS){
            if(rotation_left_right>(-PI)){
                rotation_left_right=rotation_left_right-delta_time;
            }else{
                rotation_left_right=PI;
            }
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_D)==GLFW_PRESS){
             if(rotation_left_right<PI){
                rotation_left_right=rotation_left_right+delta_time;
            }else{
                rotation_left_right=-PI;
            }
        }
        //camera projection an transformation matrix calculation


        //gluLookAt(sin(rotation_left_right),cos(rotation_left_right),1,0,0,0,0,1,0);
        //atan(rotation_up_down)
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES,256*256*2,GL_UNSIGNED_INT,0);
        /*glUseProgram(ProgrammID);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

        glDrawArrays(GL_TRIANGLES,0,3);
        glDisableVertexAttribArray(0);
        */
    /* Swap front and back buffers */
        glfwSwapBuffers(MainWindow);
        /* Poll for and process events */
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
        printf("Escape");
    }
    if(key==GLFW_KEY_W&&action==GLFW_PRESS){

    }

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
    #define Resolution 256
    #define ScaleFact 400

    GLuint VaoId=0;
    GLuint VboPositionsId=0;
    GLuint VboIndicesId=0;

    //Generate Vertex Array Object
    glGenVertexArrays(1,&VaoId);
    glBindVertexArray(VaoId);


    //Generate Vertex Positions
    float plane_vertices[12]={  //for testing
        -0.8f,-0.8f,0.0f,
        0.8f,-0.8f,0.0f,
        -0.8f,0.8f,0.0f,
        0.8f,0.8f,0.0f
    };
    /*float* plane_vertices =malloc(3*Resolution*Resolution*sizeof(float));      //TODO free this pointer ( memory leak )
    long vert_index=0;
    for(int y=0;y<Resolution;y++){
        for(int x=0;x<Resolution;x++){
            plane_vertices[vert_index++]=((float)x)/ScaleFact;
            plane_vertices[vert_index++]=((float)y)/ScaleFact; //Set height (y) to zero
            plane_vertices[vert_index++]=0.0f;
        }
    }
    */
    glGenBuffers(1, &VboPositionsId);                                                          //create buffer
    glBindBuffer(GL_ARRAY_BUFFER, VboPositionsId);                                            //Link buffer
    printf("Sizeof %d",sizeof(plane_vertices));
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices),plane_vertices,GL_STATIC_DRAW);    //Upload data to Buffer, Vertex data is set only once and drawn regularly, hence we use GL_STATIC_DRAW
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,0,0);


    //Generate Vertex Indices
    GLuint plane_indices[6]={
        0,1,2,
        1,2,3
    };
    /*GLuint* plane_indices = malloc((Resolution-1)*(Resolution-1)*6*sizeof(GLuint));         //TODO free allocated Memory
    vert_index=0;
    for(unsigned int z=0;z<(Resolution-1);z++){
        for(unsigned int x=0;x<(Resolution-1);x++){
            //Generate first triangle
            plane_indices[vert_index++]=x+(z*Resolution);   //Vertex lower left first triangle
            plane_indices[vert_index++]=x+1+((z+1)*Resolution); //Vertex upper right first triangle
            plane_indices[vert_index++]=x+((z+1)*Resolution); //Vertex upper left first triangle
            //Generate second triangle
            plane_indices[vert_index++]=x+(z*Resolution);   //Vertex lower left second triangle
            plane_indices[vert_index++]=x+((z+1)*Resolution); //Vertex lower right second triangle
            plane_indices[vert_index++]=x+1+((z+1)*Resolution); //Vertex upper right first triangle
        }
    }
*/
    glGenBuffers(1, &VboIndicesId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VboIndicesId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_indices),plane_indices,GL_STATIC_DRAW);
}


/*
psi=(fftwf_complex*)fftw_malloc(sizeof(fftw_complex)*simWidth*simHeight);
fft = fftwf_plan_dft_2d(simWidth,simHeight,psi,psi,FFTW_FORWARD,FFTW_MEASURE); //psi is in and out for result
ifft = fftwf_plan_dft_2d(simWidth,simHeight,psi,psi,FFTW_BACKWARD,FFTW_MEASURE);
//BuildMomentumPropagator

*/
