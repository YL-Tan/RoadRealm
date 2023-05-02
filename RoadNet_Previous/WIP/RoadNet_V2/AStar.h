// AStar.h
// Team 8

#ifndef ROADREALM_ASTAR_H
#define ROADREALM_ASTAR_H

#include "RoadNetShared.h"

struct Node {
    NodePosition currentPos;

    vec3 color = WHITE;
    NodeStates currentState = OPEN;
    // bool isOpen = true;

    Node() {}

    Node(int row, int col) {
        currentPos.row = row;
        currentPos.col = col;
    }

};

// Grid start, goal; // Collection, vector, map, set
vector<DefinedObjectives> objectives;

class GridPrimitive {
private:
    vec3 nodesDefaultColor = WHITE;

    void FormulateGrid() {
        for (int row = 0; row < NROWS; row++) {
            for (int col = 0; col < NCOLS; col++) {
                Node cellNode(row, col);
                cellNode.color = nodesDefaultColor;

                // Add To Collection/Vector
                gridNodes.push_back(cellNode);
            }
        }
    }

public:
    // N * N grid's nodes/cells
    vector<Node> gridNodes = {};
    //vector<NodePosition> gridNodeP

    GridPrimitive() {
        FormulateGrid();
    }

    GridPrimitive(const vec3 &colorInput) {
        FormulateGrid();
        nodesDefaultColor = colorInput;
    }

    void DrawGrid() {
        // Draw All Cells
        for (Node cell: gridNodes) {
            DrawRectangle(cell.currentPos.PosWindowProj().x, cell.currentPos.PosWindowProj().y,
                          cell.currentPos.PosWindowProj().z, cell.currentPos.PosWindowProj().w,
                          cell.color);
        }

    }

    bool NodeHandler(int nodeIndex) {
        Node *node = &gridNodes.at(nodeIndex);
        if(node->currentState == OPEN)
        {
            // Node State Changes, Swap/Toggled
            node->currentState = CLOSED_ROAD;
            node->color = GREY;
            return true;
        }
         if (node->currentState == CLOSED_ROAD) {
             // Node State Changes, Swap/Toggled
             node->currentState = OPEN;
             node->color = WHITE;
             return true;
         }
        return false;
    }

};

#endif //ROADREALM_ASTAR_H
