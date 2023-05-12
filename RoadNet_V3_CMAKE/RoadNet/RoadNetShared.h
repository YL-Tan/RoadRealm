// RoadNetShared.h
// Team 8

#ifndef ROADREALM_ROADNETSHARED_H
#define ROADREALM_ROADNETSHARED_H

#include <iostream>
#include <vector>
#include "Draw.h"
#include <string>
#include <random>
#include "GLXtras.h"

using namespace std;

#define NROWS 12
#define NCOLS 10
#define H_EDGE_BUFFER 40
#define W_EDGE_BUFFER 100
#define INFO_MSG_SIZE 9

int APP_WIDTH = 1000, APP_HEIGHT = 800, X_POS = 20, Y_POS = 20,
        GLOBAL_W = APP_WIDTH - W_EDGE_BUFFER, GLOBAL_H = APP_HEIGHT - H_EDGE_BUFFER;

int GRID_W = GLOBAL_W * 0.75, DISP_W = GLOBAL_W - (GLOBAL_W * 0.22); // +  150;
float DX = (float) GRID_W / NCOLS, DY = (float) GLOBAL_H / NROWS;

const vec3 WHITE(1, 1, 1), BLACK(0, 0, 0), GREY(.5, .5, .5), RED(1, 0, 0),
        GREEN(0, 1, 0), BLUE(0, 0, 1), YELLOW(1, 1, 0),
        ORANGE(1, .55f, 0), PURPLE(.8f, .1f, .5f), CYAN(0, 1, 1), PALE_GREY(.8, .8, .8);

enum NodeStates {
    OPEN, CLOSED_ROAD, CLOSED_HOUSE, CLOSED_FACTORY, POTENTIAL_ROAD
};

enum GameplayState {
    DRAW_STATE, WIPE_STATE
};

enum ApplicationStates {
    STARTING_MENU, GAME_STATE
};

enum InfoLabelsIndex {
    MOUSE_CLICK_LABEL = 0,
    MOUSE_MOVE_LABEL = 1,
    DIMS_LABEL = 2,
    FPS_LABEL = 3,
    TIME_LABEL = 4,
    NUM_OF_ROAD_LABEL = 5,
    GRID_STATE_LABEL = 6,
    ERROR_MSG_LABEL = 7,
    LOGS_MSG_LABEL = 8
};

struct InfoPanel {

    struct Message {
        string msgInfo = "";
        vec3 msgColor = WHITE;

        Message(string info, const vec3 &color) {
            this->msgInfo = info;
            this->msgColor = color;
        }
    };

    vector<Message> infoPanelMessages = {};

    InfoPanel() {
        for (int i = 0; i < INFO_MSG_SIZE; i++) {
            infoPanelMessages.push_back(Message("", WHITE));
        }
    }

    void InfoDisplay(float fontScale) {
        int maxHeight = GLOBAL_H;
        for (Message &message: infoPanelMessages) {
            Text(DISP_W + 5, maxHeight, message.msgColor, fontScale, message.msgInfo.c_str());

            maxHeight -= 20;
        }
    }

    bool AddMessage(InfoLabelsIndex labelsIndex, const string &msg, const vec3 &msgColor) {
        if (!msg.empty()) {
            // Add Message To Vector
            infoPanelMessages.at(labelsIndex).msgInfo = msg;
            infoPanelMessages.at(labelsIndex).msgColor = msgColor;

            return true;
        }
        return false;
    }
};

struct NodePosition {
    int row = -1, col = -1;

    NodePosition() {};

    NodePosition(int r, int c) : row(r), col(c) {};

    float DistanceTo(NodePosition i) {
        float drow = (float) (i.row - row), dcol = (float) (i.col - col);
        return sqrt(drow * drow + dcol * dcol);
    }

    bool Valid() { return row >= 0 && row < NROWS && col >= 0 && col < NCOLS; }

    vec2 Point() { return vec2(X_POS + (col + .5) * DX, Y_POS + (row + .5) * DY); }

    vec4 PosWindowProj() {
        return vec4((int) (X_POS + DX * (float) col), (int) (Y_POS + DY * (float) row),
                    (int) DX - 1, (int) DY - 1);
    }

    bool AlignmentPosMatches(int r, int c) {
        if (row == r && col == c) {
            return true;
        }
        return false;
    }

    bool AlignmentPosMatches(NodePosition comparePos) {
        return AlignmentPosMatches(comparePos.row, comparePos.col);
    }

    bool operator==(NodePosition &i) { return i.row == row && i.col == col; }
};

void DrawBorders() {
    // Top Line, (Top-Right -> Top-Left)
    Line(vec2(GLOBAL_W, GLOBAL_H + (H_EDGE_BUFFER / 2) + 1), vec2(5, GLOBAL_H + (H_EDGE_BUFFER / 2) + 1), 1, WHITE);
    // Left Line (Bottom-Left -> Top-Left)
    Line(vec2(5, 5), vec2(5, GLOBAL_H + (H_EDGE_BUFFER / 2) + 1), 1, WHITE);

    // Right Line (Top-Right -> Bottom-Right)
    Line(vec2(GLOBAL_W, GLOBAL_H + (H_EDGE_BUFFER / 2) + 1), vec2(GLOBAL_W, 5), 1, WHITE);

    // Mid Line, (Mid-Right, Mid-Left)
    Line(vec2(DISP_W, GLOBAL_H + (H_EDGE_BUFFER / 2) + 1), vec2(DISP_W, 5), 1, WHITE);

    // Bottom Line (Bottom-Left -> Bottom-Right)
    Line(vec2(5, 5), vec2(GLOBAL_W, 5), 1, WHITE);
}

