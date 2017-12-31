/*TODO list
-compile GLEW as static library
-implement webcam brightest spot detection https://www.pyimagesearch.com/2014/09/29/finding-brightest-spot-image-using-python-opencv/
*/
#define GLEW_STATIC
#include "libraries/GLEW_2.1.0/include/glew.h"
#include "libraries/GLFW_3.2.1/include/glfw3.h"
//#include "libraries/FFTW_3.3.5/include/fftw3.h"     //Depending on the desired precision use fftw3 (double), fftw3f (single) or fftwl (long double)
#include <stdio.h>
#include <stdlib.h>
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button,int action, int mods);
void mouse_button_callback();
void createPlane();
void createCube();
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
    glDisable(GL_CULL_FACE);
    glClearColor(1.0f,1.0f,0.0f,0.5f);
   /*
    //Generate Texture
    GLuint TextureId = 0;
    glGenTextures(1,&TextureId);
    glBindTexture(GL_TEXTURE_2D, TextureId);
    glTexImage2D();
    glTexSubImage2D(GL_TEXTURE_2D,0,GL_RG,)
    */
    while (!glfwWindowShouldClose(MainWindow)){
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        //glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
        glUseProgram(ProgrammID);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);
        glDrawArrays(GL_TRIANGLES,0,3);
        glDisableVertexAttribArray(0);

    /* Swap front and back buffers */
        glfwSwapBuffers(MainWindow);
        /* Poll for and process events */
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
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
    fseek(filepointer,0,SEEK_END);                      //shift filePointer to EndOfFile Position to get filelength
    long filelength = ftell(filepointer);               //get filePointer position
    fseek(filepointer,0,SEEK_SET);                      //move file Pointer back to first line of file
    char* filestring = (char*)malloc(filelength+1);     //
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

/*
psi=(fftwf_complex*)fftw_malloc(sizeof(fftw_complex)*simWidth*simHeight);
fft = fftwf_plan_dft_2d(simWidth,simHeight,psi,psi,FFTW_FORWARD,FFTW_MEASURE); //psi is in and out for result
ifft = fftwf_plan_dft_2d(simWidth,simHeight,psi,psi,FFTW_BACKWARD,FFTW_MEASURE);
//BuildMomentumPropagator

*/
