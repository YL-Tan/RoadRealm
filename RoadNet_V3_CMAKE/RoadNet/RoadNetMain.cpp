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
#include <chrono>
#include <functional>
#include <map>

using namespace std;

unsigned int NUM_OF_FRAMES = 0;
double INIT_FPS_TIME = 0, RANDOM_TIMER = 0;
bool GLOBAL_MOUSE_DOWN = false;

enum State {draw, wipe};
vec2 CURRENT_CLICKED_CELL((NROWS + NCOLS), (NROWS + NCOLS));

set<vec2> PREV_DRAGGED_CELLS;

InfoPanel infoPanel;
State globalState = draw;

time_t oldtime = clock();
chrono::duration<double> gameClock;
int currNumRoads = 10;

// vector<RoadRunnerLinker> ROAD_RUNNERS;

map<size_t, RoadRunnerLinker> ROAD_RUNNERS;

hash<string> STRING_HASH_FUN;

// vector<Vehicle> VEHICLES_COLLECTION;

void Update() {
    if (!infoPanel.isPaused) {
        time_t now = clock();
        float dt = (float)(now - oldtime) / CLOCKS_PER_SEC;
        oldtime = now;

        //VehicleA.Update(dt);

        /*for (RoadRunnerLinker& runnerLinkers : ROAD_RUNNERS)
        {
            runnerLinkers.vehicleRunner.Update(dt);
        }*/
        for(auto &runnerLinkers: ROAD_RUNNERS)
        {
            runnerLinkers.second.vehicleRunner.Update(dt);
        }
        gameClock += chrono::duration<double>(dt);
        infoPanel.timeDisplay = "Time: " + to_string(gameClock.count()) + "s";
    }

    infoPanel.gridWinDim = "Grid Window: W: " + to_string(GRID_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.infoWinDim = "Info Window: W: " + to_string(DISP_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.appWinDisp = "Global Window: W: " + to_string(GLOBAL_W) + " H: " + to_string(GLOBAL_H);
    infoPanel.gridPrimitiveDim = "Grid DIM: (" + to_string(NROWS) + " by " + to_string(NCOLS) + ")";
    infoPanel.numRoads = "Number of Roads: " + to_string(currNumRoads);
}

Node ToggleNodeState(int col, int row, GridPrimitive &gridPrimitive,  vector<NodePosition>& path, string& pathKey) {
    if (col < NCOLS && row < NROWS) {
        // Get Potential Index, Will Explain On Tuesday,
        // [0,0] bottom Left
        int potentialIndex = CombineDigits(row, col);

        Node node = gridPrimitive.NodeHandler(potentialIndex);
        path.push_back(node.currentPos);

        pathKey += to_string(node.currentPos.row) + to_string(node.currentPos.col);

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

        // RoadRunnerLinker roadRunnerLinker;
        Vehicle vehicleRunner(-.2f, .4f);

        int counter = 0;
        NodePosition home, factory;
        int homeIndex = -1, factoryIndex = -1;
        int oldNumRoads = currNumRoads;

        string pathHashKey;

        for (const vec2& cell: PREV_DRAGGED_CELLS) {

            Node getNode = ToggleNodeState((int) cell.x, (int) cell.y, gridPrimitive, vehicleRunner.runnerPath, pathHashKey);

            if(getNode.currentState == CLOSED_HOUSE)
            {
              cout << "Placement House: " << counter << "\n";

              vehicleRunner.overlayColor = getNode.overlayColor;

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
            currNumRoads -= 1;
            counter += 1;
        }

        bool validatePath = gridPrimitive.IsDestinationLinked(home, factory, homeIndex, factoryIndex);

        if(validatePath)
        {
            currNumRoads += 2; // compensate from including the starting and ending nodes.
            // Hash PathKey
            size_t hashValue = STRING_HASH_FUN(pathHashKey);

            RoadRunnerLinker runnerLinker(hashValue, vehicleRunner, true);

            ROAD_RUNNERS.insert({hashValue, runnerLinker});
        }
        else 
        {
            currNumRoads = oldNumRoads;
        }
        // Clear
        PREV_DRAGGED_CELLS.clear();
    }
    else
    {
        //FIXME
        for (const vec2& cell : PREV_DRAGGED_CELLS) {
            
        }
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

void KeyButton(int key, bool down, bool shift, bool control) {
    if (down == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_P:
                infoPanel.togglePause();
                break;
            case GLFW_KEY_D:
                globalState = draw;
                infoPanel.status = "Draw";
                break;
            case GLFW_KEY_W:
                globalState = wipe;
                infoPanel.status = "Wipe";
                break;
            // Add other key actions 
        }
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

    for(auto &runnerLinkers: ROAD_RUNNERS)
    {
        runnerLinkers.second.vehicleRunner.Draw(infoPanel.logsMsg);
    }

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

const char* usage = R"(
	Press P to pause the game
)";

int main(int ac, char **av) {
    GLFWwindow *w = InitGLFW(100, 100, APP_WIDTH, APP_HEIGHT, "RoadRealm");

    printf("Usage:%s", usage);

    RegisterMouseButton(MouseButton);
    RegisterMouseMove(MouseMove);
    RegisterResize(Resize);
    RegisterKeyboard(KeyButton);

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