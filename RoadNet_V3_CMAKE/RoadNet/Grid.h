// Grid.h
// Team 8

#ifndef ROADREALM_ASTAR_H
#define ROADREALM_ASTAR_H

#include "RoadNetShared.h"

struct DestinationObjectives {
    Node houseNode;
    Node factoryNode;
    bool destLinked = false;
};

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

    bool IsAClosedBuilding(const Node &node) {
        if (node.currentState == CLOSED_FACTORY || node.currentState == CLOSED_HOUSE) {
            return true;
        }
        return false;
    }

    bool IsSimilarNodePos(NodePosition nodePosComp, NodePosition nodePosComp1) {
        return nodePosComp.AlignmentPosMatches(nodePosComp1);
    }

public:
    // N * N grid's nodes/cells
    vector<Node> gridNodes = {};

    vector<DestinationObjectives> gridDestObjectives;

    GridPrimitive() {
        FormulateGrid();
    }

    GridPrimitive(const vec3 &colorInput) {
        FormulateGrid();
        nodesDefaultColor = colorInput;
    }

    bool UpdateDestinationLink(NodePosition homePos, NodePosition factoryPos, bool isLinked) {
        for (DestinationObjectives &objectives: gridDestObjectives) {
            if (IsSimilarNodePos(homePos, objectives.houseNode.currentPos) &&
                IsSimilarNodePos(factoryPos, objectives.factoryNode.currentPos) && objectives.destLinked != isLinked) {
                // Update Performed.
                objectives.destLinked = isLinked;
                return true;
            }
        }
        // No Updates Performed. (Combined: No record of home-to-factory being an objective destination)
        return false;
    }

    bool AddNewObjective(int startR, int startC, int endR, int endC) {
        int startNIndex = CombineDigits(startR, startC);
        int endNIndex = CombineDigits(endR, endC);

        if (startNIndex < gridNodes.size() && endNIndex < gridNodes.size()) {
            if (!IsAClosedBuilding(gridNodes.at(startNIndex)) && !IsAClosedBuilding(gridNodes.at(endNIndex))) {
                // House = Start, Factory = End
                gridNodes.at(startNIndex).currentState = CLOSED_HOUSE;
                gridNodes.at(endNIndex).currentState = CLOSED_FACTORY;

                vec3 shrRandColor = GetRandomColor();

                gridNodes.at(startNIndex).overlayColor = shrRandColor;
                gridNodes.at(endNIndex).overlayColor = shrRandColor;

                // Add To Objectives
                gridDestObjectives.push_back({gridNodes.at(startNIndex), gridNodes.at(endNIndex)});
                return true;
            }
        }
        return false;
    }

    void DrawGrid() {
        // Draw All Cells
        for (Node cell: gridNodes) {

            // Draw Grid
            DrawRectangle(cell.currentPos.PosWindowProj().x, cell.currentPos.PosWindowProj().y,
                          cell.currentPos.PosWindowProj().z, cell.currentPos.PosWindowProj().w,
                          cell.color);

            if (cell.currentState == CLOSED_HOUSE) {
                Disk(vec2(X_POS + (cell.currentPos.col + .5) * DX, Y_POS + (cell.currentPos.row + .5) * DY),
                     25, cell.overlayColor);
            }
            if (cell.currentState == CLOSED_FACTORY) {
                Disk(vec2(X_POS + (cell.currentPos.col + .5) * DX, Y_POS + (cell.currentPos.row + .5) * DY),
                     25, cell.overlayColor);

                // Horizontal Line
                Line(vec2(X_POS + (cell.currentPos.col + 0.0) * DX, Y_POS + (cell.currentPos.row + 0.5) * DY),
                     vec2(X_POS + (cell.currentPos.col + 1.0) * DX, Y_POS + (cell.currentPos.row + 0.5) * DY),
                     1.0f, cell.overlayColor);

                // Vertical Line
                Line(vec2(X_POS + (cell.currentPos.col + 0.5) * DX, Y_POS + (cell.currentPos.row + 1.0) * DY),
                     vec2(X_POS + (cell.currentPos.col + 0.5) * DX, Y_POS + (cell.currentPos.row + 0.0) * DY),
                     1.0f, cell.overlayColor);
            }
        }
    }

    Node NodeHandler(int nodeIndex) {
        Node *node = &gridNodes.at(nodeIndex);
        if (node->currentState == OPEN) {
            // Node State Changes, Swap/Toggled
            node->currentState = node->transState;
            node->transState = CLOSED_ROAD;
            node->color = ORANGE;
            return *node;
        }
        if (node->currentState == CLOSED_ROAD) {
            // Node State Changes, Swap/Toggled
            node->currentState = node->transState;
            node->transState = OPEN;
            node->color = ORANGE;
            return *node;
        }
        if (node->currentState == POTENTIAL_ROAD) {
            node->currentState = node->transState;
            node->transState = POTENTIAL_ROAD;

            if (node->currentState == CLOSED_ROAD) {
                node->color = GREY;
            }
            if (node->currentState == OPEN) {
                node->color = WHITE;
            }
            return *node;
        }
        return *node;
    }

    Node GetNode(const vec2 &gridAxisPoint) {
        int nodeIndex = CombineDigits((int) gridAxisPoint.y, (int) gridAxisPoint.x);
        if (nodeIndex < gridNodes.size()) {
            return gridNodes.at(nodeIndex);
        }
        return {};
    }

    void GridReset() {
        this->gridDestObjectives.clear();
        for (Node &cell: gridNodes) {
            cell.NodeReset();
        }
    }
};

#endif //ROADREALM_ASTAR_H
