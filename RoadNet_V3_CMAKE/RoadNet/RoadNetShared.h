/**
 * @file RoadNetShared.h
 * @author Team 8: Edwin Kaburu, Vincent Marklynn, Yong Long Tan
 * @date 5/29/2023
 */

#ifndef ROADREALM_ROADNETSHARED_H
#define ROADREALM_ROADNETSHARED_H

#include <iostream>
#include <vector>
#include "Draw.h"
#include <string>
#include <random>
#include "GLXtras.h"

using namespace std;

#define NROWS 14
#define NCOLS 14
#define H_EDGE_BUFFER 40
#define W_EDGE_BUFFER 100
#define INFO_MSG_SIZE 12
#define NUM_OF_RND_VAL 5

enum NodeStates {
    OPEN, CLOSED_ROAD, CLOSED_HOUSE, CLOSED_FACTORY, POTENTIAL_ROAD
};

enum GameplayState {
    DRAW_STATE, WIPE_STATE
};

enum ApplicationStates {
    STARTING_MENU, GAME_STATE
};

enum InfoLabelsIndex {
    MOUSE_CLICK_LABEL = 0,
    MOUSE_MOVE_LABEL = 1,
    DIMS_LABEL = 2,
    FPS_LABEL = 3,
    TIME_LABEL = 4,
    NUM_OF_ROAD_LABEL = 5,
    APPLICATION_STATE_LABEL = 6,
    GAMEPLAY_STATE_LABEL = 7,
    ERROR_MSG_LABEL = 8,
    RUNNERS_COUNT_LABEL = 9,
    COUNTDOWN = 10,
    EVT_MSG_LABEL = 11
};

int APP_WIDTH = 1000, APP_HEIGHT = 800, X_POS = 20, Y_POS = 20,
        GLOBAL_W = APP_WIDTH - W_EDGE_BUFFER, GLOBAL_H = APP_HEIGHT - H_EDGE_BUFFER;

int GRID_W = GLOBAL_W * 0.75, DISP_W = GLOBAL_W - (GLOBAL_W * 0.22); // +  150;
float DX = (float) GRID_W / NCOLS, DY = (float) GLOBAL_H /
                                        NROWS, MAX_DIAMETER_SIZE = 25, MAX_CIR_EXPANSION = 0.6, DIAMETER_PERCENT = 0, ANIMATION_SPEED = 0.65, DIAMETER_LENGTH = 0;

const vec3 WHITE(1, 1, 1), BLACK(0, 0, 0), GREY(.5, .5, .5), RED(1, 0, 0),
        GREEN(0, 1, 0), BLUE(0, 0, 1), YELLOW(1, 1, 0),
        ORANGE(1, .55f, 0), PURPLE(.8f, .1f, .5f), CYAN(0, 1, 1), PALE_GREY(.8, .8, .8);

GameplayState GLOBAL_GAMEPLAY_STATE = DRAW_STATE;
ApplicationStates APPLICATION_STATE = STARTING_MENU;
string GLOBAL_EVENT_LABEL;
bool REDUCE_DIAMETER = true;

/**
 * @class InfoPanel
 * @details Representation and functionalities for a Information System
 */
class InfoPanel {
private:

    /**
     * @struct Message
     * @details An Information System's Message Representation
     */
    struct Message {
        string msgInfo;
        vec3 msgColor = WHITE;

        /**
         * Message() Default  Struct Constructor For Message Instance
         * @param info String Information
         * @param color Vec3 Color
         */
        Message(string info, const vec3 &color) {
            this->msgInfo = info;
            this->msgColor = color;
        }
    };

    // Message Collection
    vector<Message> generalMsgInfo = {};

public:
    /**
     * InfoPanel() Default Constructor For a InfoPanel Instance.
     */
    InfoPanel() {
        for (int i = 0; i < INFO_MSG_SIZE; i++) {
            generalMsgInfo.push_back(Message("", WHITE));
        }
    }