void GetRandomDistribution(int distribLimit, int numOfRndValues, vector<int> &distribRndPlacement) {
    random_device generator;
    uniform_int_distribution<int> distribution(0, distribLimit);

    for (int i = 0; i < numOfRndValues; i++) {
        distribRndPlacement.push_back(distribution(generator));
    }
}

vec2 GetRandomPoint() {
    // Limit will be the Dimensions
    vector<int> rndNodePoint = {};

    GetRandomDistribution(NROWS, 2, rndNodePoint);

    return {(rndNodePoint.at(0) % NCOLS), (rndNodePoint.at(1) % NROWS)};
}

vec3 GetRandomColor(int stride = 1) {
    int maxColorShades = 255;

    vector<int> rndDistribColors = {};

    GetRandomDistribution(maxColorShades, 3, rndDistribColors);

    float red = (float) ((rndDistribColors.at(0) + stride) % maxColorShades) / (float) maxColorShades;
    float green = (float) ((rndDistribColors.at(1) + stride) % maxColorShades) / (float) maxColorShades;
    float blue = (float) ((rndDistribColors.at(2) + stride) % maxColorShades) / (float) maxColorShades;

    return {red, green, blue};
}

void DrawVertex(float row, float col) {
    Disk(vec2(X_POS + (col + .5) * DX, Y_POS + (row + .5) * DY), 6, BLUE);
}

void DrawSegment(NodePosition i1, NodePosition i2, float width, vec3 col) {
    Line(i1.Point(), i2.Point(), width, col, col);
}

void DrawPath(vector<NodePosition> &path, float width, vec3 color) {
    for (size_t i = 1; i < path.size(); i++)
        DrawSegment(path.at(i - 1), path.at(i), width, color);
}

void DrawRectangle(int x, int y, int w, int h, vec3 col) {
    Quad(x, y, x, y + h, x + w, y + h, x + w, y, true, col);
}

float PathLength(vector<NodePosition> &path) {
    float len = 0;
    for (size_t i = 1; i < path.size(); i++)
        len += path[i].DistanceTo(path[i - 1]);
    return len;
}

vec2 PointOnPath(float dist, vector<NodePosition> &path) {
    float accumDist = 0;
    NodePosition i1 = path[0];
    for (size_t i = 1; i < path.size(); i++) {
        NodePosition i2 = path[i];
        float d = i1.DistanceTo(i2);
        accumDist += d;
        if (accumDist >= dist) {
            float alpha = (accumDist - dist) / d;
            return vec2(i2.col + alpha * (i1.col - i2.col), i2.row + alpha * (i1.row - i2.row));
        }
        i1 = i2;
    }
    return vec2(i1.col, i1.row);
}

int GetHighestTenthPow(int digit) {
    int highestDeciPow = 10;
    while (digit >= highestDeciPow) {
        highestDeciPow *= 10;
    }
    return highestDeciPow;
}

int CombineDigits(int leftDigit, int rightDigit) {
    //return (leftDigit * GetHighestTenthPow(rightDigit)) + rightDigit;
    return (NCOLS * leftDigit) + rightDigit;
}

struct Node {
    NodePosition currentPos;
    vec3 color = WHITE;
    vec3 overlayColor = WHITE;
    NodeStates currentState = OPEN;
    NodeStates transState = POTENTIAL_ROAD;

    Node() {}

    Node(int row, int col) {
        currentPos.row = row;
        currentPos.col = col;
    }

    void NodeReset() {
        this->color = WHITE;
        this->overlayColor = WHITE;
        this->currentState = OPEN;
        this->transState = POTENTIAL_ROAD;
    }
};

struct Vehicle {
    float speed = -.5, t = 1;
    vec3 overlayColor;
    vector<NodePosition> runnerPath;

    Vehicle() {}

    Vehicle(float s, float tt) {
        this->speed = s;
        this->t = tt;
    }

    Vehicle(float s, float tt, const vec3 &color, const vector<NodePosition> &path) {
        this->speed = s;
        this->t = tt;
        this->overlayColor = color;
        this->runnerPath = path;
    }

    void Update(float dt) {
        t += dt * speed;
        if (t < 0 || t > 1) speed = -speed;
        t = t < 0 ? 0 : t > 1 ? 1 : t;
    }

    void Draw(string &drawLogs) {

        if (!runnerPath.empty()) {
            // Draw Path Using Vehicle Color
            DrawPath(runnerPath, 2.5f, overlayColor);

            float dist = t * PathLength(runnerPath);
            vec2 p = PointOnPath(dist, runnerPath);

            // Draw Disk Dot
            Disk(vec2(X_POS + (p.x + .5) * DX, Y_POS + (p.y + .5) * DY), 20, overlayColor);

            drawLogs = "runner pos: " + to_string(p.x) + " " + to_string(p.y);
        }
    }
};

struct RoadRunnerLinker {
    size_t hashedId;
    bool isLinked = false;

    // vec3 roadColor;
    Vehicle vehicleRunner;
    // vector<NodePosition> roadPath;

    RoadRunnerLinker(size_t hashedVal, const Vehicle &runner, bool linkStatus) {
        this->hashedId = hashedVal;
        this->vehicleRunner = runner;
        this->isLinked = linkStatus;
    }
};


#endif //ROADREALM_ROADNETSHARED_H
