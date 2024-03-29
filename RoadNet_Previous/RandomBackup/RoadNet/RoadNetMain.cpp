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
#include <fstream>
#include "Sprite.h"
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

using namespace std;

unsigned int NUM_OF_FRAMES = 0;
double INIT_FPS_TIME = 0;
bool GLOBAL_MOUSE_DOWN = false, GLOBAL_PAUSE = false, GLOBAL_DRAW_BORDERS = false, ACTIVE_GAME_RESET = false, GAME_OVER = false, CLEAR_ROADS = false;
// In Seconds
double REPLENISH_INTERVAL = 10.0, FRAMES_PER_SECONDS = 0;
int REPLENISH_ROADS_NUM = 20;
float FONT_SCALE = 10.0f;

int PAIR_GENERATION_RETRY = 3;
string LOCAL_STORAGE = "RoadNet/Storage/best_record.txt";

vec2 CURRENT_CLICKED_CELL((NROWS + NCOLS), (NROWS + NCOLS));

vector<vec2> PREV_DRAGGED_CELLS;

InfoPanel infoPanel;
Sprite myResetButton, myExitButton, myStartButton, myQuitButton, backGround, myPauseButton, myResumeButton, myClearButton;

GLFWwindow *w = InitGLFW(100, 100, APP_WIDTH, APP_HEIGHT, "RoadRealm");

time_t oldtime = clock();
chrono::duration<double> gameClock;
int currNumRoads = 20;
double lastReplenishTime = 0.0;
double lastPairSpawnTime = 0.0;
float countDown = 5.0f;
float bufferTime = 5.0f;

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

void setBestRecord(double bestRecord) {
    std::ofstream file(LOCAL_STORAGE);

    if (file.is_open()) {
        file << bestRecord;
    }

    file.close();
}

double getBestRecord() {
    std::ifstream file(LOCAL_STORAGE);
    double longestDuration = 0.0;

    if (file.is_open()) {
        file >> longestDuration;
    }

    file.close();

    return longestDuration;
}

void checkAndSaveBestRecord(chrono::duration<double> gameClock) {
    double newRecord = gameClock.count();
    double currentBestScore = getBestRecord();

    if (newRecord > currentBestScore) {
        setBestRecord(newRecord);
    }
}

void replenishRoads() {
    if (gameClock.count() - lastReplenishTime >= REPLENISH_INTERVAL) {
        currNumRoads += REPLENISH_ROADS_NUM;
        cout << "Just replenished " << REPLENISH_ROADS_NUM << " roads to player" << endl;

        lastReplenishTime = gameClock.count();
    }
}


void GenerateDestination(GridPrimitive &gridPrimitive, int radius = 3, int retryCount = 3) {

    vec2 rndStPoint;
    vec2 rndEdPoint;

    bool addStatus = false;

    do {
        rndStPoint = GetRandomPoint();
        rndEdPoint = GetRandomPoint(5, radius, rndStPoint);


    }while(gridPrimitive.IsAClosedNodeState(rndStPoint, true) || gridPrimitive.IsAClosedNodeState(rndEdPoint, true));

    addStatus = gridPrimitive.AddNewObjective((int) rndStPoint.y, (int) rndStPoint.x, (int) rndEdPoint.y,
                                              (int) rndEdPoint.x);

    if (!addStatus && retryCount > 0) {
        GenerateDestination(gridPrimitive, radius + 1, retryCount - 1);
    }

    if (APPLICATION_STATE == GAME_STATE && addStatus) {
        PlaySound(TEXT("RoadNet/Sounds/chime.wav"), NULL, SND_FILENAME | SND_ASYNC);
    }
}

void spawnPair(int interval, GridPrimitive &gridPrimitive, int radius = 2) {
    if (gameClock.count() - lastPairSpawnTime >= interval) {
        GenerateDestination(gridPrimitive, radius, PAIR_GENERATION_RETRY);
        lastPairSpawnTime = gameClock.count();
    }
}


