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
#define W_EDGE_BUFFER 200

int APP_WIDTH = 1000, APP_HEIGHT = 800, X_POS = 20, Y_POS = 20,
        GLOBAL_W = APP_WIDTH - W_EDGE_BUFFER, GLOBAL_H = APP_HEIGHT - H_EDGE_BUFFER;

int GRID_W = GLOBAL_W * 0.75, DISP_W = GLOBAL_W - (GLOBAL_W * 0.10); // +  150;
float DX = (float) GRID_W / NCOLS, DY = (float) GLOBAL_H / NROWS;

const vec3 WHITE(1, 1, 1), BLACK(0, 0, 0), GREY(.5, .5, .5), RED(1, 0, 0),
        GREEN(0, 1, 0), BLUE(0, 0, 1), YELLOW(1, 1, 0),
        ORANGE(1, .55f, 0), PURPLE(.8f, .1f, .5f), CYAN(0, 1, 1), PALE_GREY(.8, .8, .8);

enum NodeStates {
    OPEN, CLOSED_ROAD, CLOSED_HOUSE, CLOSED_FACTORY, DEBUG_OUT_BOUNDS, POTENTIAL_ROAD
};

enum DebugColorIndex
{
    BLUE_COLOR_I,
    ORANGE_COLOR_I,
    PURPLE_COLOR_I,
    RED_COLOR_I,
    GREEN_COLOR_1
};


struct InfoPanel {
    string mouseSpaceDisp;
    string mouseSpaceMoveDisp;
    string gridWinDim;
    string infoWinDim;
    string appWinDisp;
    string gridPrimitiveDim;
    string errorMsg;
    string logsMsg;
    double fpsDisp = 0;
    bool isPaused = false;
    string timeDisplay;
    string status;
    string numRoads;


    void InfoDisplay() {
        Text(DISP_W, GLOBAL_H, WHITE, 10.0f, mouseSpaceDisp.c_str());
        Text(DISP_W, GLOBAL_H - 20, ORANGE, 10.0f, mouseSpaceMoveDisp.c_str());

        Text(DISP_W, GLOBAL_H - 40, WHITE, 10.0f, gridWinDim.c_str());
        Text(DISP_W, GLOBAL_H - 60, WHITE, 10.0f, infoWinDim.c_str());
        Text(DISP_W, GLOBAL_H - 80, WHITE, 10.0f, appWinDisp.c_str());

        Text(DISP_W, GLOBAL_H - 100, GREEN, 10.0f, gridPrimitiveDim.c_str());

        Text(DISP_W, GLOBAL_H - 120, PURPLE, 10.0f, errorMsg.c_str());

        Text(DISP_W, GLOBAL_H - 140, YELLOW, 10.0f, logsMsg.c_str());

        Text(DISP_W, GLOBAL_H - 160, WHITE, 10.0f, to_string(fpsDisp).c_str());

        Text(DISP_W, GLOBAL_H - 180, WHITE, 12.0f, timeDisplay.c_str());

        Text(DISP_W, GLOBAL_H - 200, WHITE, 12.0f, status.c_str());

        Text(DISP_W, GLOBAL_H - 220, WHITE, 10.0f, numRoads.c_str());
    }

    void togglePause() {
        isPaused = !isPaused;
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

    vec4 PosWindowProj()
    {
        return vec4((int) (X_POS + DX * (float) col), (int) (Y_POS + DY * (float) row),
                    (int) DX - 1,   (int) DY - 1);
    }

    bool AlignmentPosMatches(int r, int c)
    {
        if(row == r && col == c)
        {
            return true;
        }
        return false;
    }
    bool AlignmentPosMatches(NodePosition comparePos)
    {
        return AlignmentPosMatches(comparePos.row, comparePos.col);
    }

    bool operator==(NodePosition &i) { return i.row == row && i.col == col; }
};


vec3 DebugGetColor(DebugColorIndex colorIndex)
{
    if(colorIndex == BLUE_COLOR_I)
    {
        return BLUE;
    }
    if(colorIndex == ORANGE_COLOR_I)
    {
        return ORANGE;
    }
    if(colorIndex == PURPLE_COLOR_I)
    {
        return PURPLE;
    }
    if(colorIndex == RED_COLOR_I)
    {
        return RED;
    }
    return GREEN;
}

vec3 GetRandomColor()
{
    int randomIndex = rand() % 5 + 0;
    return DebugGetColor((DebugColorIndex)randomIndex);
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

    Node() {}

    Node(int row, int col) {
        currentPos.row = row;
        currentPos.col = col;
    }
};

struct Vehicle {
    float speed = -.5, t = 1;
    vec3 overlayColor;
    vector<NodePosition> runnerPath;

    Vehicle() { }
    Vehicle(float s, float tt)
    {
        this->speed = s;
        this->t = tt;
    }
    Vehicle(float s, float tt , const vec3& color, const vector<NodePosition>& path)
    {
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

        if(!runnerPath.empty())
        {
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

struct RoadRunnerLinker
{
    size_t hashedId;
    bool isLinked = false;

    // vec3 roadColor;
    Vehicle vehicleRunner;
    // vector<NodePosition> roadPath;

    RoadRunnerLinker(size_t hashedVal,  const Vehicle& runner, bool linkStatus)
    {
        this->hashedId = hashedVal;
        this->vehicleRunner = runner;
        this->isLinked = linkStatus;
    }
};


#endif //ROADREALM_ROADNETSHARED_H
