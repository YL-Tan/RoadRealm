// Team 8 Road Network

#include <glad.h>
#include <glfw3.h>
#include "Draw.h"
#include "GLXtras.h"
#include <vector>

using namespace std;

struct Cell {
    // Arrangement x1, y1, x2, y2, x3, y3, x4, y4;
    vector<vec2> cellPoints;
};


class GridPrimitive {
private:
    int dimensions = 2, stride = 2, expectedNumOfPoints = 0, expectRowPoints = dimensions * 2;;
    //float startingX = -1.0f, startingY = 1.0f, cellLength = (1.0f / dimensions);
    float startingX = -0.9f, startingY = 0.9f, cellLength = (0.9f / dimensions);
    // float startingX = 0.0f, startingY = 500.0f, cellLength = (500.0f / dimensions);
public:
    vector<vec2> gridPrimitivePoints = {};
    vector<Cell> gridPrimitiveCells = {};

    GridPrimitive(int dim)
    {
        this->dimensions = dim;
        gridInitialize();
    }
    GridPrimitive() {
        gridInitialize();
    }

    void gridInitialize()
    {
        cout << "Formulating Grid";

        expectedNumOfPoints = (dimensions * dimensions) * 4;
        formulateGrid();
        formulateCellsMaps();
        cout << "\nTotal Number Of Cells:\t" << gridPrimitiveCells.size() << "\n";
    }

    bool isIntDivisible(int digit, int operand=2)
    {
        if(digit % operand == 0)
        {
            return true;
        }
        return false;
    }
    void formulateGrid() {
        cout << "\n";
        int numOfPoints = 0;

        int yCounter = 1;
        while (numOfPoints < expectedNumOfPoints) {
            // X-Axis
            float xRowReset = startingX;
            int rPointCounter = 0;

            int xCounter = 1;
            while (rPointCounter < expectRowPoints) {
                // Get Point Coordinate
                cout << "[" << xRowReset << " , " << startingY << "]\n";
                gridPrimitivePoints.push_back(vec2(xRowReset, startingY));
                xRowReset = xRowReset + cellLength;

                if(isIntDivisible(xCounter))
                {
                    xRowReset = xRowReset - (cellLength * 0.9f);
                }
                rPointCounter += 1;
                numOfPoints += 1;

                xCounter += 1;
            }
            startingY = startingY - cellLength;
            if (isIntDivisible(yCounter))
            {
                startingY = startingY + (cellLength * 0.9f);
            }

            yCounter += 1;
        }

        cout << "Num of Points: " << numOfPoints << "\tVector-Size: "
             << gridPrimitivePoints.size() << "\tExpected Points: "<< expectedNumOfPoints << "\n";
    }

    void formulateCellsMaps()
    {
        // int virtual_limit = dimensions * stride;
        int virtual_row = 0;

        int i = 0, numOfPairs = 0, cellMapIndex = 0;
        while ( i < gridPrimitivePoints.size() - 1)
        {
            vec2 pointA = gridPrimitivePoints.at(i);
            vec2 pointB = gridPrimitivePoints.at((i + 1));

            Cell line;
            if (!isIntDivisible(virtual_row))
            {
                // Get Saved Primitive Cell
                gridPrimitiveCells.at(cellMapIndex).cellPoints.push_back(pointA);
                gridPrimitiveCells.at(cellMapIndex).cellPoints.push_back(pointB);
                cellMapIndex += 1;
            }
            else
            {
                // New Primitive Cell
                line.cellPoints.push_back(pointA);
                line.cellPoints.push_back(pointB);
                gridPrimitiveCells.push_back(line);
                // Reset Cell Map Index
                cellMapIndex = gridPrimitiveCells.size() - dimensions;
            }

            numOfPairs += 1;

            if(numOfPairs == dimensions)
            {
                virtual_row += 1;
                numOfPairs = 0;
            }
            i += stride;
        }
    }

};

#define DIM 2
int WIN_WIDTH = 500, WIN_HEIGHT = 500;
const vec3 BACKGROUND_COLOR(0, 0, 0);
const vec3 DEFAULT_CELL_COLOR(0.8f, 0.93f, 0.06f);
// float dy = (float) (WIN_HEIGHT) / DIM, dx = (float) (WIN_WIDTH) / DIM;

float dy = (float) (WIN_HEIGHT) / 1000.0f,
        dx = (float) (WIN_WIDTH) / 1000.0f;

bool printOnce = true;