    /**
     * InfoDisplay() Display Function For InfoPanel
     *
     * @param fontScale Float Font Size
     */
    void InfoDisplay(float fontScale) {
        int maxHeight = GLOBAL_H;
        if (APPLICATION_STATE != STARTING_MENU) {
            for (Message &message: generalMsgInfo) {
                Text(DISP_W + 5, maxHeight, message.msgColor, fontScale, message.msgInfo.c_str());

                maxHeight -= 20;
            }
        } else {
            Text(DISP_W + 5, maxHeight, generalMsgInfo.at(FPS_LABEL).msgColor, fontScale,
                 generalMsgInfo.at(FPS_LABEL).msgInfo.c_str());
            Text(DISP_W + 5, maxHeight - 20, generalMsgInfo.at(APPLICATION_STATE_LABEL).msgColor, fontScale,
                 generalMsgInfo.at(APPLICATION_STATE_LABEL).msgInfo.c_str());
        }

    }

    /**
     * AddMessage() Insert Message To Collect
     *
     * @param labelsIndex Type Of Message Label
     * @param msg String Information
     * @param msgColor Vec3 Color
     * @return Boolean Condition
     */
    bool AddMessage(InfoLabelsIndex labelsIndex, const string &msg, const vec3 &msgColor) {
        if (!msg.empty()) {
            // Add Message To Vector
            generalMsgInfo.at(labelsIndex).msgInfo = msg;
            generalMsgInfo.at(labelsIndex).msgColor = msgColor;
            return true;
        }
        return false;
    }
};

/**
 * NodePosition() Represent The Position Of A Node
 */
struct NodePosition {
    int row = -1, col = -1;

    /**
     * NodePosition() Default Struct Constructor For NodePosition
     */
    NodePosition() {};

    /**
     * NodePosition() Struct Constructor For NodePosition with Initial Variables
     *
     * @param r Integer Row
     * @param c Integer Column
     */
    NodePosition(int r, int c) : row(r), col(c) {};

    /**
     * DistanceTo()
     *
     * @param i NodePosition
     * @return Float Distance
     */
    float DistanceTo(NodePosition i) {
        float drow = (float) (i.row - row), dcol = (float) (i.col - col);
        return sqrt(drow * drow + dcol * dcol);
    }

    /**
     * Valid()
     * @return Boolean Condition
     */
    bool Valid() { return row >= 0 && row < NROWS && col >= 0 && col < NCOLS; }

    /**
     * Point() Find the Point Position On Screen
     * @return Vec2 Point
     */
    vec2 Point() { return vec2(X_POS + (col + .5) * DX, Y_POS + (row + .5) * DY); }

    /**
     * PosWindowProj() Position Projection On the Screen
     * @return vec4 Position
     */
    vec4 PosWindowProj() {
        return vec4((int) (X_POS + DX * (float) col), (int) (Y_POS + DY * (float) row),
                    (int) DX - 1, (int) DY - 1);
    }

    /**
     * AlignmentPosMatches() Similarity Position Test
     *
     * @param r Integer Row
     * @param c Integer Column
     * @return Boolean Conditions
     */
    bool AlignmentPosMatches(int r, int c) {
        if (row == r && col == c) {
            return true;
        }
        return false;
    }

    /**
     * AlignmentPosMatches() Similarity Position Test
     *
     * @param comparePos NodePosition
     * @return Boolean Condition
     */
    bool AlignmentPosMatches(NodePosition comparePos) {
        return AlignmentPosMatches(comparePos.row, comparePos.col);
    }

    /**
     * ==() Similarity Position Test
     * @param i NodePosition Struct
     * @return Boolean Condition
     */
    bool operator==(NodePosition &i) { return i.row == row && i.col == col; }
};

/**
 * DrawBorders() Draw Layout Border
 */
void DrawBorders() {
    // Top Line, (Top-Right -> Top-Left)
    Line(vec2(GLOBAL_W, GLOBAL_H + (H_EDGE_BUFFER / 2) + 1), vec2(5, GLOBAL_H + (H_EDGE_BUFFER / 2) + 1), 1, WHITE);
    // Left Line (Bottom-Left -> Top-Left)
    Line(vec2(5, 5), vec2(5, GLOBAL_H + (H_EDGE_BUFFER / 2) + 1), 1, WHITE);
    // Right Line (Top-Right -> Bottom-Right)
    Line(vec2(GLOBAL_W, GLOBAL_H + (H_EDGE_BUFFER / 2) + 1), vec2(GLOBAL_W, 5), 1, WHITE);
    // Mid Line, (Mid-Right, Mid-Left)
    Line(vec2(DISP_W, GLOBAL_H + (H_EDGE_BUFFER / 2) + 1), vec2(DISP_W, 5), 1, WHITE);
    // Bottom Line (Bottom-Left -> Bottom-Right)
    Line(vec2(5, 5), vec2(GLOBAL_W, 5), 1, WHITE);
}

