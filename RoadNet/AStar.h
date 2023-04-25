// AStar.h
// Team 8

#ifndef ROADREALM_ASTAR_H
#define ROADREALM_ASTAR_H

#include "RoadNetShared.h"

struct Node {
    Grid from;
    float g = 0, h = 0, f = 0;
    bool roadPlaced = false, open = false, closed = false;
};


class AStar {
private:
    Grid start, goal;

    bool LowestIndex(Grid &lowest) {
        bool found = false;
        float low = FLT_MAX;
        for (int row = 0; row < NROWS; row++)
            for (int col = 0; col < NCOLS; col++) {
                Node &n = nodes[row][col];
                if (n.open && n.f < low) {
                    low = n.f;
                    lowest.row = row;
                    lowest.col = col;
                    found = true;
                }
            }
        return found;
    }

public:
    Node nodes[NROWS][NCOLS];

    int ReconstructPath(Grid gridStart, vector<Grid> &path) {
        path.resize(0);
        path.push_back(gridStart);
        Node &n = nodes[gridStart.row][gridStart.col];
        while (n.from.row >= 0) {
            path.push_back(n.from);
            n = nodes[n.from.row][n.from.col];
        }
        return path.size();
    }

    bool ComputePath(Grid s, Grid g) {
        // initialize
        start = s;
        goal = g;
        Node &n = nodes[start.row][start.col];
        n.g = 0;
        n.h = start.DistanceTo(goal);
        n.f = n.h + n.g;
        n.open = true;
        // loop
        Grid xIndex;
        while (LowestIndex(xIndex)) {
            if (xIndex.col == goal.col && xIndex.row == goal.row)
                return true;
            Node &x = nodes[xIndex.row][xIndex.col];
            // remove x from openSet, add to closedSet
            x.open = false;
            x.closed = true;
            // for each y in neighbor_nodes(x)
            for (int col = xIndex.col - 1; col <= xIndex.col + 1; col++)
                for (int row = xIndex.row - 1; row <= xIndex.row + 1; row++) {
                    Grid yIndex(row, col);
                    // test all eight neighbors, but not self
                    if (yIndex == xIndex || !yIndex.Valid())
                        continue;
                    // skip closed nodes
                    Node &y = nodes[row][col];
                    if (y.closed || y.roadPlaced)
                        continue;
                    // tentative_g_score := g_score[x] + dist_between(x,y)
                    bool tentativeIsBetter;
                    float tentativeG = x.g + xIndex.DistanceTo(yIndex);
                    if (!y.open) {
                        y.open = true;
                        y.h = yIndex.DistanceTo(goal);
                        tentativeIsBetter = true;
                    } else
                        tentativeIsBetter = tentativeG < y.g;

                    if (tentativeIsBetter) {
                        y.from = xIndex;
                        y.g = tentativeG;
                        y.f = y.g + y.h;
                    }
                } // end for row loop

        } // end while(LowestIndex)
        return false;
    } // end ComputePath
    void DrawPaths(float width, vec3 color) {
        for (int row = 0; row < NROWS; row++)
            for (int col = 0; col < NCOLS; col++) {
                Node &n = nodes[row][col];
                if (n.from.row > -1)
                    DrawSegment(Grid(row, col), n.from, width, color);
            }
    }

    void Draw() {
        DrawRectangle(X_POS, Y_POS, GRID_W, GLOBAL_H, BLACK);
        for (int row = 0; row < NROWS; row++)
            for (int col = 0; col < NCOLS; col++) {
                Node n = nodes[row][col];
                vec3 color = n.open ? GREEN : n.closed ? RED : n.roadPlaced ? GREY : WHITE;
                DrawRectangle((int) (X_POS + DX * (float) col), (int) (Y_POS + DY * (float) row),
                              (int) DX - 1, (int) DY - 1, color);
            }
        /* glPointSize(20);
         if (start.row > -1)
             Disk(vec2(X_POS+(start.col+.5)*DX, Y_POS+(start.row+.5)*DY), 20, RED);
         if (goal.row > -1)
             Disk(vec2(X_POS+(goal.col+.5)*DX, Y_POS+(goal.row+.5)*DY), 20, BLUE);*/
    }
};


#endif //ROADREALM_ASTAR_H
