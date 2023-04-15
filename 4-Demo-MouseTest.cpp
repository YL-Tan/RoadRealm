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
GLFWmonitor* primaryMonitor = nullptr;
GLFWwindow* w = nullptr;
bool isFullscreen = false;
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

void ToggleFullscreen() {
    if (!isFullscreen) {
        glfwGetWindowPos(w, &windowedX, &windowedY);    // Store the current window position and size
        glfwGetWindowSize(w, &windowedWidth, &windowedHeight);

        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwSetWindowMonitor(w, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else {
        glfwSetWindowMonitor(w, NULL, windowedX, windowedY, windowedWidth, windowedHeight, GLFW_DONT_CARE);
    }
    isFullscreen = !isFullscreen;
}

void KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS)
        if (key == GLFW_KEY_F)
            ToggleFullscreen();
}

GLFWwindow* InitGLFW(int x, int y, const char* title) {
    if (!glfwInit())
        return NULL;
    primaryMonitor = glfwGetPrimaryMonitor();
    GLFWwindow* w = glfwCreateWindow(windowedWidth, windowedHeight, title, NULL, NULL);

    glfwMakeContextCurrent(w);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    return w;
}

const char* usage = R"(
	Press F to toggle the view mode from the windowed mode to fullscreen
)";

int main() {
	// GLFWwindow *w = InitGLFW(200, 200, 600, 600, "Mouse Test");
    w = InitGLFW(windowedX, windowedY, "Mouse Test");
    printf("Usage:%s", usage);
    
	int width, height;
	glfwGetFramebufferSize(w, &width, &height); // Get the initial window size
	Resize(w, width, height); // initialize the grid
    
    glfwSetKeyCallback(w, KeyCallback); // Register the key callback
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