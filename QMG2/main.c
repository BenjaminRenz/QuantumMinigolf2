//TODO compile GLEW as static library
#define GLEW_STATIC
#include "libraries/GLEW_2.1.0/include/glew.h"
#include "libraries/GLFW_3.2.1/include/glfw3.h"
#include <stdio.h>
#include <stdlib.h>
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button,int action, int mods);
void mouse_button_callback();
void createPlane();
GLuint CompileShaderFromFile(char FilePath[] ,GLuint shaderType);
int main(int argc, char* argv[]){
    //GLFW init
    if (!glfwInit()){
        return -1;
    }
    GLFWwindow* MainWindow = glfwCreateWindow(600, 400, "Quantum Minigolf 2.0", NULL, NULL);
    //GLFWwindow* MainWindow = glfwCreateWindow(1920, 1080, "Quantum Minigolf 2.0", glfwGetPrimaryMonitor(), NULL);
    if (!MainWindow){
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(MainWindow);
    //GLEW init
    GLenum err = glewInit();
    if(GLEW_OK != err){
        printf("Error: glewInit() failed.");
    }
    printf("QuantumMinigolf v2:\n");
    printf("using OpenGl Version: %s\n",glGetString(GL_VERSION));
    //Register Callbacks for user input
    glfwSetKeyCallback(MainWindow,key_callback);
    glfwSetMouseButtonCallback(MainWindow, mouse_button_callback);
    //Initialize shaders
    GLuint vertexShaderId = CompileShaderFromFile(".\\res\\shaders\\vertex.glsl",GL_VERTEX_SHADER);
    GLuint fragmentShaderId = CompileShaderFromFile(".\\res\\shaders\\fragment.glsl",GL_FRAGMENT_SHADER);
    GLuint ProgrammID = glCreateProgram();              //create program to run on GPU
    glAttachShader(ProgrammID, vertexShaderId);         //attach vertex shader to new program
    glAttachShader(ProgrammID, fragmentShaderId);       //attach fragment shader to new program
    glLinkProgram(ProgrammID);                          //create execuatble
    createPlane();
    while (!glfwWindowShouldClose(MainWindow)){
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glUseProgram(ProgrammID);
        /* Swap front and back buffers */
        glfwSwapBuffers(MainWindow);
        /* Poll for and process events */
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void createPlane(){//GLuint* vertexbuffer, GLuint* indexbuffer){
    #define Resolution 256
    #define VertBufSize Resolution*Resolution
    #define IndexBufSize VertBufSize*6
    #define ScaleFact 1000
    //
    GLuint vertexbuffer=0;
    GLuint indexbuffer=0;
    //Generate Vertex Positions
    float plane_vertices[VertBufSize];
    long vert_index=0;
    for(int z=0;z<Resolution;z++){
        for(int x=0;x<Resolution;x++){
            plane_vertices[vert_index++]=((float)x)/ScaleFact;
            plane_vertices[vert_index++]=0; //Set height (y) to zero
            plane_vertices[vert_index++]=((float)z)/ScaleFact;
        }
    }
    glGenBuffers(1, &vertexbuffer);                                                          //create buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);                                            //Link buffer
    glBufferData(GL_ARRAY_BUFFER, VertBufSize*sizeof(plane_vertices),plane_vertices,GL_STATIC_DRAW);    //Upload data to Buffer
    //Vertex data is set only once and drawn regularly, hence we use GL_STATIC_DRAW

    //Set data format for gpu and enable position attribute
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, sizeof(float)*3,(void*)0);
    glEnableVertexAttribArray(0);

    //Generate Triangles
    long plane_indices[IndexBufSize];
    vert_index=0;
    for(int z=0;z<(Resolution-1);z++){
        for(int x=0;x<(Resolution-1);x++){
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
    glGenBuffers(1, &indexbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufSize*sizeof(plane_indices),plane_indices,GL_STATIC_DRAW);
    //Shader creation

    glDrawElements(GL_TRIANGLES,IndexBufSize,GL_UNSIGNED_INT,NULL);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(key==GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        printf("Escape");
    }
}
void mouse_button_callback(GLFWwindow* window, int button,int action, int mods){
    if(button== GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS){
        printf("LMB Down");
    }
}

/** \brief Loads Shader code from file into RAM and then compiles the shader.
 *
 * \param Path to the shader file.
 * \param Type of shader to use e.g.: GL_FRAGMENT_SHADER
 * \return
 *
 */
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
    GLint logLength = 0;
    glGetShaderiv(ShaderId,GL_COMPILE_STATUS,&compStatus);
    glGetShaderiv(ShaderId,GL_INFO_LOG_LENGTH,&logLength);
    if(compStatus!=GL_TRUE){
        printf("Error: Compilation of shader %d failed\n",ShaderId);
        //TODO free resources
        //return;
    }
    free(filestring);                                   //Delete Shader string from heap
    fclose(filepointer);
    return ShaderId;
}