/**
 * IsWithInBounds() In Bounds Validation Checker
 *
 * @param row Integer Row
 * @param col Integer Column
 * @return Boolean Condition
 */
bool IsWithInBounds(int row, int col) {
    if ((col < NCOLS && row < NROWS) && (col > -1 && row > -1)) {
        return true;
    }
    return false;
}

/**
 * IsWithInBounds() In Bounds Validation Checker
 *
 * @param point Vec2 Point
 * @return Boolean Condition
 */
bool IsWithInBounds(const vec2 &point) {
    return IsWithInBounds(point.y, point.x);
}

/**
 * GetDistance() Manhattan Distance Calculation
 *
 * @param firstPoint Vec2 Point
 * @param secondPoint Vec2 Point
 * @return Integer Distance
 */
int GetDistance(const vec2 &firstPoint, const vec2 &secondPoint) {
    int xAxisDist = (int) abs((secondPoint.x - firstPoint.x));
    int yAxisDist = (int) abs((secondPoint.y - firstPoint.y));

    return xAxisDist + yAxisDist;
}

/**
 * FindNeighbors()
 *
 * @param startingPoint Vec2 Starting Point
 * @param dist Integer Distance
 * @param potentialLocations Locations Collection
 */
void FindNeighbors(const vec2 &startingPoint, int dist, vector<vec2> &potentialLocations) {
    for (int i = (int) startingPoint.y - dist; i <= (int) startingPoint.y + dist; i++) {
        for (int j = (int) startingPoint.x - dist; j <= (int) startingPoint.x + dist; j++) {
            if ((!(i == (int) startingPoint.y && j == (int) startingPoint.x))) {
                if (IsWithInBounds(i, j) && GetDistance(vec2(j, i), startingPoint) > 2)
                    potentialLocations.emplace_back(j, i);
            }
        }
    }
}

/**
 * GetRandomDistribution() Random Distributed Numbers Generator
 *
 * @param distribLimit Integer Upper Bound Number Generated
 * @param numOfRndValues Integer Number Of Random Values
 * @param distribRndPlacement Vector Collective Holder
 */
void GetRandomDistribution(int distribLimit, int numOfRndValues, vector<int> &distribRndPlacement) {
    random_device generator;
    uniform_int_distribution<int> distribution(0, distribLimit);

    for (int i = 0; i < numOfRndValues; i++) {
        distribRndPlacement.push_back(distribution(generator));
    }
}

/**
 * GetRandomPoint() Get A Random Point
 *
 * @param numOfRndValues Integer Number Of Random Values
 * @param rndIndex Integer Index
 * @param potentialVal Vector Collective Potential Values
 * @return Vec2 Point
 */
vec2 GetRandomPoint(int numOfRndValues = NUM_OF_RND_VAL, int rndIndex = 0, const vector<vec2> &potentialVal = {}) {
    vector<int> rndNodePoint = {};
    GetRandomDistribution((NROWS * numOfRndValues), numOfRndValues, rndNodePoint);

    if (!potentialVal.empty()) {
        int getRndIndex = rndNodePoint.at(rndIndex) % potentialVal.size();
        return potentialVal.at(getRndIndex);
    }

    vec2 rndPoint = vec2((rndNodePoint.at(rand() % (rndNodePoint.size() - 1)) % NCOLS),
                         (rndNodePoint.at(rand() % (rndNodePoint.size() - 1)) % NROWS));
    return rndPoint;
}

/**
 * GetRandomColor() Get Random Color
 *
 * @param stride Integer Stride
 * @return Vec3 Color
 */
vec3 GetRandomColor(int stride = 1) {
    int maxColorShades = 255;

    vector<int> rndDistribColors = {};

    GetRandomDistribution(maxColorShades, 3, rndDistribColors);

    float red = (float) ((rndDistribColors.at(0) + stride) % maxColorShades) / (float) maxColorShades;
    float green = (float) ((rndDistribColors.at(1) + stride) % maxColorShades) / (float) maxColorShades;
    float blue = (float) ((rndDistribColors.at(2) + stride) % maxColorShades) / (float) maxColorShades;

    return {red, green, blue};
}