void Update(GridPrimitive &gridPrimitive) {

    time_t now = clock();
    float dt;

    if (APPLICATION_STATE == GAME_STATE && !GLOBAL_PAUSE) {
        dt = (float) (now - oldtime) / CLOCKS_PER_SEC;


        oldtime = now;
        for (auto &runnerLinkers: ROAD_RUNNERS) {
            runnerLinkers.second.vehicleRunner.Update(dt);
        }

        gameClock += chrono::duration<double>(dt);

        if (!gridPrimitive.IsAllDestinationLinked()) {
            if (bufferTime > 0) {
                bufferTime -= dt;
            } else {
                countDown -= dt;
                if (countDown <= 0.0f) {
                    PlaySound(TEXT("RoadNet/Sounds/game_over.wav"), NULL, SND_FILENAME | SND_ASYNC);
                    checkAndSaveBestRecord(gameClock);
                    GAME_OVER = true;
                    ACTIVE_GAME_RESET = true;
                    countDown = 5.0f;
                    bufferTime = 5.0f;
                    APPLICATION_STATE = STARTING_MENU;
                }
            }
        } else {
            countDown = 5.0f;  // reset countdown if all destinations are linked
            bufferTime = 5.0f;
        }
        int radius = ((((int) gameClock.count() % 3600) / 60) + 1) * 2;
        spawnPair(5, gridPrimitive, radius);
        replenishRoads();
    } else {
        oldtime = now;
    }

    gridPrimitive.GridUpdate();
    // EVENT_LABEL
    infoPanel.AddMessage(TIME_LABEL, "Time: " + formatDuration(gameClock), WHITE);
    infoPanel.AddMessage(DIMS_LABEL, "Grid DIM: (" + to_string(NROWS) + " by " + to_string(NCOLS) + ")", WHITE);
    infoPanel.AddMessage(NUM_OF_ROAD_LABEL, "Number of Roads: " + to_string(currNumRoads), WHITE);
    infoPanel.AddMessage(APPLICATION_STATE_LABEL, PrintApplicationState(), PURPLE);
    infoPanel.AddMessage(GAMEPLAY_STATE_LABEL, PrintGameplayState(), PURPLE);
    infoPanel.AddMessage(FPS_LABEL, ("FPS: " + to_string(FRAMES_PER_SECONDS)), WHITE);
    infoPanel.AddMessage(RUNNERS_COUNT_LABEL, ("Total Runners: " + to_string(ROAD_RUNNERS.size())), YELLOW);
    infoPanel.AddMessage(EVT_MSG_LABEL, GLOBAL_EVENT_LABEL, CYAN);
}

bool LinkedPathFormulation(GridPrimitive &gridPrimitive, NodePosition homePos, NodePosition factoryPos,
                           const Vehicle &vehicleRunner, const string &pathHashKey) {
    bool updateLinkStatus = false;

    if (GLOBAL_GAMEPLAY_STATE == DRAW_STATE) {
        updateLinkStatus = gridPrimitive.UpdateDestinationLink(homePos, factoryPos, true);

        if (updateLinkStatus) {
            currNumRoads += 2;

            RoadRunnerLinker runnerLinker(pathHashKey, vehicleRunner, true);
            ROAD_RUNNERS.insert({pathHashKey, runnerLinker});
            currNumRoads -= (int) PREV_DRAGGED_CELLS.size();

            infoPanel.AddMessage(ERROR_MSG_LABEL, "Valid Linking", GREEN);
        }
    }

    if (GLOBAL_GAMEPLAY_STATE == WIPE_STATE) {
        auto findRunner = ROAD_RUNNERS.find(pathHashKey);
        if (findRunner != ROAD_RUNNERS.end()) {
            updateLinkStatus = gridPrimitive.UpdateDestinationLink(homePos, factoryPos, false);
            if (updateLinkStatus) {
                cout << pathHashKey << endl;
                ROAD_RUNNERS.erase(findRunner);
                currNumRoads += (int) PREV_DRAGGED_CELLS.size() - 2;

                infoPanel.AddMessage(ERROR_MSG_LABEL, "Valid Linking", GREEN);
            }
        }
    }
    return updateLinkStatus;
}

