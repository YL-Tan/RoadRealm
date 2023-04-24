//
// Created by EdwinPc on 4/24/2023.
//

#ifndef ROADREALM_ROADNETSHARED_H
#define ROADREALM_ROADNETSHARED_H

#include <iostream>
#include <vector>
#include "Draw.h"
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
        GREEN(0, 1, 0), BLUE(0, 0, 1), YELLOW(1, 1, 0), ORANGE(1, .55f, 0), PURPLE(.8f, .1f, .5f), CYAN(0, 1, 1);

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

#endif //ROADREALM_ROADNETSHARED_H
