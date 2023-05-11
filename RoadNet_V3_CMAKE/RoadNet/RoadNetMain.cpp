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
#include <iomanip>
#include <sstream>

using namespace std;

unsigned int NUM_OF_FRAMES = 0;
double INIT_FPS_TIME = 0;
bool GLOBAL_MOUSE_DOWN = false, GLOBAL_PAUSE = false, GLOBAL_DRAW_BORDERS = false;
double REPLENISH_INTERVAL = 10.0; // 10 seconds
int REPLENISH_ROADS_NUM = 10;

vec2 CURRENT_CLICKED_CELL((NROWS + NCOLS), (NROWS + NCOLS));

set<vec2> PREV_DRAGGED_CELLS;

InfoPanel infoPanel;
GameplayState globalState = DRAW_STATE;

time_t oldtime = clock();
chrono::duration<double> gameClock;
int currNumRoads = 10;
double lastReplenishTime = 0.0;
string status = "Draw";

map<size_t, RoadRunnerLinker> ROAD_RUNNERS;

hash<string> STRING_HASH_FUN;

string formatDuration(const chrono::duration<double> &duration) {
    int totalSeconds = static_cast<int>(duration.count());
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    stringstream ss;
    ss << setw(2) << setfill('0') << hours << "H"
       << setw(2) << setfill('0') << minutes << "M"
       << setw(2) << setfill('0') << seconds << "S";

    return ss.str();
}

void replenishRoads() {
    if (gameClock.count() - lastReplenishTime >= REPLENISH_INTERVAL) {
        currNumRoads += REPLENISH_ROADS_NUM;
        cout << "Just replenished " << REPLENISH_ROADS_NUM << " roads to player" << endl;

        lastReplenishTime = gameClock.count();
    }
}

void Update() {

    time_t now = clock();
    float dt = (float) (now - oldtime) / CLOCKS_PER_SEC;

    if (!GLOBAL_PAUSE) {
        dt = (float) (now - oldtime) / CLOCKS_PER_SEC;
        oldtime = now;
        for (auto &runnerLinkers: ROAD_RUNNERS) {
            runnerLinkers.second.vehicleRunner.Update(dt);
        }
        gameClock += chrono::duration<double>(dt);
        replenishRoads();
    } else {
        oldtime = now;
    }

    infoPanel.AddMessage(TIME_LABEL, "Time: " + formatDuration(gameClock), WHITE);
    infoPanel.AddMessage(DIMS_LABEL, "Grid DIM: (" + to_string(NROWS) + " by " + to_string(NCOLS) + ")", WHITE);
    infoPanel.AddMessage(NUM_OF_ROAD_LABEL, "Number of Roads: " + to_string(currNumRoads), WHITE);
    infoPanel.AddMessage(GRID_STATE_LABEL, status, WHITE);

}