bool AreValidDraggedCells(GridPrimitive &gridPrimitive, Node &houseNode, Node &factoryNode) {
    vec2 curCell, prevCell;

    int i = 0;
    float rowDiff = 0, colDiff = 0;
    bool partOfCross = false;

    // Validate Cell
    vec2 ptlHouse = PREV_DRAGGED_CELLS.at(i);
    vec2 ptlFactory = PREV_DRAGGED_CELLS.at(PREV_DRAGGED_CELLS.size() - 1);

    houseNode = gridPrimitive.GetNode(ptlHouse);
    factoryNode = gridPrimitive.GetNode(ptlFactory);

    if (houseNode.currentState != CLOSED_HOUSE || factoryNode.currentState != CLOSED_FACTORY) {
        return false;
    }

    while (i < PREV_DRAGGED_CELLS.size() - 1) {
        prevCell = PREV_DRAGGED_CELLS.at(i);
        curCell = PREV_DRAGGED_CELLS.at((i + 1));

        rowDiff = abs(curCell.y - prevCell.y);
        colDiff = abs(curCell.x - prevCell.x);

        partOfCross = (curCell.x == prevCell.x) || (curCell.y == prevCell.y);

        if (rowDiff > 1 || colDiff > 1 || !partOfCross) {
            return false;
        }
        // Check if Closed Node State
        if (i > 0 && gridPrimitive.IsAClosedNodeState(prevCell, true)) {
            return false;
        }
        if (((int) PREV_DRAGGED_CELLS.size() > currNumRoads) && GLOBAL_GAMEPLAY_STATE == DRAW_STATE) {
            infoPanel.AddMessage(ERROR_MSG_LABEL, "Not enough road!", RED);
            return false;
        }
        i += 1;
    }
    return true;
}

void ToggleDraggedCellsStates(GridPrimitive &gridPrimitive) {
    if (!GLOBAL_MOUSE_DOWN && !PREV_DRAGGED_CELLS.empty()) {

        Vehicle vehicleRunner(-.55f, .0f);
        Node houseNode, factoryNode;
        string pathHashKey;

        bool isErrorCorrect = AreValidDraggedCells(gridPrimitive, houseNode, factoryNode);

        if (isErrorCorrect) {

            for (const vec2 &cell: PREV_DRAGGED_CELLS) {

                Node getNode = gridPrimitive.GetNode(vec2(cell.x, cell.y));
                // Hash Key
                pathHashKey += to_string(getNode.currentPos.row) + to_string(getNode.currentPos.col);
                // Add To Path
                vehicleRunner.runnerPath.push_back(getNode.currentPos);

                if (getNode.currentState != CLOSED_HOUSE && getNode.currentState != CLOSED_FACTORY) {
                    vehicleRunner.overlayColor = houseNode.overlayColor;
                }
            }
            // Link Result
            isErrorCorrect = LinkedPathFormulation(gridPrimitive, houseNode.currentPos, factoryNode.currentPos,
                                                   vehicleRunner,
                                                   pathHashKey);
            // Toggle/Handle Node State If Successful
            if (isErrorCorrect) {
                cout << "Is Correct Linking\n";
                gridPrimitive.ResetNodes(PREV_DRAGGED_CELLS, false);
            }
        }

        if (!isErrorCorrect) {
            PlaySound(TEXT("RoadNet/Sounds/error.wav"), NULL, SND_FILENAME | SND_ASYNC);
            cout << "Is Not Correct Dragging Linking\n";
            gridPrimitive.ResetNodes(PREV_DRAGGED_CELLS, true);
            infoPanel.AddMessage(ERROR_MSG_LABEL, "InValid Linking", RED);
        }

        PREV_DRAGGED_CELLS.clear();
        CURRENT_CLICKED_CELL = vec2((NROWS + NCOLS), (NROWS + NCOLS));
    } else {
        if (CURRENT_CLICKED_CELL.x < NCOLS && CURRENT_CLICKED_CELL.y < NROWS
            && CURRENT_CLICKED_CELL.x > -1 && CURRENT_CLICKED_CELL.y > -1) {
            gridPrimitive.NodeHandler(
                    CombineDigits((int) CURRENT_CLICKED_CELL.y, (int) CURRENT_CLICKED_CELL.x));
        }
        CURRENT_CLICKED_CELL = vec2((NROWS + NCOLS), (NROWS + NCOLS));
    }
}


bool ClickedCellHandled(int col, int row) {
    if (col >= NCOLS || row >= NROWS) {
        infoPanel.AddMessage(ERROR_MSG_LABEL, "Out Of Grid Mouse Click", RED);
        // Assuming Out of Bounds
        return true;
    }
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
        GLOBAL_GAMEPLAY_STATE = DRAW_STATE;
        GLOBAL_MOUSE_DOWN = false;
        GLOBAL_PAUSE = false;
        GLOBAL_DRAW_BORDERS = false;

        currNumRoads = 20;
        lastReplenishTime = 0.0;
        lastPairSpawnTime = 0.0;
        countDown = 5.0f;
        bufferTime = 5.0f;

        DIAMETER_LENGTH = 0;
        DIAMETER_PERCENT = 0;
        REDUCE_DIAMETER = true;

        gameClock = chrono::duration<double>(0);

        PREV_DRAGGED_CELLS.clear();
        ROAD_RUNNERS.clear();
        gridPrimitive.GridReset();
    }
    if (CLEAR_ROADS) {
        ROAD_RUNNERS.clear();
        int numRoadsAfterClear = gridPrimitive.GridClearAndCountRoads();
        currNumRoads += numRoadsAfterClear;
    }
    CLEAR_ROADS = false;
    ACTIVE_GAME_RESET = false;
}

