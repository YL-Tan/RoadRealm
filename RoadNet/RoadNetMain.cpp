// RoadRealm.cpp - 2D road network planning game

#include <glad.h>
#include <GLFW/glfw3.h>
#include "Draw.h"
#include "GLXtras.h"
#include "Text.h"
#include <limits>
#include <ctime>
#include <vector>
#include "AStar.h"


using namespace std;

AStar aStar;

struct InfoPanel
{
    string mouseSpaceDisp;
    string gridDimDisp;
    string textDimDisp;
    string appDimDisp;
    string errorMsg;
    double fpsDisp = 0;

    void InfoDisplay()
    {
        Text(DISP_W , GLOBAL_H , WHITE, 10.0f, mouseSpaceDisp.c_str());
        Text(DISP_W, GLOBAL_H - 20, WHITE, 10.0f, gridDimDisp.c_str());
        Text(DISP_W, GLOBAL_H - 40, WHITE, 10.0f, textDimDisp.c_str());
        Text(DISP_W, GLOBAL_H - 60, WHITE, 10.0f, appDimDisp.c_str());
        Text(DISP_W, GLOBAL_H - 80, WHITE, 10.0f, errorMsg.c_str());
        Text(DISP_W, GLOBAL_H - 100, WHITE, 10.0f, to_string(fpsDisp).c_str());
    }
};

InfoPanel infoPanel;
unsigned int FPS = 0;
double INITIAL_TIME = 0;

void Display() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    UseDrawShader(ScreenMode());
    aStar.Draw();

    infoPanel.gridDimDisp = "Grid Dim: W: " + to_string(GRID_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.textDimDisp = "Info Dim: W: " + to_string(DISP_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.appDimDisp = "Global Dim: W: " + to_string(GLOBAL_W) + " H: " + to_string(GLOBAL_H);

    infoPanel.InfoDisplay();

    // Text(DISP_W , GLOBAL_H , WHITE, 10.0f, infoPanel.ToString().c_str());

   /* string displayText = "This is a Text";
    Text(GLOBAL_W , GLOBAL_H , WHITE, 12.0f, displayText.c_str());
    cout << "Grid Display W: " <<  (GRID_W) << " H: " <<  (GLOBAL_H) << "\n";
    cout << "Info Display W: " <<  (GLOBAL_W) << " H: " <<  (GLOBAL_H) << "\n";*/
/*
      for(string display : messageLists)
      {
          Text(DISP_W , GLOBAL_H , WHITE, 10.0f, display.c_str());
      }*/
    //Text(350,350, WHITE, 12.0f, displayText.c_str());

    glFlush();
}

void MouseButton(float xmouse, float ymouse, bool left, bool down) {
    if (down) {
        int col = (int) ((xmouse-X_POS)/DX), row = (int) ((ymouse-Y_POS)/DY);
        if(col < NCOLS && row < NROWS)
        {
            aStar.nodes[row][col].roadPlaced = !aStar.nodes[row][col].roadPlaced;
            // reset astar nodes
            for (int row = 0; row < NROWS; row++)
                for (int col = 0; col < NCOLS; col++) {
                    Node &n = aStar.nodes[row][col];
                    n.open = n.closed = false;
                }
            infoPanel.mouseSpaceDisp = "Mouse Click: X" + to_string(col) + " Y " + to_string(row);
            infoPanel.errorMsg = "Success";
        } else{
            infoPanel.errorMsg = "Out Of Grid Mouse Click";
        }

        // Debug
        /*messageLists.clear();
        string mouseDisplay = "Mouse Click: X" + to_string(col) + " Y " + to_string(row);
        messageLists.push_back(mouseDisplay);
        cout << mouseDisplay << "\n";*/
    }
}

void UpdateAppVariables(int width, int height)
{
    // Update Global Window
    GLOBAL_W = width - W_EDGE_BUFFER;
    GLOBAL_H = height - H_EDGE_BUFFER;

    // Update Grid Width and Display Width
    GRID_W = GLOBAL_W * 0.75, DISP_W = GLOBAL_W - (GLOBAL_W * 0.10); //GLOBAL_W - 150; // DISP_W = GLOBAL_W - GRID_W;

    // Update Difference of X-Axis AND Y-Axis
    DX = (float) GRID_W / NCOLS, DY = (float) GLOBAL_H / NROWS;
}
void Resize(int width, int height) {
    glViewport(0, 0, width, height);
    UpdateAppVariables(width, height);
    glFlush();
}

void FormulateFps()
{
    double dif = glfwGetTime() - INITIAL_TIME;
    if (dif >= 1.0f)
    {
        cout << "FPS" << FPS << "\n";
        infoPanel.fpsDisp = FPS;
        // Reset Fps Counter
        FPS = 0;
        // Set Current Time as Initial
        INITIAL_TIME = glfwGetTime();
    }
    FPS = FPS + 1;
}

int main(int ac, char **av) {
    GLFWwindow *w = InitGLFW(100, 100, APP_WIDTH, APP_HEIGHT, "RoadRealm");
    RegisterMouseButton(MouseButton);
    RegisterResize(Resize);

    INITIAL_TIME = glfwGetTime();

    while (!glfwWindowShouldClose(w)) {
        FormulateFps();
        Display();
        glfwSwapBuffers(w);
        glfwPollEvents();
    }
    //double endTime = glfwGetTime();
    //cout << currentTime << "\t" << endTime << " = " << (endTime - currentTime);
}