void Display(GridPrimitive &gridPrimitive) {
    glClearColor(BACKGROUND_COLOR.x, BACKGROUND_COLOR.y, BACKGROUND_COLOR.z, 1);	// set background color
    glClear(GL_COLOR_BUFFER_BIT);							// clear background and z-buffer

    // Create Custom Frag and Pixel Shader

    //UseDrawShader(ScreenMode());
    // Cell test = gridPrimitive.gridPrimitiveCells.at(0);
    // Cell test2 = gridPrimitive.gridPrimitiveCells.at(1);
    UseTriangleShader(Viewport());

    /*// Use Triangles
    Cell cellBox = gridPrimitive.gridPrimitiveCells.at(0);
    // Right-Right Angle.
    vec3 topLeftPt(cellBox.cellPoints.at(0), 0);
    vec3 pt1(cellBox.cellPoints.at(1), 0);
    vec3 pt2(cellBox.cellPoints.at(2), 0);
    Triangle(topLeftPt, pt1, pt2, DEFAULT_CELL_COLOR, DEFAULT_CELL_COLOR, DEFAULT_CELL_COLOR);*/

    for(Cell cellBox : gridPrimitive.gridPrimitiveCells)
    {
        Quad(cellBox.cellPoints.at(0).x , cellBox.cellPoints.at(0).y, // Vertical Lines
             cellBox.cellPoints.at(2).x, cellBox.cellPoints.at(2).y,

                // Horizontal Lines
             cellBox.cellPoints.at(3).x, cellBox.cellPoints.at(3).y,
             cellBox.cellPoints.at(1).x, cellBox.cellPoints.at(1).y,
             true,
             DEFAULT_CELL_COLOR,1.0f, 0.1f);
        if(printOnce)
        {
            cout << "\nx1\t y1\tx2\ty2\tx3\ty3\tx4\ty4";
            cout << "\n" <<
                 cellBox.cellPoints.at(0).x << "\t" <<
                 cellBox.cellPoints.at(0).y << "\t" <<
                 cellBox.cellPoints.at(2).x << "\t" <<
                 cellBox.cellPoints.at(2).y << "\t" <<
                 cellBox.cellPoints.at(3).x << "\t" <<
                 cellBox.cellPoints.at(3).y << "\t" <<
                 cellBox.cellPoints.at(1).x << "\t" <<
                 cellBox.cellPoints.at(1).y;
        }
        // Modify Dx and Dy
    }
    // Quad(0.0f, 0.0f, 0.0f, 248.55f, 249.45f,  248.55f, 249.45,  0.0f, true, DEFAULT_CELL_COLOR);

    printOnce = false;

    /* Quad(test.cellPoints.at(0).x, test.cellPoints.at(0).y, // Vertical Lines
          test.cellPoints.at(2).x, test.cellPoints.at(2).y,
          // Horizontal Lines
          test.cellPoints.at(3).x, test.cellPoints.at(3).y,
          test.cellPoints.at(1).x, test.cellPoints.at(1).y,
          true,
          DEFAULT_CELL_COLOR);*/

    /*
    // Vertical Line
    Line(test.cellPoints.at(0), test.cellPoints.at(2), (1.0f/2), DEFAULT_CELL_COLOR);
    Line(test.cellPoints.at(1), test.cellPoints.at(3), (1.0f/2), DEFAULT_CELL_COLOR);
    // Horizontal Line
    Line(test.cellPoints.at(0), test.cellPoints.at(1), (1.0f/2), DEFAULT_CELL_COLOR);
    Line(test.cellPoints.at(2), test.cellPoints.at(3), (1.0f/2), DEFAULT_CELL_COLOR);
     * */

    glFlush();
}

void Resize(int width, int height) {
    glViewport(0, 0, WIN_WIDTH = width, WIN_HEIGHT = height);
}

const char *usage = R"(
	mouse-click on block to pick
	mouse-drag to move block
)";

int main() {

    GridPrimitive gridPrimitive(DIM);

    GLFWwindow *w = InitGLFW(200, 200, WIN_WIDTH, WIN_HEIGHT, "Test");
    //printf("Usage:%s", usage);
    // RegisterMouseMove(MouseMove);
    // RegisterMouseButton(MouseButton);


    RegisterResize(Resize);
    //glfwSwapInterval(1);

    cout << "dy: " << dy << " dx: " << dx << "\n";

    while (!glfwWindowShouldClose(w)) {

        Display(gridPrimitive);
        glfwSwapBuffers(w);
        glfwPollEvents();
    }
    glfwDestroyWindow(w);
    glfwTerminate();
}