void MouseButton(float xmouse, float ymouse, bool left, bool down) {

    int col = (int) ((xmouse - X_POS) / DX), row = (int) ((ymouse - Y_POS) / DY);
    infoPanel.AddMessage(MOUSE_CLICK_LABEL, "Mouse Move: X" + to_string(col) + " Y " + to_string(row), WHITE);

    if (down) {
        PlaySound(TEXT("RoadNet/Sounds/click_x.wav"), NULL, SND_FILENAME | SND_ASYNC);
        GLOBAL_MOUSE_DOWN = true;

        if (APPLICATION_STATE == GAME_STATE && myResetButton.Hit(xmouse, ymouse)) {
            GLOBAL_EVENT_LABEL = "RESET_EVT";

            ACTIVE_GAME_RESET = true;
        } else if (APPLICATION_STATE == GAME_STATE && myPauseButton.Hit(xmouse, ymouse)) {
            GLOBAL_PAUSE = !GLOBAL_PAUSE;
            GLOBAL_EVENT_LABEL = "PLAY_EVT";
            if (GLOBAL_PAUSE) {
                GLOBAL_EVENT_LABEL = "PAUSE_EVT";
            }
        } else if (APPLICATION_STATE == GAME_STATE && myExitButton.Hit(xmouse, ymouse)) {
            GLOBAL_EVENT_LABEL = "EXIT_EVT";

            APPLICATION_STATE = STARTING_MENU;
            ACTIVE_GAME_RESET = true;
        } else if (APPLICATION_STATE == GAME_STATE && myClearButton.Hit(xmouse, ymouse)) {
            GLOBAL_EVENT_LABEL = "CLEAR_EVT";

            CLEAR_ROADS = true;

        } else if (APPLICATION_STATE == STARTING_MENU && myStartButton.Hit(xmouse, ymouse)) {
            GLOBAL_EVENT_LABEL = "START_EVT";

            GAME_OVER = false;
            APPLICATION_STATE = GAME_STATE;
        } else if (APPLICATION_STATE == STARTING_MENU && myQuitButton.Hit(xmouse, ymouse)) {
            GLOBAL_EVENT_LABEL = "QUIT_EVT";

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
                PlaySound(TEXT("RoadNet/Sounds/click_x.wav"), NULL, SND_FILENAME | SND_ASYNC);
                GLOBAL_PAUSE = !GLOBAL_PAUSE;
                GLOBAL_EVENT_LABEL = "PLAY_EVT";
                if (GLOBAL_PAUSE) {
                    GLOBAL_EVENT_LABEL = "PAUSE_EVT";
                }
                break;
            case GLFW_KEY_SPACE:
                PlaySound(TEXT("RoadNet/Sounds/click_x.wav"), NULL, SND_FILENAME | SND_ASYNC);
                GLOBAL_PAUSE = !GLOBAL_PAUSE;
                GLOBAL_EVENT_LABEL = "PLAY_EVT";
                if (GLOBAL_PAUSE) {
                    GLOBAL_EVENT_LABEL = "PAUSE_EVT";
                }
                break;
            case GLFW_KEY_D:
                PlaySound(TEXT("RoadNet/Sounds/click_x.wav"), NULL, SND_FILENAME | SND_ASYNC);
                if (GLOBAL_GAMEPLAY_STATE == WIPE_STATE) {
                    GLOBAL_GAMEPLAY_STATE = DRAW_STATE;
                } else {
                    GLOBAL_GAMEPLAY_STATE = WIPE_STATE;
                }
                break;
            case GLFW_KEY_W:
                PlaySound(TEXT("RoadNet/Sounds/click_x.wav"), NULL, SND_FILENAME | SND_ASYNC);
                GLOBAL_GAMEPLAY_STATE = WIPE_STATE;
                break;
            case GLFW_KEY_B:
                PlaySound(TEXT("RoadNet/Sounds/click_x.wav"), NULL, SND_FILENAME | SND_ASYNC);
                GLOBAL_DRAW_BORDERS = !GLOBAL_DRAW_BORDERS;

                GLOBAL_EVENT_LABEL = "BORDER_DISP_EVT";
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

    if (APPLICATION_STATE == STARTING_MENU) {
        FONT_SCALE = 13.0f;

        backGround.Display();
        myStartButton.Display();
        myQuitButton.Display();

        // Best record
        double bestRecord = getBestRecord();
        if (bestRecord != 0.0) {
            chrono::duration<double> longestDuration(bestRecord);
            Text(GLOBAL_W / 2 - 95, GLOBAL_H / 2 + 50, BLACK, FONT_SCALE,
                 ("BEST RECORD: " + formatDuration(longestDuration)).c_str());
        } else {
            Text(GLOBAL_W / 2 - 95, GLOBAL_H / 2 + 50, BLACK, FONT_SCALE, "BEST RECORD: 00H00M00S");
        }

        if (GAME_OVER) {
            Text(GLOBAL_W / 2 - 85, GLOBAL_H / 2 + 100, RED, 30.0f, "Game Over");
            infoPanel.AddMessage(COUNTDOWN, " ", WHITE);
        }
    }
    if (APPLICATION_STATE == GAME_STATE) {
        if (gameClock.count() < 0.05) {
            PlaySound(TEXT("RoadNet/Sounds/call_to_arms.wav"), NULL, SND_FILENAME | SND_ASYNC);
        }
        FONT_SCALE = 12.0f;
        myResetButton.Display();
        myExitButton.Display();
        myClearButton.Display();
        if (GLOBAL_PAUSE) {
            myPauseButton.Display();
        } else {
            myResumeButton.Display();
        }

        gridPrimitive.DrawGrid();

        for (auto &runnerLinkers: ROAD_RUNNERS) {
            string runnerDrawLog;
            runnerLinkers.second.vehicleRunner.Draw(runnerDrawLog);
        }
        if (GLOBAL_DRAW_BORDERS) {
            DrawBorders();
        }
        stringstream stream;
        stream << fixed << setprecision(3) << countDown;
        string countDownFormatted = stream.str();

        if (countDown < 5.0f) {
            infoPanel.AddMessage(COUNTDOWN, "Countdown: " + countDownFormatted + "S", RED);
        } else {
            infoPanel.AddMessage(COUNTDOWN, "Countdown: " + countDownFormatted + "S", GREEN);
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
    myResetButton.Initialize("RoadNet/Images/resetButton.png");
    myExitButton.Initialize("RoadNet/Images/exitButton.png");
    myStartButton.Initialize("RoadNet/Images/startButton.png");
    myQuitButton.Initialize("RoadNet/Images/quitButton.png");
    backGround.Initialize("RoadNet/Images/background.jpg");
    myPauseButton.Initialize("RoadNet/Images/pauseButton.png");
    myResumeButton.Initialize("RoadNet/Images/resumeButton.png");
    myClearButton.Initialize("RoadNet/Images/clearButton.png");

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

    myPauseButton.SetScale(vec2(.1f, .1f));
    myPauseButton.SetPosition(vec2(.7f, 0.0f));

    myResumeButton.SetScale(vec2(.1f, .1f));
    myResumeButton.SetPosition(vec2(.7f, 0.0f));

    myClearButton.SetScale(vec2(.1f, .1f));
    myClearButton.SetPosition(vec2(.7f, -.25f));

    INIT_FPS_TIME = glfwGetTime();
    PlaySound(TEXT("RoadNet/Sounds/program_start.wav"), NULL, SND_FILENAME | SND_ASYNC);
    GridPrimitive gridPrimitive;
    while (!glfwWindowShouldClose(w)) {
        Update(gridPrimitive);

        FRAMES_PER_SECONDS = NUM_OF_FRAMES / (glfwGetTime() - INIT_FPS_TIME);

        Display(gridPrimitive);

        NUM_OF_FRAMES += 1;

        if (APPLICATION_STATE == GAME_STATE) {
            ToggleDraggedCellsStates(gridPrimitive);
            ResetGameState(gridPrimitive);
        }

        glfwSwapBuffers(w);
        glfwPollEvents();
    }
}