/**
 * DrawVertex() Draw a Vertex
 *
 * @param row Integer Row
 * @param col Integer Column
 */
void DrawVertex(float row, float col) {
    Disk(vec2(X_POS + (col + .5) * DX, Y_POS + (row + .5) * DY), 6, BLUE);
}

/**
 * DrawSegment() Draw a Segment
 *
 * @param i1 NodePosition Struct
 * @param i2 NodePosition Struct
 * @param width Float Width
 * @param col Vec3 Color
 */
void DrawSegment(NodePosition i1, NodePosition i2, float width, vec3 col) {
    Line(i1.Point(), i2.Point(), width, col, col);
}

/**
 * DrawPath() Draw A Path
 *
 * @param path vector Collective Path
 * @param width Float Width
 * @param color Vec3 Color
 */
void DrawPath(vector<NodePosition> &path, float width, vec3 color) {
    for (size_t i = 1; i < path.size(); i++)
        DrawSegment(path.at(i - 1), path.at(i), width, color);
}

/**
 * DrawRectangle() Draw A Rectangle
 *
 * @param x Integer x
 * @param y Integer y
 * @param w Integer w
 * @param h Integer h
 * @param col vec3 Color
 */
void DrawRectangle(int x, int y, int w, int h, vec3 col) {
    Quad(x, y, x, y + h, x + w, y + h, x + w, y, true, col);
}

/**
 * PathLength() Find Path length
 *
 * @param path vector collective path
 * @return Float Length
 */
float PathLength(vector<NodePosition> &path) {
    float len = 0;
    for (size_t i = 1; i < path.size(); i++)
        len += path[i].DistanceTo(path[i - 1]);
    return len;
}

/**
 * PointOnPath() Find Point On Path
 *
 * @param dist Float Dist
 * @param path vector collective path
 * @return vec2 Point
 */
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

/**
 * CombineDigits() Find Index Of Node With Grid Nodes
 *
 * @param leftDigit Integer Left Digit
 * @param rightDigit Integer Right Digit
 * @return Integer Mapping Index
 */
int CombineDigits(int leftDigit, int rightDigit) {
    //return (leftDigit * GetHighestTenthPow(rightDigit)) + rightDigit;
    return (NCOLS * leftDigit) + rightDigit;
}

/**
 * GetMin() Get the smallest digit
 *
 * @tparam D Data Type
 * @param digit1 D Digit 1
 * @param digit2  Digit 2
 * @return Smallest Digit
 */
template<typename D>
D GetMin(D digit1, D digit2) {
    if (std::is_same<D, int>::value || std::is_same<D, unsigned>::value || std::is_same<D, double>::value ||
        std::is_same<D, float>::value) {
        if (digit1 < digit2) {
            return digit1;
        }
        return digit2;
    }

    throw ::invalid_argument("Only Numbers in int, unsigned int, double, float in morphological form allowed");
}

/**
 * GetCirDiam() Get Circle Diameter
 *
 * @param animateDraw Boolean Animate Draw
 * @return Float Circle Diameter
 */
float GetCirDiam(bool animateDraw) {
    if (animateDraw) {
        if ((REDUCE_DIAMETER && DIAMETER_PERCENT > MAX_DIAMETER_SIZE) || (!REDUCE_DIAMETER && DIAMETER_PERCENT < 0)) {
            REDUCE_DIAMETER = !REDUCE_DIAMETER;
        }
        if (REDUCE_DIAMETER) {
            DIAMETER_PERCENT = DIAMETER_PERCENT + ANIMATION_SPEED;
        }
        if (!REDUCE_DIAMETER) {
            DIAMETER_PERCENT = DIAMETER_PERCENT - ANIMATION_SPEED;
        }
        DIAMETER_LENGTH = MAX_DIAMETER_SIZE - DIAMETER_PERCENT;
        return DIAMETER_LENGTH;
    }
    return MAX_DIAMETER_SIZE;
}

/**
 * PrintNodeState() Print NodeState Type
 *
 * @param nodeStates NodeState Type
 * @return String NodeState
 */
string PrintNodeState(NodeStates nodeStates) {
    if (nodeStates == CLOSED_ROAD) {
        return "CLOSED_ROAD";
    }
    if (nodeStates == CLOSED_HOUSE) {
        return "CLOSED_HOUSE";
    }
    if (nodeStates == CLOSED_FACTORY) {
        return "CLOSED_FACTORY";
    }
    if (nodeStates == POTENTIAL_ROAD) {
        return "POTENTIAL_ROAD";
    }
    return "OPEN";
}

