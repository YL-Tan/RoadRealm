// RoadNetShared.h
// Team 8

#ifndef ROADREALM_ROADNETSHARED_H
#define ROADREALM_ROADNETSHARED_H

#include <iostream>
#include <vector>
#include "Draw.h"
#include <string>
#include "GLXtras.h"

using namespace std;

#define NROWS 20
#define NCOLS 20
#define H_EDGE_BUFFER 40
#define W_EDGE_BUFFER 200

int APP_WIDTH = 1000, APP_HEIGHT = 800, X_POS = 20, Y_POS = 20,
        GLOBAL_W = APP_WIDTH - W_EDGE_BUFFER, GLOBAL_H = APP_HEIGHT - H_EDGE_BUFFER;

int GRID_W = GLOBAL_W * 0.75, DISP_W = GLOBAL_W - (GLOBAL_W * 0.10); // +  150;
float DX = (float) GRID_W / NCOLS, DY = (float) GLOBAL_H / NROWS;

const vec3 WHITE(1, 1, 1), BLACK(0, 0, 0), GREY(.5, .5, .5), RED(1, 0, 0),
        GREEN(0, 1, 0), BLUE(0, 0, 1), YELLOW(1, 1, 0),
        ORANGE(1, .55f, 0), PURPLE(.8f, .1f, .5f), CYAN(0, 1, 1);

float pathLength;

struct InfoPanel {
    string mouseSpaceDisp;
    string mouseSpaceMoveDisp;
    string gridWinDim;
    string infoWinDim;
    string appWinDisp;
    string gridPrimitiveDim;
    string errorMsg;
    double fpsDisp = 0;
    int timeSinceStart = 0;

    void InfoDisplay() {
        Text(DISP_W, GLOBAL_H, WHITE, 10.0f, mouseSpaceDisp.c_str());
        Text(DISP_W, GLOBAL_H - 20, ORANGE, 10.0f, mouseSpaceMoveDisp.c_str());

        Text(DISP_W, GLOBAL_H - 40, WHITE, 10.0f, gridWinDim.c_str());
        Text(DISP_W, GLOBAL_H - 60, WHITE, 10.0f, infoWinDim.c_str());
        Text(DISP_W, GLOBAL_H - 80, WHITE, 10.0f, appWinDisp.c_str());

        Text(DISP_W, GLOBAL_H - 100, GREEN, 10.0f, gridPrimitiveDim.c_str());

        Text(DISP_W, GLOBAL_H - 120, PURPLE, 10.0f, errorMsg.c_str());
        Text(DISP_W, GLOBAL_H - 140, WHITE, 10.0f, to_string(fpsDisp).c_str());

        Text(DISP_W, GLOBAL_H - 160, WHITE, 10.0f, to_string(timeSinceStart).c_str());
    }
};

class Grid {
public:
    int row = -1, col = -1;

    Grid() {};

    Grid(int r, int c) : row(r), col(c) {};

    float DistanceTo(Grid i) {
        float drow = (float) (i.row - row), dcol = (float) (i.col - col);
        return sqrt(drow * drow + dcol * dcol);
    }

    bool Valid() { return row >= 0 && row < NROWS && col >= 0 && col < NCOLS; }

    vec2 Point() { return vec2(X_POS + (col + .5) * DX, Y_POS + (row + .5) * DY); }

    bool operator==(Grid &i) { return i.row == row && i.col == col; }
};

vector<Grid> path;

void DrawVertex(float row, float col) {
    Disk(vec2(X_POS + (col + .5) * DX, Y_POS + (row + .5) * DY), 6, BLUE);
}

void DrawSegment(Grid i1, Grid i2, float width, vec3 col) {
    Line(i1.Point(), i2.Point(), width, col, col);
}

void DrawPath(vector<Grid> &path, float width, vec3 color) {
    for (size_t i = 1; i < path.size(); i++)
        DrawSegment(path[i - 1], path[i], width, color);
}

void DrawRectangle(int x, int y, int w, int h, vec3 col) {
    Quad(x, y, x, y + h, x + w, y + h, x + w, y, true, col);
}

float PathLength() {
    float len = 0;
    for (size_t i = 1; i < path.size(); i++)
        len += path[i].DistanceTo(path[i - 1]);
    return len;
}

vec2 PointOnPath(float dist) {
    float accumDist = 0;
    Grid i1 = path[0];
    for (size_t i = 1; i < path.size(); i++) {
        Grid i2 = path[i];
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

class Bot {
    // represents a moving agent on the grid. It has methods for updating the agent's position
    // along the computed path and drawing the agent on the screen.
public:
    float speed = -.5, t = 1;
    int idBot = 0;
    Bot() { }
    Bot(float s, float tt, int id) : t(tt), speed(s), idBot(id) { }
    void Update(float dt) {
        t += dt * speed;
        if (t < 0 || t > 1) speed = -speed;
        t = t < 0 ? 0 : t > 1 ? 1 : t;
    }
    void Draw(vec3 color) {
        vec2 p = PointOnPath(t * pathLength);
        Disk(vec2(X_POS + (p.x + .5) * DX, Y_POS + (p.y + .5) * DY), 20, color);
    }
};
#endif //ROADREALM_ROADNETSHARED_H
