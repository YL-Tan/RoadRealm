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
#include "Grid.h"
#include <string>
#include <set>

using namespace std;

unsigned int NUM_OF_FRAMES = 0;
double INIT_FPS_TIME = 0, RANDOM_TIMER = 0;
bool GLOBAL_MOUSE_DOWN = false;

vec2 CURRENT_CLICKED_CELL((NROWS + NCOLS), (NROWS + NCOLS));

set<vec2> PREV_DRAGGED_CELLS;

InfoPanel infoPanel;

time_t oldtime = clock();

vector<RoadPathLinker> ROADS;

vector<Vehicle> VEHICLES_COLLECTION;

void Update() {
    time_t now = clock();
    float dt = (float) (oldtime - now) / CLOCKS_PER_SEC;
    oldtime = now;

    //VehicleA.Update(dt);

    for(Vehicle&runner : VEHICLES_COLLECTION)
    {
        runner.Update(dt);
    }
}

Node ToggleNodeState(int col, int row, GridPrimitive &gridPrimitive,  vector<NodePosition>& path) {
    if (col < NCOLS && row < NROWS) {
        // Get Potential Index, Will Explain On Tuesday,
        // [0,0] bottom Left
        int potentialIndex = CombineDigits(row, col);

        Node node = gridPrimitive.NodeHandler(potentialIndex);

        path.push_back(node.currentPos);

        infoPanel.mouseSpaceDisp = "Mouse Click: X" + to_string(col) + " Y " + to_string(row);
        infoPanel.errorMsg = "Success";

        return node;

    } else {
        infoPanel.errorMsg = "Out Of Grid Mouse Click";
    }
    return {};
}

void ToggleDraggedCellsStates(GridPrimitive &gridPrimitive) {
    if (!GLOBAL_MOUSE_DOWN && !PREV_DRAGGED_CELLS.empty()) {

        RoadPathLinker roadPathLinker;

        int counter = 0;

        NodePosition home, factory;
        int homeIndex = -1, factoryIndex = -1;

        for (const vec2& cell: PREV_DRAGGED_CELLS) {

            Node getNode = ToggleNodeState((int) cell.x, (int) cell.y, gridPrimitive, roadPathLinker.roadPath);

            if(getNode.currentState == CLOSED_HOUSE)
            {
              cout << "Placement House: " << counter << "\n";
              roadPathLinker.color = getNode.overlayColor;

              home = getNode.currentPos;
              homeIndex = counter;
            }
            if(getNode.currentState == CLOSED_FACTORY)
            {
                cout << "Placement Factory: " << counter << "\n";
                factory = getNode.currentPos;
                factoryIndex = counter;
                // roadPathLinker.isLinked = true;
            }
            counter += 1;
        }

        bool validatePath = gridPrimitive.IsValidDestination(home, factory, homeIndex, factoryIndex);

        if(validatePath)
        {
            roadPathLinker.isLinked = true;
            ROADS.push_back(roadPathLinker);

            Vehicle runner(-.2f, .4f, ROADS.at(ROADS.size() - 1));

            VEHICLES_COLLECTION.push_back(runner);
        }

        // Clear
        PREV_DRAGGED_CELLS.clear();
    }
}

bool ClickedCellHandled(int col, int row) {
    for (const vec2 &cell: PREV_DRAGGED_CELLS) {
        if (cell.x == floor(col) && cell.y == floor(row)) {
            return true;
        }
    }
    return false;
}

bool AccumulateDraggedCell(float xMouse, float yMouse) {
    int col = (int) ((xMouse - X_POS) / DX), row = (int) ((yMouse - Y_POS) / DY);

    if (!ClickedCellHandled(col, row)) {

        CURRENT_CLICKED_CELL = vec2(col, row);

        PREV_DRAGGED_CELLS.insert(CURRENT_CLICKED_CELL);

        return true;
    }
    return false;
}

bool AccumulateDraggedCell(int col, int row) {
    if (!ClickedCellHandled(col, row)) {

        CURRENT_CLICKED_CELL = vec2(col, row);

        PREV_DRAGGED_CELLS.insert(CURRENT_CLICKED_CELL);

        return true;
    }
    return false;
}

void MouseButton(float xmouse, float ymouse, bool left, bool down) {
    if (down) {
        GLOBAL_MOUSE_DOWN = true;
        AccumulateDraggedCell(xmouse, ymouse);
    } else {
        GLOBAL_MOUSE_DOWN = false;
    }
}

void MouseMove(float x, float y, bool leftDown, bool rightDown) {
    int col = (int) ((x - X_POS) / DX), row = (int) ((y - Y_POS) / DY);
    infoPanel.mouseSpaceMoveDisp = "Mouse Move: X" + to_string(col) + " Y " + to_string(row);

    if (GLOBAL_MOUSE_DOWN) {
        AccumulateDraggedCell(col, row);
    }
}

void Display(GridPrimitive gridPrimitive) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    UseDrawShader(ScreenMode());

    gridPrimitive.DrawGrid();


    for(Vehicle&runner: VEHICLES_COLLECTION)
    {
        runner.Draw(infoPanel.logsMsg);
    }

    infoPanel.gridWinDim = "Grid Window: W: " + to_string(GRID_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.infoWinDim = "Info Window: W: " + to_string(DISP_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.appWinDisp = "Global Window: W: " + to_string(GLOBAL_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.gridPrimitiveDim = "Grid DIM: (" + to_string(NROWS) + " by " + to_string(NCOLS) + ")";

    infoPanel.InfoDisplay();

    glFlush();
}

void UpdateAppVariables(int width, int height) {
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

int main(int ac, char **av) {
    GLFWwindow *w = InitGLFW(100, 100, APP_WIDTH, APP_HEIGHT, "RoadRealm");

    RegisterMouseButton(MouseButton);
    RegisterMouseMove(MouseMove);
    RegisterResize(Resize);

    INIT_FPS_TIME = glfwGetTime();

    RANDOM_TIMER = glfwGetTime();

    GridPrimitive gridPrimitive;

    gridPrimitive.AddNewObjective(0, 1, 3, 0);

    gridPrimitive.AddNewObjective(4, 2, 6, 3);

    gridPrimitive.AddNewObjective(7, 3, 5, 2);

    while (!glfwWindowShouldClose(w)) {
        Update();

        // FPS Calculator
        infoPanel.fpsDisp = NUM_OF_FRAMES / (glfwGetTime() - INIT_FPS_TIME);

        Display(gridPrimitive);

        glfwSwapBuffers(w);

        glfwPollEvents();

        ToggleDraggedCellsStates(gridPrimitive);

        NUM_OF_FRAMES += 1;

    }
}