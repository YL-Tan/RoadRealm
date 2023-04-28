// AStar.h
// Team 8

#ifndef ROADREALM_ASTAR_H
#define ROADREALM_ASTAR_H

#include "RoadNetShared.h"

struct Node {
    NodePosition currentPos;
    // Distance Cost f(n) = g(n) + h(n)
    float g = 0, h = 0, f = 0;
    // State of the Cell/Node
    bool roadPlaced = false, open = false, closed = false;
    // vec3 color = WHITE;
};

struct DefinedObjectives
{
    NodePosition start;
    NodePosition end;
    vec3 color;
};

// Grid start, goal; // Collection, vector, map, set
vector<DefinedObjectives> objectives;

class AStar {
private:
    bool LowestIndex(NodePosition &lowest) {
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

    // SOLID PRINCIPLES
    bool ComputePath(NodePosition start, NodePosition goal) {
        // initialize
        // start = s;
        // goal = g;
        Node& n = nodes[start.row][start.col];
        n.g = 0;
        n.h = start.DistanceTo(goal);
        n.f = n.h + n.g;
        n.open = true;
        // loop
        NodePosition xIndex;
        while (LowestIndex(xIndex)) {
            if (xIndex.col == goal.col && xIndex.row == goal.row)
                return true;
            Node& x = nodes[xIndex.row][xIndex.col];
            // remove x from openSet, add to closedSet
            x.open = false;
            x.closed = true;
            // for each y in neighbor_nodes(x)
            for (int dir = 0; dir < 4; dir++) {
                int row = xIndex.row;
                int col = xIndex.col;

                if (dir == 0) { // Up
                    row -= 1;
                }
                else if (dir == 1) { // Down
                    row += 1;
                }
                else if (dir == 2) { // Left
                    col -= 1;
                }
                else if (dir == 3) { // Right
                    col += 1;
                }

                NodePosition yIndex(row, col);

                // test all four neighbors, but not self
                if (yIndex == xIndex || !yIndex.Valid())
                    continue;
                // skip closed nodes
                Node& y = nodes[row][col];
                if (y.closed || !y.roadPlaced)  // changed the logic
                    continue;
                // tentative_g_score := g_score[x] + dist_between(x,y)
                bool tentativeIsBetter;
                float tentativeG = x.g + xIndex.DistanceTo(yIndex);
                if (!y.open) {
                    y.open = true;
                    y.h = yIndex.DistanceTo(goal);
                    tentativeIsBetter = true;
                }
                else
                    tentativeIsBetter = tentativeG < y.g;

                if (tentativeIsBetter) {
                    y.currentPos = xIndex;
                    y.g = tentativeG;
                    y.f = y.g + y.h;
                }
            } // end for dir loop

        } // end while(LowestIndex)
        return false;
    } // end ComputePath

    int ReconstructPath(NodePosition nodeEnd) {
        path.resize(0);
        path.push_back(nodeEnd);
        Node &n = nodes[nodeEnd.row][nodeEnd.col];

        while (n.currentPos.row >= 0) {
            path.push_back(n.currentPos);
            n = nodes[n.currentPos.row][n.currentPos.col];
        }
        return path.size();
    }

public:

    Node nodes[NROWS][NCOLS];

    void MainReconstruct()
    {
        for(const DefinedObjectives& definedObjectives : objectives)
        {
            ReconstructPath(definedObjectives.end);
        }
    }

    void MainCompute()
    {
        for(const DefinedObjectives& definedObjectives : objectives)
        {
            ComputePath(definedObjectives.start, definedObjectives.end);
        }
    }
    /*void DrawPaths(float width, vec3 color) {
        for (int row = 0; row < NROWS; row++)
            for (int col = 0; col < NCOLS; col++) {
                Node &n = nodes[row][col];
                if (n.from.row > -1)
                    DrawSegment(Grid(row, col), n.from, width, color);
            }
    }*/

    void Draw() {
        DrawRectangle(X_POS, Y_POS, GRID_W, GLOBAL_H, BLACK);
        for (int row = 0; row < NROWS; row++)
            for (int col = 0; col < NCOLS; col++) {
                Node n = nodes[row][col];
                vec3 color = n.open ? GREEN : n.closed ? GREY : n.roadPlaced ? GREY : WHITE;
                DrawRectangle((int) (X_POS + DX * (float) col), (int) (Y_POS + DY * (float) row),
                              (int) DX - 1, (int) DY - 1, color);
                /*DrawRectangle(n.currentPos.Point4().x, n.currentPos.Point4().y,
                              n.currentPos.Point4().z, n.currentPos.Point4().w, color);*/
            }

        DrawCircle();
    }
    void DrawCircle()
    {
        for(const DefinedObjectives& definedObjectives : objectives)
        {
            glPointSize(20);
            if (definedObjectives.start.row > -1)
                Disk(vec2(X_POS+(definedObjectives.start.col+.5)*DX, Y_POS+(definedObjectives.start.row+.5)*DY), 20,
                     definedObjectives.color);
            if (definedObjectives.end.row > -1)
                Disk(vec2(X_POS+(definedObjectives.end.col+.5)*DX, Y_POS+(definedObjectives.end.row+.5)*DY), 20,
                     definedObjectives.color);
        }
    }
};


#endif //ROADREALM_ASTAR_H
