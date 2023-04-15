// 4-Demo-MouseTest.cpp

#include <glad.h>
#include <glfw3.h>
#include "Draw.h"
#include "GLXtras.h"
#include <vector>

class Grid {
public:
    int resolution;
    int cellSize;
    int winWidth, winHeight;

    Grid(int resolution, int winWidth, int winHeight) : resolution(resolution), winWidth(winWidth), winHeight(winHeight) {
        UpdateCellSize();
    }

    void UpdateCellSize() {
        int newCellSize = std::min(winWidth, winHeight) / resolution;
        cellSize = (newCellSize > 0) ? newCellSize : 1; // make sure cellSize is at least 1
    }

    void Resize(int width, int height) {
        winWidth = width;
        winHeight = height;
        UpdateCellSize();
    }

    void Draw() {
        vec3 color(0, 0, 0); // black
        int horizontalLines = winHeight / cellSize + 1; // + 1 to add a boundary line
        int verticalLines = winWidth / cellSize + 1;    // + 1 to add a boundary line

        for (int i = 0; i <= horizontalLines; i++) {
            Line(vec2(0, i * cellSize), vec2(winWidth, i * cellSize), 1.0f, color);
        }
        for (int i = 0; i <= verticalLines; i++) {
            Line(vec2(i * cellSize, 0), vec2(i * cellSize, winHeight), 1.0f, color);
        }
    }
};

int windowedX = 200, windowedY = 200, windowedWidth = 600, windowedHeight = 600;
int RESOLUTION = 64;
Grid grid(RESOLUTION, 0, 0);        // Initialize with 0, will be updated in Resize
vec3 backColor(0, .5f, 0);
std::vector<std::vector<bool>> gridState;

void Display() {
    glClearColor(backColor.x, backColor.y, backColor.z, 1);    // set background color
    glClear(GL_COLOR_BUFFER_BIT);                            // clear background and z-buffer
    UseDrawShader(ScreenMode());
    grid.Draw();
    glFlush();
}

void Resize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    grid.Resize(width, height);
}

int main() {
    GLFWwindow* w = InitGLFW(windowedX, windowedY, windowedWidth, windowedHeight, "Mouse Test");
    Resize(w, windowedWidth, windowedHeight); // initialize the grid
    //RegisterMouseButton(MouseButton);

    glfwSetFramebufferSizeCallback(w, Resize);
    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(w)) {
        Display();
        glfwSwapBuffers(w);
        glfwPollEvents();
    }
    glfwDestroyWindow(w);
    glfwTerminate();
}