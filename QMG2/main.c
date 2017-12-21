#define GLEW_STATIC     //We are linking glew as static lib
#include "libraries/GLEW_2.1.0/include/glew.h"
#include "libraries/GLFW_3.2.1/include/glfw3.h"
#include <stdio.h>

int main(int argc, char* argv[]){
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