/**
 * PrintGameplayState() Print GameplayState Type
 *
 * @return String GameplayState
 */
string PrintGameplayState() {
    if (GLOBAL_GAMEPLAY_STATE == WIPE_STATE) {
        return "WIPE_STATE";
    }
    return "DRAW STATE";
}

/**
 * PrintApplicationState() Print ApplicationState Type
 *
 * @return String ApplicationState
 */
string PrintApplicationState() {
    if (APPLICATION_STATE == STARTING_MENU) {
        return "STARTING_MENU";
    }
    return "GAME_STATE";
}

/**
 * @struct Node
 * @details Represent A Node.
 */
struct Node {
    NodePosition currentPos;

    vec3 color = WHITE;
    vec3 overlayColor = WHITE;

    NodeStates currentState = OPEN;
    NodeStates transState = POTENTIAL_ROAD;
    NodeStates prevState = OPEN;

    bool isConnected = false;

    /**
     * Node() Default Node Constructor
     */
    Node() {}

    /**
     * Node() Node Constructor with Initial Variables
     * @param row Integer Row
     * @param col Integer Columns
     */
    Node(int row, int col) {
        currentPos.row = row;
        currentPos.col = col;
    }

    /**
     * NodeReset() Reset A Node
     */
    void NodeReset() {
        this->color = WHITE;
        this->overlayColor = WHITE;

        this->currentState = OPEN;
        this->transState = POTENTIAL_ROAD;
        this->prevState = OPEN;
    }
};

/**
 * @struct Vehicle
 * @details Represents a Vehicle
 */
struct Vehicle {
    float speed = -.5, t = 1;
    vec3 overlayColor;
    vector<NodePosition> runnerPath;

    /**
     * Vehicle() Default Vehicle Constructor
     */
    Vehicle() {}

    /**
     * Vehicle() Vehicle Constructor with initial variables
     *
     * @param s Float S
     * @param tt Float tt
     */
    Vehicle(float s, float tt) {
        this->speed = s;
        this->t = tt;
    }

    /**
     * Vehicle() Vehicle Constructor with initial variables
     *
     * @param s Float S
     * @param tt Float tt
     * @param color Vec3 Color
     * @param path Vector Path Collection
     */
    Vehicle(float s, float tt, const vec3 &color, const vector<NodePosition> &path) {
        this->speed = s;
        this->t = tt;
        this->overlayColor = color;
        this->runnerPath = path;
    }

    /**
     * Update() Update Time And Speed derivative
     * @param dt Float DT
     */
    void Update(float dt) {
        t += speed * dt / PathLength(runnerPath);
        if (t < 0 || t > 1) speed = -speed;
        t = t < 0 ? 0 : t > 1 ? 1 : t;
    }

    /**
     * Draw() Vehicle Draw Function
     *
     * @param drawLogs String Draw Log Information
     */
    void Draw(string &drawLogs) {

        if (!runnerPath.empty()) {
            // Draw Path Using Vehicle Color
            DrawPath(runnerPath, 2.5f, overlayColor);

            float dist = t * PathLength(runnerPath);
            vec2 p = PointOnPath(dist, runnerPath);

            // Draw Disk Dot
            Disk(vec2(X_POS + (p.x + .5) * DX, Y_POS + (p.y + .5) * DY), (MAX_DIAMETER_SIZE * 0.5f), overlayColor);

            drawLogs = "runner pos: " + to_string(p.x) + " " + to_string(p.y);
        }
    }
};

/**
 * @struct RoadRunnerLinker
 * @details Vehicle And Destination Linker
 */
struct RoadRunnerLinker {
    string hashedId;
    bool isLinked = false;
    Vehicle vehicleRunner;

    /**
     * RoadRunnerLinker() Default Constructor with initial variables
     * @param hashedVal String hash value
     * @param runner Vehicle Structure
     * @param linkStatus Boolean Linking Status
     */
    RoadRunnerLinker(string hashedVal, const Vehicle &runner, bool linkStatus) {
        this->hashedId = hashedVal;
        this->vehicleRunner = runner;
        this->isLinked = linkStatus;
    }
};

#endif //ROADREALM_ROADNETSHARED_H
