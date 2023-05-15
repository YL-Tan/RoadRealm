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
#include "Sprite.h"

using namespace std;

unsigned int NUM_OF_FRAMES = 0;
double INIT_FPS_TIME = 0;
bool GLOBAL_MOUSE_DOWN = false, GLOBAL_PAUSE = false, GLOBAL_DRAW_BORDERS = false, ACTIVE_GAME_RESET = false;
// In Seconds
double REPLENISH_INTERVAL = 10.0, FRAMES_PER_SECONDS = 0;
int REPLENISH_ROADS_NUM = 20;
float FONT_SCALE = 10.0f;

vec2 CURRENT_CLICKED_CELL((NROWS + NCOLS), (NROWS + NCOLS));

vector<vec2> PREV_DRAGGED_CELLS;

InfoPanel infoPanel;
ApplicationStates application = STARTING_MENU;
Sprite myResetButton, myExitButton, myStartButton, myQuitButton, backGround;

GLFWwindow *w = InitGLFW(100, 100, APP_WIDTH, APP_HEIGHT, "RoadRealm");

time_t oldtime = clock();
chrono::duration<double> gameClock;
int currNumRoads = 20;
double lastReplenishTime = 0.0;
string status = "Draw";

hash<string> STRING_HASH_FUN;

map<string, RoadRunnerLinker> ROAD_RUNNERS;

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
    infoPanel.AddMessage(FPS_LABEL, to_string(FRAMES_PER_SECONDS), WHITE);
}

