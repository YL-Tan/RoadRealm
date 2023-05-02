#include <iostream>
#include <glad/glad.h>
#include "GLFW/glfw3.h"

using namespace std;

// Callback FrameBufferSize. Happens When Window is Resized (Flexibility)
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // Change or Reset The ViewPort During Resizing
    glViewport(0,0, width, height);
}

// Inputs Processing
void processInput(GLFWwindow * window)
{
    // Process Escape Key When Pressed
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        // Set True on Glfw window to close
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {

    std::cout << "Hello, World!" << std::endl;

    // Initialize GLFW
    glfwInit();

    // Provide Windows Hints To Glfw, Major and Minor Version (Opengl 3.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Use The Core Profile for OpenGl
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Initialize Window Object
    GLFWwindow *window = glfwCreateWindow(800,800, "Template Window", NULL, NULL);

    // Window Validation Check
    if(window == NULL)
    {
        cout << "GLFW Window Not Created\n";
        // Terminate Glfw
        glfwTerminate();
        return -1;
    }
    // Set The Current Focus or Context To This Window
    glfwMakeContextCurrent(window);

    // Validate Glad is Loaded
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        // Failed: Glad Not Initialized
        cout << "Failed To Initialize Glad\n";
        // Terminate Glfw
        glfwTerminate();
        return -1;
    }

    // Set ViewPort: Tells OpenGl How To Display
    glViewport(0,0,800,800);
    // Register Callback To Glfw to call pointer to method, on every window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Main Infinite While Loop
    while (!glfwWindowShouldClose(window))
    {
        // Process Key Inputs
        processInput(window);

        // Rendering Commands
        // Change Color
        glClearColor(0.16f, 0.38f, 0.38f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap Buffers between the back and frontend buffers
        glfwSwapBuffers(window);
        // Poll Events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}