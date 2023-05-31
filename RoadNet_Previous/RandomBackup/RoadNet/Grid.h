/**
 * @file Grid.h
 * @author Team 8: Edwin Kaburu, Vincent Marklynn, Yong Long Tan
 * @date 5/29/2023
 */
#ifndef ROADREALM_GRID_H
#define ROADREALM_GRID_H

#include "RoadNetShared.h"

/**
 * @struct DestinationObjectives
 * @details Data Structure for House and Factory Node along with Linking Status
 */
struct DestinationObjectives {
    Node &houseNode;
    Node &factoryNode;
    bool destLinked = false;
};

/**
 * @class GridPrimitive
 * @details Representation and functionalities for a Grid
 */
class GridPrimitive {
private:
    // Default background Node Color
    vec3 nodesDefaultColor = WHITE;
    // List of Objective Destinations
    vector<DestinationObjectives> gridDestObjectives;
    // Reversion condition
    bool revertState = false;

    /**
     * FormulateGrid() Will formulate the nodes contained within the grid based on NROWS AND NCOLS
     */
    void FormulateGrid() {
        for (int row = 0; row < NROWS; row++) {
            for (int col = 0; col < NCOLS; col++) {
                Node cellNode(row, col);
                cellNode.color = nodesDefaultColor;
                // Add Node to Collection
                gridNodes.push_back(cellNode);
            }
        }
    }

    /**
     * IsSimilarNodePos()  Validate If Position of Two Node are in the same grid's axis spot.
     *
     * @param nodePosComp NodePosition Struct
     * @param nodePosComp1 NodePosition Struct
     * @return Boolean Condition
     */
    bool IsSimilarNodePos(NodePosition nodePosComp, NodePosition nodePosComp1) {
        return nodePosComp.AlignmentPosMatches(nodePosComp1);
    }

    /**
     * DoesDestinationExists() Validate If Vec2 Position On Grid is a Destination Objective
     *
     * @param potHouse Vec2 Struct Position
     * @param potFactory Vec2 Struct Position
     * @return Boolean Condition
     */
    bool DoesDestinationExists(const vec2 &potHouse, const vec2 &potFactory) {
        for (DestinationObjectives &objectives: gridDestObjectives) {
            bool isHouseDuplicate = objectives.houseNode.currentPos.AlignmentPosMatches(potHouse.y, potHouse.x);
            bool isFactDuplicate = objectives.factoryNode.currentPos.AlignmentPosMatches(potFactory.y, potFactory.x);

            if (isHouseDuplicate || isFactDuplicate) {
                return true;
            }
        }
        return false;
    }

