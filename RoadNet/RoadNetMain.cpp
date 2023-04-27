// RoadNetMain.cpp - 2D road network planning game
// Team 8 (Edwin Kaburu, Vincent Marklynn, Yong Long Tan)
// NOTE: Before starting up the game, please ensure to include:
//       GLXtras.cpp, Draw.cpp, IO.cpp, Letters.cpp, Text.cpp

#include <glad.h>
#include <GLFW/glfw3.h>
#include "Draw.h"
#include "GLXtras.h"
#include "Text.h"
#include <limits>
#include <ctime>
#include <vector>
#include "AStar.h"
#include <string>
#include <set>

using namespace std;

unsigned int NUM_OF_FRAMES = 0;
double INITIAL_TIME = 0;
bool GLOBAL_MOUSE_DOWN = false;
set<vec2> DRAGGED_CELLS;

InfoPanel infoPanel;
time_t oldtime = clock();
Grid start(0, 0), stop(2, 2);
Bot botA(-.2f, .4f, 0); // , botB(.3f, .8f, 1);

AStar aStar;

// Display
// updating the state of the application, and rendering the graphics.
void Update() {
    time_t now = clock();
    float dt = (float)(oldtime - now) / CLOCKS_PER_SEC;
    oldtime = now;
    botA.Update(dt);
    // botB.Update(dt);
}

void Display() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    UseDrawShader(ScreenMode());
    aStar.Draw();
    //aStar.DrawPaths(1.5f, ORANGE);
    DrawPath(path, 3.5f, WHITE);
    // bots
    botA.Draw(RED);
    //botB.Draw(GREEN);

    infoPanel.gridWinDim = "Grid Window: W: " + to_string(GRID_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.infoWinDim = "Info Window: W: " + to_string(DISP_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.appWinDisp = "Global Window: W: " + to_string(GLOBAL_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.gridPrimitiveDim = "Grid DIM: (" + to_string(NROWS) + " by " + to_string(NCOLS) + ")";

    infoPanel.InfoDisplay();

    glFlush();
}

void ToggleCellBinaryState(int col, int row)
{
    if (col < NCOLS && row < NROWS) {
        aStar.nodes[row][col].roadPlaced = !aStar.nodes[row][col].roadPlaced;
        // reset astar nodes
        for (int row = 0; row < NROWS; row++)
            for (int col = 0; col < NCOLS; col++) {
                Node& n = aStar.nodes[row][col];
                n.open = n.closed = false;
            }
        infoPanel.mouseSpaceDisp = "Mouse Click: X" + to_string(col) + " Y " + to_string(row);
        infoPanel.errorMsg = "Success";
    }
    else {
        infoPanel.errorMsg = "Out Of Grid Mouse Click";
    }
}
bool DraggedCellExists(int col, int row)
{
    for(vec2 cell : DRAGGED_CELLS)
    {
        if(cell.x == floor(col) && cell.y == floor(row))
        {
            return true;
        }
    }
    return false;
}
bool AccumulateDraggedCell(float xMouse, float yMouse)
{
    int col = (int)((xMouse - X_POS) / DX), row = (int)((yMouse - Y_POS) / DY);

    if(!DraggedCellExists(col,row))
    {
        DRAGGED_CELLS.insert(vec2(col, row));
        return true;
    }
    return false;
}
bool AccumulateDraggedCell(int col, int row)
{
    if(!DraggedCellExists(col,row))
    {
        DRAGGED_CELLS.insert(vec2(col, row));
        return true;
    }
    return false;
}

void MouseButton(float xmouse, float ymouse, bool left, bool down) {
    if (down) {
        GLOBAL_MOUSE_DOWN = true;

        AccumulateDraggedCell(xmouse, ymouse);
    }
    else
    {
        GLOBAL_MOUSE_DOWN = false;
        for(vec2 cell : DRAGGED_CELLS)
        {
            ToggleCellBinaryState((int)cell.x, (int)cell.y);
        }
        DRAGGED_CELLS.clear();
    }
    // compute new path
    aStar.ComputePath(start, stop);
    aStar.ReconstructPath(aStar.goal, path);
    pathLength = PathLength();
}

void MouseMove(float x, float y, bool leftDown, bool rightDown )
{
    int col = (int)((x - X_POS) / DX), row = (int)((y - Y_POS) / DY);
    infoPanel.mouseSpaceMoveDisp = "Mouse Move: X" + to_string(col) + " Y " + to_string(row);

    if (GLOBAL_MOUSE_DOWN)
    {
        bool debugStatus = AccumulateDraggedCell(col, row);
        if(debugStatus)
        {
            cout << "\n----- Dragged Cell Collection---------\n";
            for(vec2 cell : DRAGGED_CELLS)
            {
                cout << cell.x << "\t" << cell.y << "\n";
            }
        }
    }
    else
    {
        for(vec2 cell : DRAGGED_CELLS)
        {
            ToggleCellBinaryState((int)cell.x, (int)cell.y);
        }
        DRAGGED_CELLS.clear();
    }
}

void UpdateAppVariables(int width, int height) {
    // Update Global Window
    GLOBAL_W = width - W_EDGE_BUFFER;
    GLOBAL_H = height - H_EDGE_BUFFER;

    // Update Grid Width and Display Width
    GRID_W = GLOBAL_W * 0.75, DISP_W = GLOBAL_W - (GLOBAL_W * 0.10); //GLOBAL_W - 150; // DISP_W = GLOBAL_W - GRID_W;

    // Update Difference of X-Axis AND Y-Axis
    DX = (float)GRID_W / NCOLS, DY = (float)GLOBAL_H / NROWS;
}

void Resize(int width, int height) {
    glViewport(0, 0, width, height);
    UpdateAppVariables(width, height);
    glFlush();
}


int main(int ac, char** av) {
    GLFWwindow* w = InitGLFW(100, 100, APP_WIDTH, APP_HEIGHT, "RoadRealm");

    RegisterMouseButton(MouseButton);
    RegisterMouseMove(MouseMove);
    RegisterResize(Resize);

    INITIAL_TIME = glfwGetTime();

    // path-finding
    aStar.ComputePath(start, stop);
    aStar.ReconstructPath(aStar.goal, path);
    pathLength = PathLength();

    while (!glfwWindowShouldClose(w)) {
        Update();
        // FPS Calculator
        infoPanel.fpsDisp = NUM_OF_FRAMES / (glfwGetTime() - INITIAL_TIME);
        Display();
        glfwSwapBuffers(w);
        glfwPollEvents();
        NUM_OF_FRAMES += 1;
    }
}