//TODO compile GLEW as static library
#define GLEW_STATIC
#include "libraries/GLEW_2.1.0/include/glew.h"
#include "libraries/GLFW_3.2.1/include/glfw3.h"
#include <stdio.h>
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button,int action, int mods);
void mouse_button_callback();
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

    printf("OpenGl Version: %s",glGetString(GL_VERSION));
    //Register Callbacks for user input
    glfwSetKeyCallback(MainWindow,key_callback);
    glfwSetMouseButtonCallback(MainWindow, mouse_button_callback);
    void createPlane();
    while (!glfwWindowShouldClose(MainWindow)){
        glClear(GL_COLOR_BUFFER_BIT);
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
    GLuint* vertexbuffer=0;
    GLuint* indexbuffer=0;
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
    glGenBuffers(1, vertexbuffer);                                                          //create buffer
    glBindBuffer(GL_ARRAY_BUFFER, *vertexbuffer);                                            //Link buffer
    glBufferData(GL_ARRAY_BUFFER, VertBufSize*sizeof(plane_vertices),plane_vertices,GL_STATIC_DRAW);    //Upload data to Buffer
    //Vertex data is set only once and drawn regularly, hence we use GL_STATIC_DRAW
    glEnableVertexAttribArray(*vertexbuffer);
    //Generate Triangles
    long plane_indices[IndexBufSize];
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
    glGenBuffers(1, indexbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indexbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufSize*sizeof(plane_indices),plane_indices,GL_STATIC_DRAW);
    //t
    glDrawElements(GL_TRIANGLES,VertBufSize,GL_UNSIGNED_INT,indexbuffer);      //TODO !!
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