Node ToggleNodeState(int col, int row, GridPrimitive &gridPrimitive, vector<NodePosition> &path, string &pathKey) {
    if (col < NCOLS && row < NROWS && col > -1 && row > -1) {
        // Get Potential Index, Will Explain On Tuesday,
        // [0,0] bottom Left
        int potentialIndex = CombineDigits(row, col);
        // cout << col << "\t" << row << "\t" << potentialIndex << "\n";
        Node node = gridPrimitive.NodeHandler(potentialIndex);

        path.push_back(node.currentPos);

        pathKey += to_string(node.currentPos.row) + to_string(node.currentPos.col);

        infoPanel.AddMessage(MOUSE_CLICK_LABEL, "Mouse Click: X" + to_string(col) + " Y " + to_string(row), WHITE);
        infoPanel.AddMessage(ERROR_MSG_LABEL, "Success", WHITE);

        return node;
    } else {
        infoPanel.AddMessage(ERROR_MSG_LABEL, "Out Of Grid Mouse Click", WHITE);
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

        for (const vec2 &cell: PREV_DRAGGED_CELLS) {

            Node getNode = ToggleNodeState((int) cell.x, (int) cell.y, gridPrimitive, vehicleRunner.runnerPath,
                                           pathHashKey);

            if (getNode.currentState == CLOSED_HOUSE) {
                cout << "Placement House: " << counter << "\n";

                vehicleRunner.overlayColor = getNode.overlayColor;

                home = getNode.currentPos;
                homeIndex = counter;
            }
            if (getNode.currentState == CLOSED_FACTORY) {
                cout << "Placement Factory: " << counter << "\n";
                factory = getNode.currentPos;
                factoryIndex = counter;
                // roadPathLinker.isLinked = true;
            }

            counter += 1;
        }

        bool validatePath = gridPrimitive.IsDestinationLinked(home, factory, homeIndex, factoryIndex);
        if ((int) PREV_DRAGGED_CELLS.size() <= currNumRoads) {

            if (validatePath) {
                currNumRoads += 2; // compensate from including the starting and ending nodes.
                // Hash PathKey
                size_t hashValue = STRING_HASH_FUN(pathHashKey);

                RoadRunnerLinker runnerLinker(hashValue, vehicleRunner, true);

                ROAD_RUNNERS.insert({hashValue, runnerLinker});
                currNumRoads -= (int) PREV_DRAGGED_CELLS.size();
            } else {
                currNumRoads = oldNumRoads;
            }
            if (globalState == WIPE_STATE) {
                cout << pathHashKey << endl;
                size_t hashValue = STRING_HASH_FUN(pathHashKey);
                ROAD_RUNNERS.erase(hashValue);
                currNumRoads += (int) PREV_DRAGGED_CELLS.size() - 2;
            }
        } else {
            cout << "Not enough roads!" << endl;
        }
        // Clear
        PREV_DRAGGED_CELLS.clear();
        CURRENT_CLICKED_CELL = vec2((NROWS + NCOLS), (NROWS + NCOLS));
    } else {
        if (CURRENT_CLICKED_CELL.x < NCOLS && CURRENT_CLICKED_CELL.y < NROWS
            && CURRENT_CLICKED_CELL.x > -1 && CURRENT_CLICKED_CELL.y > -1) {
            Node getNode = gridPrimitive.NodeHandler(
                    CombineDigits((int) CURRENT_CLICKED_CELL.y, (int) CURRENT_CLICKED_CELL.x));
            //cout << "Current State: " << getNode.currentState << "\n";
        }
        CURRENT_CLICKED_CELL = vec2((NROWS + NCOLS), (NROWS + NCOLS));
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

    infoPanel.AddMessage(MOUSE_MOVE_LABEL, "Mouse Move: X" + to_string(col) + " Y " + to_string(row), WHITE);

    if (GLOBAL_MOUSE_DOWN) {
        AccumulateDraggedCell(col, row);
    }
}

void KeyButton(int key, bool down, bool shift, bool control) {
    if (down == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_P:
                GLOBAL_PAUSE = !GLOBAL_PAUSE;
                break;
            case GLFW_KEY_SPACE:
                GLOBAL_PAUSE = !GLOBAL_PAUSE;
                break;
            case GLFW_KEY_D:
                if (globalState == WIPE_STATE) {
                    globalState = DRAW_STATE;
                    status = "Draw";
                } else {
                    globalState = WIPE_STATE;
                    status = "Delete";
                }
                break;
            case GLFW_KEY_W:
                globalState = WIPE_STATE;
                status = "Delete";
                break;
            case GLFW_KEY_B:
                GLOBAL_DRAW_BORDERS = !GLOBAL_DRAW_BORDERS;
                break;
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

    for (auto &runnerLinkers: ROAD_RUNNERS) {
        string runnerDrawLog;
        runnerLinkers.second.vehicleRunner.Draw(runnerDrawLog);

        infoPanel.AddMessage(LOGS_MSG_LABEL, runnerDrawLog, WHITE);
    }
    if (GLOBAL_DRAW_BORDERS) {
        DrawBorders();
    }

    infoPanel.InfoDisplay();

    glFlush();
}

void UpdateAppVariables(int width, int height) {
    // Update Global Window
    GLOBAL_W = width - W_EDGE_BUFFER;
    GLOBAL_H = height - H_EDGE_BUFFER;

    // Update Grid Width and Display Width
    GRID_W = GLOBAL_W * 0.75, DISP_W = GLOBAL_W - (GLOBAL_W * 0.22);

    // Update Difference of X-Axis AND Y-Axis
    DX = (float) GRID_W / NCOLS, DY = (float) GLOBAL_H / NROWS;
}

void Resize(int width, int height) {
    glViewport(0, 0, width, height);
    UpdateAppVariables(width, height);
    glFlush();
}

const char *usage = R"(
	Press P or space key to pause the game
)";

int main(int ac, char **av) {
    GLFWwindow *w = InitGLFW(100, 100, APP_WIDTH, APP_HEIGHT, "RoadRealm");

    printf("Usage:%s", usage);

    RegisterMouseButton(MouseButton);
    RegisterMouseMove(MouseMove);
    RegisterResize(Resize);
    RegisterKeyboard(KeyButton);

    INIT_FPS_TIME = glfwGetTime();

    GridPrimitive gridPrimitive;

    for (int i = 0; i < 5; i++) {
        vec2 rndStPoint = GetRandomPoint();
        vec2 rndEdPoint = GetRandomPoint();

        gridPrimitive.AddNewObjective((int) rndStPoint.y, (int) rndStPoint.x, (int) rndEdPoint.y, (int) rndEdPoint.x);
    }

    // Reserve Vector Size
    double framesPerSecond = 0;
    while (!glfwWindowShouldClose(w)) {
        Update();

        // FPS Calculator
        framesPerSecond = NUM_OF_FRAMES / (glfwGetTime() - INIT_FPS_TIME);

        infoPanel.AddMessage(FPS_LABEL, to_string(framesPerSecond), WHITE);

        Display(gridPrimitive);

        glfwSwapBuffers(w);

        glfwPollEvents();

        ToggleDraggedCellsStates(gridPrimitive);

        NUM_OF_FRAMES += 1;

    }
}