    /**
     * NodeStatesHandler() Node State Event Handler
     *
     * @param node Node Struct
     * @return Boolean Condition
     */
    bool NodeStatesHandler(Node *node) {
        if (node->currentState == OPEN && GLOBAL_GAMEPLAY_STATE == DRAW_STATE) {
            node->currentState = node->transState;
            node->transState = CLOSED_ROAD;
            node->color = ORANGE;
            return true;
        }
        if (node->currentState == CLOSED_ROAD && GLOBAL_GAMEPLAY_STATE == WIPE_STATE) {
            node->currentState = node->transState;
            node->transState = OPEN;
            node->color = ORANGE;
            return true;
        }
        if (node->currentState == POTENTIAL_ROAD) {
            node->currentState = node->transState;
            if (this->revertState) {
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

    /**
     * NeighborIsOccupied() Validate If Adjacent Neighbours are Closed States
     *
     * @param point Vec2 Struct Grid Position
     * @return Boolean Condition
     */
    bool NeighborIsOccupied(const vec2 &point) {
        vec2 adjacentBottom = vec2(point.x - 1, point.y);
        vec2 adjacentTop = vec2(point.x + 1, point.y);
        vec2 adjacentLeft = vec2(point.x, point.y - 1);
        vec2 adjacentRight = vec2(point.x, point.y + 1);

        vec2 adjacentBottomLeft = vec2(point.x - 1, point.y - 1);
        vec2 adjacentTopLeft = vec2(point.x + 1, point.y - 1);
        vec2 adjacentBottomRight = vec2(point.x - 1, point.y + 1);
        vec2 adjacentTopRight = vec2(point.x + 1, point.y + 1);

        vector<vec2> adjacentNodes = {adjacentBottom, adjacentTop, adjacentLeft, adjacentRight, adjacentBottomLeft,
                                      adjacentTopLeft, adjacentBottomRight, adjacentTopRight};

        for (auto node: adjacentNodes) {
            if (IsAClosedNodeState(node)) {
                return true;
            }
        }
        return false;
    }

public:
    // List Of Nodes In Grid
    vector<Node> gridNodes = {};

    /**
     * GridPrimitive() Default Constructor For Grid Primitive Instance
     */
    GridPrimitive() {
        FormulateGrid();
    }

    /**
     * GridPrimitive() Default Constructor For Grid Primitive Instance with default color
     *
     * @param colorInput Vec3 Color
     */
    GridPrimitive(const vec3 &colorInput) {
        FormulateGrid();
        nodesDefaultColor = colorInput;
    }

    /**
     * UpdateDestinationLink() Update Linking Status from House To Factory
     *
     * @param homePos House NodePosition
     * @param factoryPos Factory NodePosition
     * @param isLinked Linking Status
     * @return Boolean Condition
     */
    bool UpdateDestinationLink(NodePosition homePos, NodePosition factoryPos, bool isLinked) {
        for (DestinationObjectives &objectives: gridDestObjectives) {
            if (IsSimilarNodePos(homePos, objectives.houseNode.currentPos) &&
                IsSimilarNodePos(factoryPos, objectives.factoryNode.currentPos) && objectives.destLinked != isLinked) {
                objectives.destLinked = isLinked;
                objectives.houseNode.isConnected = isLinked;
                objectives.factoryNode.isConnected = isLinked;
                // Update Performed.
                return true;
            }
        }
        // No Updates Performed. (Combined: No record of home-to-factory being an objective destination)
        return false;
    }

    /**
     * AddNewObjective() Add a new Destination Objective from possible house and factory, row to col axis
     *
     * @param startR Integer S Row
     * @param startC Integer S Col
     * @param endR Integer E Row
     * @param endC Integer E Col
     * @return Boolean Condition
     */
    bool AddNewObjective(int startR, int startC, int endR, int endC) {

        if (!DoesDestinationExists(vec2(startR, startC), vec2(endR, endC))) {
            int startNIndex = CombineDigits(startR, startC);
            int endNIndex = CombineDigits(endR, endC);
            vec2 startPoint = vec2(startC, startR);
            vec2 endPoint = vec2(endC, endR);

            if (startNIndex < gridNodes.size() && endNIndex < gridNodes.size() && !NeighborIsOccupied(startPoint) &&
                !NeighborIsOccupied(endPoint)) {
                if (!IsAClosedNodeState(gridNodes.at(startNIndex), true) && !IsAClosedNodeState(gridNodes.at(endNIndex),
                                                                                                true)) {
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

    /**
     *  DrawGrid() GridPrimitive Draw Function
     */
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

    /**
     * GridUpdate() Grid Update Function
     */
    void GridUpdate() {
        // Update Diameter Max Size
        MAX_DIAMETER_SIZE = GetMin<float>((int) DX - 1, (int) DY - 1) * MAX_CIR_EXPANSION;
    }

    /**
     * NodeHandler() Node Handler
     *
     * @param nodeIndex Node's Collection Index Position
     * @return Node
     */
    Node NodeHandler(int nodeIndex) {
        Node *node = &gridNodes.at(nodeIndex);
        if (this->revertState && !IsAClosedNodeState(*node, true)) {
            cout << "\tCR-State: " << PrintNodeState(node->currentState) << "\tPR-State"
                 << PrintNodeState(node->prevState) << "\tTR-State" << PrintNodeState(node->transState) << "\n";
            node->currentState = POTENTIAL_ROAD;
        }
        NodeStatesHandler(node);
        return *node;
    }

    /**
     * GetNode() Gets A Node Based on its Grid Axis Position
     *
     * @param gridAxisPoint Vec2 Struct Position
     * @return Node
     */
    Node GetNode(const vec2 &gridAxisPoint) {
        int nodeIndex = CombineDigits((int) gridAxisPoint.y, (int) gridAxisPoint.x);
        if (nodeIndex < gridNodes.size()) {
            return gridNodes.at(nodeIndex);
        }
        return {};
    }

    /**
     * IsAClosedNodeState() Validate If Node is a Closed State
     *
     * @param gridAxisPoint Vec2 Struct Position
     * @param favorClRoad Boolean Condition To Partial Consider a closed road, a closed node state.
     * @return Boolean Condition
     */
    bool IsAClosedNodeState(vec2 &gridAxisPoint, bool favorClRoad = false) {
        Node node = GetNode(gridAxisPoint);
        return IsAClosedNodeState(node, favorClRoad);
    }

    /**
     *  IsAClosedNodeState() Validate If Node is a Closed State
     *
     * @param node Node Struct
     * @param favorClRoad Boolean Condition To Partial Consider a closed road, a closed node state.
     * @return Boolean Condition
     */
    bool IsAClosedNodeState(const Node &node, bool favorClRoad = false) {
        if (node.currentState == CLOSED_FACTORY || node.currentState == CLOSED_HOUSE ||
            (favorClRoad && node.currentState == CLOSED_ROAD)) {
            return true;
        }
        return false;
    }

    /**
     * ResetNodes() Reset Selected Nodes To Previous State
     *
     * @param gridAxisPoints Collection
     * @param enableReset Boolean Condition For Reversion
     */
    void ResetNodes(vector<vec2> gridAxisPoints, bool enableReset) {
        this->revertState = enableReset;
        for (vec2 &pt: gridAxisPoints) {
            int mapToIndex = CombineDigits((int) pt.y, (int) pt.x);
            NodeHandler(mapToIndex);
        }
        this->revertState = false;
    }

    /**
     *  GridReset() Reset All Grid Nodes
     */
    void GridReset() {
        this->gridDestObjectives.clear();
        for (Node &cell: gridNodes) {
            cell.NodeReset();
        }
    }

    /**
     * IsAllDestinationLinked() Validate If All Destination Objectives Have Been Linked.
     * @return Boolean Condition
     */
    bool IsAllDestinationLinked() {
        for (DestinationObjectives &objectives: gridDestObjectives) {
            if (objectives.destLinked == false) {
                return false;
            }
        }
        return true;
    }

    /**
     * GridClearAndCountRoads() Road Counter
     * @return Integer
     */
    int GridClearAndCountRoads() {
        int counter = 0;
        for (Node &cell: gridNodes) {
            if (cell.currentState == CLOSED_ROAD) {
                counter++;
            }
            if (!IsAClosedNodeState(cell)) {
                cell.NodeReset();
            }
            cell.isConnected = false;
        }
        for (DestinationObjectives &objectives: gridDestObjectives) {
            if (objectives.destLinked) {
                objectives.destLinked = false;
            }
        }
        return counter;
    }
};

#endif //ROADREALM_GRID_H
