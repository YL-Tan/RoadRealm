// Grid.h
// Team 8

#ifndef ROADREALM_ASTAR_H
#define ROADREALM_ASTAR_H

#include "RoadNetShared.h"

struct DestinationObjectives {
    Node &houseNode;
    Node &factoryNode;
    bool destLinked = false;
};

class GridPrimitive {
private:
    vec3 nodesDefaultColor = WHITE;
    vector<DestinationObjectives> gridDestObjectives;

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

    bool IsSimilarNodePos(NodePosition nodePosComp, NodePosition nodePosComp1) {
        return nodePosComp.AlignmentPosMatches(nodePosComp1);
    }

    bool DoesDestinationExists(vec2 potHouse, vec2 potFactory) {
        for (DestinationObjectives &objectives: gridDestObjectives) {

            bool isHouseDuplicate = objectives.houseNode.currentPos.AlignmentPosMatches(potHouse.y, potHouse.x);
            bool isFactDuplicate = objectives.factoryNode.currentPos.AlignmentPosMatches(potFactory.y, potFactory.x);

            if (isHouseDuplicate || isFactDuplicate) {
                return true;
            }
        }
        return false;
    }

public:
    // N * N grid's nodes/cells
    vector<Node> gridNodes = {};

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

                objectives.houseNode.isConnected = isLinked;
                objectives.factoryNode.isConnected = isLinked;
                return true;
            }
        }
        // No Updates Performed. (Combined: No record of home-to-factory being an objective destination)
        return false;
    }

    bool AddNewObjective(int startR, int startC, int endR, int endC) {

        if (!DoesDestinationExists(vec2(startR, startC), vec2(endR, endC))) {
            int startNIndex = CombineDigits(startR, startC);
            int endNIndex = CombineDigits(endR, endC);

            if (startNIndex < gridNodes.size() && endNIndex < gridNodes.size()) {
                if (!IsAClosedNodeState(gridNodes.at(startNIndex)) && !IsAClosedNodeState(gridNodes.at(endNIndex))) {
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
        }
        return false;
    }

    void DrawGrid() {
        // Draw All Cells
        for (Node &cell: gridNodes) {

            // Draw Grid
            DrawRectangle(cell.currentPos.PosWindowProj().x, cell.currentPos.PosWindowProj().y,
                          cell.currentPos.PosWindowProj().z, cell.currentPos.PosWindowProj().w,
                          cell.color);

            if (cell.currentState == CLOSED_HOUSE) {
                Disk(vec2(X_POS + (cell.currentPos.col + .5) * DX, Y_POS + (cell.currentPos.row + .5) * DY),
                     GetCirDiam(!cell.isConnected), cell.overlayColor);
            }
            if (cell.currentState == CLOSED_FACTORY) {
                Disk(vec2(X_POS + (cell.currentPos.col + .5) * DX, Y_POS + (cell.currentPos.row + .5) * DY),
                     GetCirDiam(!cell.isConnected), cell.overlayColor);

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

    void GridUpdate() {
        // GetMin(Width, Height) -> The Smallest Length
        MAX_DIAMETER_SIZE = GetMin<float>((int) DX - 1, (int) DY - 1) * MAX_CIR_EXPANSION;
    }

    Node NodeHandler(int nodeIndex) {
        Node *node = &gridNodes.at(nodeIndex);
        if (node->currentState == OPEN && globalState == DRAW_STATE) {
            // Node State Changes, Swap/Toggled
            node->currentState = node->transState;
            node->transState = CLOSED_ROAD;
            node->color = ORANGE;
            return *node;
        }
        if (node->currentState == CLOSED_ROAD && globalState == WIPE_STATE) {
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

    bool IsAClosedNodeState(vec2 &gridAxisPoint, bool favorClRoad=false) {
        Node node = GetNode(gridAxisPoint);
        if (node.currentState == CLOSED_FACTORY || node.currentState == CLOSED_HOUSE || (favorClRoad && node.currentState == CLOSED_ROAD)) {
            return true;
        }
        return false;
    }

    bool IsAClosedNodeState(const Node &node, bool favorClRoad=false) {
        if (node.currentState == CLOSED_FACTORY || node.currentState == CLOSED_HOUSE || (favorClRoad && node.currentState == CLOSED_ROAD)) {
            return true;
        }
        return false;
    }

    void ResetNodes(vector<vec2> gridAxisPoints)
    {
        for(vec2 &pt : gridAxisPoints)
        {
            int mapToIndex = CombineDigits((int)pt.y, (int)pt.x);

            if(!IsAClosedNodeState(pt, true) && mapToIndex < gridNodes.size())
            {
                gridNodes.at(mapToIndex).NodeReset();
            }
        }
    }

    void GridReset() {
        this->gridDestObjectives.clear();
        for (Node &cell: gridNodes) {
            cell.NodeReset();
        }
    }

        bool IsAllDestinationLinked() {
        for (DestinationObjectives& objectives : gridDestObjectives) {
            if (objectives.destLinked == false) {
                return false;
            }
        }
        return true;
    }
};

#endif //ROADREALM_ASTAR_H
