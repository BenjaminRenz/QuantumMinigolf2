#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <stdio.h>
#include <uchar.h>


int main(void){
    //glfw_init
    if(!glfwInit()){
        printf("Error: glfw initialisation failed!\n");
        return 1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* mainWindow=glfwCreateWindow(1000,800,"Vulkan QMG",0,0);
    //vulkan_init
    vk_
    while(!glfwWindowShouldClose(mainWindow)){
        glfwPollEvents();
    }
    glfwDestroyWindow(mainWindow);
    glfwTerminate();
}
