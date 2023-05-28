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
    bool revertState = false;

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
    bool NodeStatesHandler(Node *node)
    {
        if (node->currentState == OPEN && GLOBAL_GAMEPLAY_STATE == DRAW_STATE) {
            // Node State Changes, Swap/Toggled
            node->currentState = node->transState;
            //node->prevState = OPEN;
            node->transState = CLOSED_ROAD;
            node->color = ORANGE;
            return true;
        }
        if (node->currentState == CLOSED_ROAD && GLOBAL_GAMEPLAY_STATE == WIPE_STATE) {
            // Node State Changes, Swap/Toggled
            node->currentState = node->transState;
            //node->prevState = CLOSED_ROAD;
            node->transState = OPEN;
            node->color = ORANGE;
            return true;
        }
        if (node->currentState == POTENTIAL_ROAD) {
            // Move to TransState
            node->currentState = node->transState;

            if (this->revertState) {
                // Move to PrevState
                node->currentState = node->prevState;
            }

            node->transState = POTENTIAL_ROAD;

            if (node->currentState == CLOSED_ROAD) {
                node->color = GREY;
                node->prevState = CLOSED_ROAD;
            }
            if (node->currentState == OPEN) {
                node->color = WHITE;
                node->prevState = OPEN;
            }
            return true;
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

    bool NeighborIsOccupied(vec2 point){
        vec2 adjacentBottom = vec2(point.x-1, point.y);
        vec2 adjacentTop= vec2(point.x+1, point.y);
        vec2 adjacentLeft= vec2(point.x, point.y - 1);
        vec2 adjacentRight= vec2(point.x, point.y + 1);

        vec2 adjacentBottomLeft= vec2(point.x-1, point.y-1);
        vec2 adjacentTopLeft= vec2(point.x+1, point.y-1);
        vec2 adjacentBottomRight= vec2(point.x-1, point.y+1);
        vec2 adjacentTopRight= vec2(point.x+1, point.y+1);


        vector<vec2> adjacentNodes = {adjacentBottom, adjacentTop, adjacentLeft, adjacentRight, adjacentBottomLeft, adjacentTopLeft, adjacentBottomRight, adjacentTopRight};

        for (auto node: adjacentNodes){
            if (IsAClosedNodeState(node)){
                return true;
            }
        }
        return false;
    }

    bool AddNewObjective(int startR, int startC, int endR, int endC) {

        if (!DoesDestinationExists(vec2(startR, startC), vec2(endR, endC))) {
            int startNIndex = CombineDigits(startR, startC);
            int endNIndex = CombineDigits(endR, endC);
            vec2 startPoint = vec2(startC, startR);
            vec2 endPoint = vec2(endC, endR);

            bool isSamePoint = IsSamePosition(startPoint, endPoint);

            if (startNIndex < gridNodes.size() && endNIndex < gridNodes.size() && !NeighborIsOccupied(startPoint) && !NeighborIsOccupied(endPoint)) {
                if (!IsAClosedNodeState(gridNodes.at(startNIndex), true) && !IsAClosedNodeState(gridNodes.at(endNIndex),
                                                                                                true) && !isSamePoint) {
                    // House = Start, Factory = End
                    gridNodes.at(startNIndex).currentState = CLOSED_HOUSE;
                    gridNodes.at(endNIndex).currentState = CLOSED_FACTORY;

                    gridNodes.at(startNIndex).color = WHITE;
                    gridNodes.at(endNIndex).color = WHITE;

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
        if(this->revertState && !IsAClosedNodeState(*node, true))
        {
            cout << "\tCR-State: " << PrintNodeState(node->currentState) << "\tPR-State" << PrintNodeState(node->prevState) << "\tTR-State" <<  PrintNodeState(node->transState) << "\n";

            node->currentState = POTENTIAL_ROAD;
        }
        NodeStatesHandler(node);
        return *node;
    }

    Node GetNode(const vec2 &gridAxisPoint) {
        int nodeIndex = CombineDigits((int) gridAxisPoint.y, (int) gridAxisPoint.x);
        if (nodeIndex < gridNodes.size()) {
            return gridNodes.at(nodeIndex);
        }
        return {};
    }

    bool IsAClosedNodeState(vec2 &gridAxisPoint, bool favorClRoad = false) {
        Node node = GetNode(gridAxisPoint);
        if (node.currentState == CLOSED_FACTORY || node.currentState == CLOSED_HOUSE ||
            (favorClRoad && node.currentState == CLOSED_ROAD)) {
            return true;
        }
        return false;
    }

    bool IsAClosedNodeState(const Node &node, bool favorClRoad = false) {
        if (node.currentState == CLOSED_FACTORY || node.currentState == CLOSED_HOUSE ||
            (favorClRoad && node.currentState == CLOSED_ROAD)) {
            return true;
        }
        return false;
    }

    void ResetNodes(vector<vec2> gridAxisPoints, bool enableReset) {
        this->revertState = enableReset;
        for (vec2 &pt: gridAxisPoints) {
            int mapToIndex = CombineDigits((int) pt.y, (int) pt.x);
            NodeHandler(mapToIndex);
        }
        this->revertState = false;
    }

    void GridReset() {
        this->gridDestObjectives.clear();
        for (Node &cell: gridNodes) {
            cell.NodeReset();
        }
    }

    bool IsAllDestinationLinked() {
        for (DestinationObjectives &objectives: gridDestObjectives) {
            if (objectives.destLinked == false) {
                return false;
            }
        }
        return true;
    }

    int GridClearAndCountRoads() {
        int counter = 0;
        for (Node& cell : gridNodes) {
            if (cell.currentState == CLOSED_ROAD) {
                counter++;
            }
            if (!IsAClosedNodeState(cell)) {
                cell.NodeReset();
            }
            cell.isConnected = false;
        }
        for (DestinationObjectives& objectives : gridDestObjectives) {
            if (objectives.destLinked) {
                objectives.destLinked = false;
            }
        }
        return counter;
    }
};

#endif //ROADREALM_ASTAR_H
