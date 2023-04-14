// 4-Demo-MouseTest.cpp

#include <glad.h>
#include <glfw3.h>
#include "Draw.h"
#include "GLXtras.h"

class Block {
public:
	int x, y, width, height;
	vec3 color;
	Block(int x, int y, int width, int height, vec3 color) : x(x), y(y), width(width), height(height), color(color) { }
};

int winWidth = 400, winHeight = 400;
Block block(100, 100, 100, 50, vec3(1, 0, 0));
vec3 backColor(0, .5f, 0);
bool blockPicked = false;
vec2 mouseRef;

void DrawBlock(Block b) {
	Quad(b.x, b.y, b.x+b.width, b.y, b.x+b.width, b.y+b.height, b.x, b.y+b.height, true, b.color);
}

void Display() {
	glClearColor(backColor.x, backColor.y, backColor.z, 1);	// set background color
	glClear(GL_COLOR_BUFFER_BIT);							// clear background and z-buffer
	UseDrawShader(ScreenMode());
	DrawBlock(block);
	glFlush();
}

bool BlockHit(float mouseX, float mouseY, Block b) {
	return mouseX >= b.x && mouseX <= b.x+b.width && mouseY >= b.y && mouseY <= b.y+b.height;
}

void MouseButton(float x, float y, bool left, bool down) {
	blockPicked = down && BlockHit(x, y, block);
	if (blockPicked)
		mouseRef = vec2(x, y);
}

void MouseMove(float x, float y, bool leftDown, bool rightDown) {
	if (leftDown && blockPicked) {
		vec2 mouse(x, y), dif = mouse-mouseRef;
		block.x += (int) dif.x;
		block.y += (int) dif.y;
		mouseRef = mouse;
	}
}

void Resize(int width, int height) {
	glViewport(0, 0, winWidth = width, winHeight = height);
}

const char *usage = R"(
	mouse-click on block to pick
	mouse-drag to move block
)";

int main() {
	GLFWwindow *w = InitGLFW(200, 200, 600, 600, "Mouse Test");
	printf("Usage:%s", usage);
	RegisterMouseMove(MouseMove);
	RegisterMouseButton(MouseButton);
	RegisterResize(Resize);
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(w)) {
		Display();
		glfwSwapBuffers(w);
		glfwPollEvents();
	}
	glfwDestroyWindow(w);
	glfwTerminate();
}