Node ToggleNodeState(int col, int row, GridPrimitive &gridPrimitive, vector<NodePosition> &path, string &pathKey) {
    if (col < NCOLS && row < NROWS && col > -1 && row > -1) {

        int potentialIndex = CombineDigits(row, col);

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

bool LinkedPathFormulation(GridPrimitive &gridPrimitive, NodePosition homePos, NodePosition factoryPos,
                           const Vehicle &vehicleRunner, const string &pathHashKey) {
    bool updateLinkStatus = false;

    if (globalState == DRAW_STATE) {
        updateLinkStatus = gridPrimitive.UpdateDestinationLink(homePos, factoryPos, true);

        if (updateLinkStatus) {
            currNumRoads += 2;

            RoadRunnerLinker runnerLinker(pathHashKey, vehicleRunner, true);
            ROAD_RUNNERS.insert({pathHashKey, runnerLinker});
            currNumRoads -= (int) PREV_DRAGGED_CELLS.size();
        }
    }

    if (globalState == WIPE_STATE) {
        auto findRunner = ROAD_RUNNERS.find(pathHashKey);
        if (findRunner != ROAD_RUNNERS.end()) {
            updateLinkStatus = gridPrimitive.UpdateDestinationLink(homePos, factoryPos, false);
            if (updateLinkStatus) {
                cout << pathHashKey << endl;
                ROAD_RUNNERS.erase(findRunner);
                currNumRoads += (int) PREV_DRAGGED_CELLS.size() - 2;
            }
        }
    }
    return updateLinkStatus;
}

bool AreValidDraggedCells(GridPrimitive &gridPrimitive) {
    vec2 curCell, prevCell;

    int i = 0;
    float rowDiff = 0, colDiff = 0;
    bool partOfCross = false, isSameAxis = false;

    // Validate Cell
    vec2 ptlHouse = PREV_DRAGGED_CELLS.at(i);
    vec2 ptlFactory = PREV_DRAGGED_CELLS.at(PREV_DRAGGED_CELLS.size() - 1);

    Node houseNode = gridPrimitive.GetNode(ptlHouse);
    Node factoryNode = gridPrimitive.GetNode(ptlFactory);

    if (houseNode.currentState != CLOSED_HOUSE || factoryNode.currentState != CLOSED_FACTORY) {
        return isSameAxis;
    }

    while (i < PREV_DRAGGED_CELLS.size() - 1) {
        prevCell = PREV_DRAGGED_CELLS.at(i);
        curCell = PREV_DRAGGED_CELLS.at((i + 1));

        rowDiff = abs(curCell.y - prevCell.y);
        colDiff = abs(curCell.x - prevCell.x);

        partOfCross = (curCell.x == prevCell.x) || (curCell.y == prevCell.y);

        if (rowDiff > 1 || colDiff > 1 || !partOfCross) {
            isSameAxis = false;
            break;
        }

        isSameAxis = true;
        i += 1;
    }

    return isSameAxis;
}

void ToggleDraggedCellsStates(GridPrimitive &gridPrimitive) {
    if (!GLOBAL_MOUSE_DOWN && !PREV_DRAGGED_CELLS.empty()) {

        Vehicle vehicleRunner(-.2f, .4f);
        NodePosition homePos, factoryPos;

        int counter = 0, homeIndex = -1, factoryIndex = -1;

        string pathHashKey;

        bool isValidDrag = AreValidDraggedCells(gridPrimitive);

        if (isValidDrag) {

            for (const vec2 &cell: PREV_DRAGGED_CELLS) {

                Node getNode = ToggleNodeState((int) cell.x, (int) cell.y, gridPrimitive, vehicleRunner.runnerPath,
                                               pathHashKey);

                if (getNode.currentState == CLOSED_HOUSE) {
                    vehicleRunner.overlayColor = getNode.overlayColor;
                    homePos = getNode.currentPos;
                    homeIndex = counter;
                }
                if (getNode.currentState == CLOSED_FACTORY) {
                    factoryPos = getNode.currentPos;
                    factoryIndex = counter;
                }
                counter += 1;
            }
            cout << "Route: " << pathHashKey << "\n";

            // Home --TO--> Factory Order
            if (homeIndex < factoryIndex && (int) PREV_DRAGGED_CELLS.size() <= currNumRoads) {
                LinkedPathFormulation(gridPrimitive, homePos, factoryPos, vehicleRunner, pathHashKey);

            } else {
                cout << "Not enough road! Or Invalid Path Selection" << endl;
            }

        }

        cout << "Dragged Status:\t" << isValidDrag << "\n";

        PREV_DRAGGED_CELLS.clear();
        CURRENT_CLICKED_CELL = vec2((NROWS + NCOLS), (NROWS + NCOLS));
    } else {
        if (CURRENT_CLICKED_CELL.x < NCOLS && CURRENT_CLICKED_CELL.y < NROWS
            && CURRENT_CLICKED_CELL.x > -1 && CURRENT_CLICKED_CELL.y > -1) {
            gridPrimitive.NodeHandler(
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

        PREV_DRAGGED_CELLS.push_back(CURRENT_CLICKED_CELL);

        return true;
    }
    return false;
}

bool AccumulateDraggedCell(int col, int row) {
    if (!ClickedCellHandled(col, row)) {

        CURRENT_CLICKED_CELL = vec2(col, row);

        PREV_DRAGGED_CELLS.push_back(CURRENT_CLICKED_CELL);

        return true;
    }
    return false;
}

void ResetGameState(GridPrimitive &gridPrimitive) {
    if (ACTIVE_GAME_RESET) {
        GLOBAL_MOUSE_DOWN = false;
        GLOBAL_PAUSE = false;
        GLOBAL_DRAW_BORDERS = false;

        currNumRoads = 10;
        lastReplenishTime = 0.0;
        status = "Draw";

        infoPanel.AddMessage(ERROR_MSG_LABEL, "", WHITE);
        infoPanel.AddMessage(LOGS_MSG_LABEL, "Reset", WHITE);

        gameClock = chrono::duration<double>(0);

        PREV_DRAGGED_CELLS.clear();
        ROAD_RUNNERS.clear();
        gridPrimitive.GridReset();

        for (int i = 0; i < 5; i++) {
            vec2 rndStPoint = GetRandomPoint();
            vec2 rndEdPoint = GetRandomPoint();

            gridPrimitive.AddNewObjective((int) rndStPoint.y, (int) rndStPoint.x, (int) rndEdPoint.y,
                                          (int) rndEdPoint.x);
        }
    }

    ACTIVE_GAME_RESET = false;
}

void MouseButton(float xmouse, float ymouse, bool left, bool down) {
    if (down) {
        GLOBAL_MOUSE_DOWN = true;

        if (application == GAME_STATE && myResetButton.Hit(xmouse, ymouse)) {
            ACTIVE_GAME_RESET = true;
            //resetGameState();
        } else if (application == GAME_STATE && myExitButton.Hit(xmouse, ymouse)) {
            application = STARTING_MENU;
            ACTIVE_GAME_RESET = true;
            //resetGameState();
            infoPanel.AddMessage(LOGS_MSG_LABEL, "", WHITE);
        } else if (application == STARTING_MENU && myStartButton.Hit(xmouse, ymouse)) {
            application = GAME_STATE;
        } else if (application == STARTING_MENU && myQuitButton.Hit(xmouse, ymouse)) {
            glfwSetWindowShouldClose(w, true);
        } else {
            AccumulateDraggedCell(xmouse, ymouse);
        }
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

    if (application == STARTING_MENU) {
        FONT_SCALE = 13.0f;

        backGround.Display();
        myStartButton.Display();
        myQuitButton.Display();
    }
    if (application == GAME_STATE) {
        FONT_SCALE = 12.0f;
        myResetButton.Display();
        myExitButton.Display();

        gridPrimitive.DrawGrid();

        for (auto &runnerLinkers: ROAD_RUNNERS) {
            string runnerDrawLog;
            runnerLinkers.second.vehicleRunner.Draw(runnerDrawLog);

            infoPanel.AddMessage(LOGS_MSG_LABEL, runnerDrawLog, WHITE);
        }
        if (GLOBAL_DRAW_BORDERS) {
            DrawBorders();
        }
    }

    infoPanel.InfoDisplay(FONT_SCALE);

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

int main(int ac, char **av) {
    myResetButton.Initialize("../RoadNet/Images/resetButton.png");
    myExitButton.Initialize("../RoadNet/Images/exitButton.png");
    myStartButton.Initialize("../RoadNet/Images/startButton.png");
    myQuitButton.Initialize("../RoadNet/Images/quitButton.png");
    backGround.Initialize("../RoadNet/Images/background.jpg");

    RegisterMouseButton(MouseButton);
    RegisterMouseMove(MouseMove);
    RegisterResize(Resize);
    RegisterKeyboard(KeyButton);

    // Starting Menu Buttons Transformation
    myStartButton.SetScale(vec2(.2f, .2f));
    myStartButton.SetPosition(vec2(-.5f, -.5f));
    myQuitButton.SetScale(vec2(.2f, .2f));
    myQuitButton.SetPosition(vec2(.5f, -.5f));

    // Game State Menu Button Transformations
    myResetButton.SetScale(vec2(.1f, .1f));
    myResetButton.SetPosition(vec2(.7f, -.5f));

    myExitButton.SetScale(vec2(.1f, .1f));
    myExitButton.SetPosition(vec2(.7f, -.75f));

    INIT_FPS_TIME = glfwGetTime();

    GridPrimitive gridPrimitive;

    while (!glfwWindowShouldClose(w)) {



        FRAMES_PER_SECONDS = NUM_OF_FRAMES / (glfwGetTime() - INIT_FPS_TIME);

        Display(gridPrimitive);



        NUM_OF_FRAMES += 1;

        if (application == GAME_STATE) {
            ToggleDraggedCellsStates(gridPrimitive);
            ResetGameState(gridPrimitive);
        }

        glfwSwapBuffers(w);
        glfwPollEvents();
    }
}