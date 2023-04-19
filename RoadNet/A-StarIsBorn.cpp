/*  AStarIsBorn.cpp: illustrate A* search algorithm for path-finding, Jules Bloomenthal 2011, updated 2023
	Pseudocode from Wikipedia
	function A*(start,goal)
		closedset := the empty set    // set of nodes already evaluated.
		openset := {start}            // set of tentative nodes to be evaluated, initially containing the start node
		came_from := the empty map    // map of navigated nodes.
		g_score[start] := 0           // cost from start along best known path.
		h_score[start] := heuristic_cost_estimate(start, goal)
		f_score[start] := g_score[start] + h_score[start]    // est. total cost from start to goal through y.
		while openset is not empty
			x := the node in openset having the lowest f_score[] value
			if x = goal
				return reconstruct_path(came_from, came_from[goal])
			remove x from openset
			add x to closedset
			foreach y in neighbor_nodes(x)
				if y in closedset
					continue
				tentative_g_score := g_score[x] + dist_between(x,y)

				if y not in openset
					add y to openset
					h_score[y] := heuristic_cost_estimate(y, goal)
					tentative_is_better := true
				else if tentative_g_score < g_score[y]
					tentative_is_better := true
				else
					tentative_is_better := false

				if tentative_is_better = true
					came_from[y] := x
					g_score[y] := tentative_g_score
					f_score[y] := g_score[y] + h_score[y]
		return failure
	function reconstruct_path(came_from, current_node)
		if came_from[current_node] is set
			p := reconstruct_path(came_from, came_from[current_node])
			return (p + current_node)
		else
			return current_node */

#include <glad.h>
#include <GLFW/glfw3.h>
#include "Draw.h"
#include "GLXtras.h"
#include <limits>
#include <time.h>
#include <vector>

using std::vector;

#define	NROWS 16
#define	NCOLS 14

vec3	wht(1,1,1), blk(0,0,0), gry(.5,.5,.5), red(1,0,0), grn(0,1,0), blu(0, 0, 1),
		dkgrn(0,.6f,0), yel(1,1,0), cyn(0,1,1), mrn(.7f, 0, 0), dkcyn(0,.5f,.5f), org(1,.55f, 0), prp(.8f,.1f,.5f);

int		appWidth = 1000, appHeight = 800;
int		x = 20, y = 20, w = appWidth-40, h = appHeight-40;
float	dx = (float) w/NCOLS, dy = (float) h/NROWS;
time_t	oldtime = clock();

// Class Declarations

class Index {
// represents a grid cell's row and column indices. 
// It provides methods for calculating distances between two cells and checking if a cell is valid.
public:
	int row = -1, col = -1;
	Index() { };
	Index(int r, int c) : row(r), col(c) { };
	float DistanceTo(Index i) {
		float drow = (float) (i.row-row), dcol = (float) (i.col-col);
		return sqrt(drow*drow+dcol*dcol);
	}
	bool Valid() { return row >= 0 && row < NROWS && col >= 0 && col < NCOLS; }
	vec2 Point() { return vec2(x+(col+.5)*dx, y+(row+.5)*dy); }
	bool operator==(Index &i) { return i.row == row && i.col == col; }
};

Index start(0, 0), stop(14, 12);
vector<Index> path;
float pathLength;

void DrawVertex(float row, float col) { Disk(vec2(x+(col+.5)*dx, y+(row+.5)*dy), 6, blu); }

void DrawSegment(Index i1, Index i2, float width, vec3 col) { Line(i1.Point(), i2.Point(), width, col, col); }

void DrawPath(vector<Index> &path, float width, vec3 color) {
	for (size_t i = 1; i < path.size(); i++)
		DrawSegment(path[i-1], path[i], width, color);
}

void DrawRectangle(int x, int y, int w, int h, vec3 col) {
	Quad(x, y, x, y+h, x+w, y+h, x+w, y, true, col);
}

float PathLength() {
	float len = 0;
	for (size_t i = 1; i < path.size(); i++)
		len += path[i].DistanceTo(path[i-1]);
	return len;
}

vec2 PointOnPath(float dist) {
	float accumDist = 0;
	Index i1 = path[0];
	for (size_t i = 1; i < path.size(); i++) {
		Index i2 = path[i];
		float d = i1.DistanceTo(i2);
		accumDist += d;
		if (accumDist >= dist) {
			float alpha = (accumDist-dist)/d;
			return vec2(i2.col+alpha*(i1.col-i2.col), i2.row+alpha*(i1.row-i2.row));
		}
		i1 = i2;
	}
	return vec2(i1.col, i1.row);
}

class Node {
// represents a node in the A* search algorithm. It stores information about the node's state
// (open, closed, or blocked) and its g, h, and f scores, which are used in the A* algorithm.
public:
	Index from;
	float g = 0, h = 0, f = 0;
	bool blocked = false, open = false, closed = false;
	Node() { }
};

class AStar {
// has methods for computing the shortest path between two points. 
// It also provides methods for drawing the grid, nodes, and paths.
public:
	// rectangular array of nodes
	Node nodes[NROWS][NCOLS];
	// start and goal nodes, set by ComputePath
	Index start, goal;
	int ReconstructPath(Index start, vector<Index> &path) {
		path.resize(0);
		path.push_back(start);
		Node &n = nodes[start.row][start.col];
		while (n.from.row >= 0) {
			path.push_back(n.from);
			n = nodes[n.from.row][n.from.col];
		}
		return path.size();
	}
	bool LowestIndex(Index &lowest) {
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
	bool ComputePath(Index s, Index g) {
		// initialize
		start = s;
		goal = g;
		Node &n = nodes[start.row][start.col];
		n.g = 0;
		n.h = start.DistanceTo(goal);
		n.f = n.h+n.g;
		n.open = true;
		// loop
		Index xIndex;
		while (LowestIndex(xIndex)) {
			if (xIndex.col == goal.col && xIndex.row == goal.row)
				return true;
			Node &x = nodes[xIndex.row][xIndex.col];
			// remove x from openSet, add to closedSet
			x.open = false;
			x.closed = true;
			// for each y in neighbor_nodes(x)
			for (int col = xIndex.col-1; col <= xIndex.col+1; col++)
				for (int row = xIndex.row-1; row <= xIndex.row+1; row++) {
					Index yIndex(row, col);
					// test all eight neighbors, but not self
					if (yIndex == xIndex || !yIndex.Valid())
						continue;
					// skip closed nodes
					Node &y = nodes[row][col];
					if (y.closed || y.blocked)
						continue;
					// tentative_g_score := g_score[x] + dist_between(x,y)
					bool tentativeIsBetter;
					float tentativeG = x.g+xIndex.DistanceTo(yIndex);
					if (!y.open) {
						y.open = true;
						y.h = yIndex.DistanceTo(goal);
						tentativeIsBetter = true;
					}
					else
						tentativeIsBetter = tentativeG < y.g;

					if (tentativeIsBetter) {
						y.from = xIndex;
						y.g = tentativeG;
						y.f = y.g+y.h;
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
					DrawSegment(Index(row, col), n.from, width, color);
			}
	}
	void Draw() {
		DrawRectangle(x, y, w, h, blk);
		for (int row = 0; row < NROWS; row++)
			for (int col = 0; col < NCOLS; col++) {
				Node n = nodes[row][col];
				vec3 color = n.open? dkgrn : n.closed? mrn : n.blocked? blu : wht;
				DrawRectangle((int) (x+dx*(float)col), (int) (y+dy*(float)row), (int) dx-1, (int) dy-1, color);
			}
		glPointSize(20);
		if (start.row > -1)
			Disk(vec2(x+(start.col+.5)*dx, y+(start.row+.5)*dy), 20, red);
		if (goal.row > -1)
			Disk(vec2(x+(goal.col+.5)*dx, y+(goal.row+.5)*dy), 20, blu);
	}
};

class Bot {
// represents a moving agent on the grid. It has methods for updating the agent's position 
// along the computed path and drawing the agent on the screen.
public:
	float speed = -.5, t = 1;
	Bot() { }
	Bot(float s, float tt) : t(tt), speed(s) { }
	void Update(float dt) {
		t += dt*speed;
		if (t < 0 || t > 1) speed = -speed;
		t = t < 0? 0 : t > 1? 1 : t;
	}
	void Draw(vec3 color) {
		vec2 p = PointOnPath(t*pathLength);
		Disk(vec2(x+(p.x+.5)*dx, y+(p.y+.5)*dy), 20, color);
	}
};

AStar astar;
Bot botA(-.5f, .2f), botB(.3f, .8f);

// Display
// updating the state of the application, and rendering the graphics.
void Update() {
	time_t now = clock();
	float dt = (float)(oldtime-now)/CLOCKS_PER_SEC;
	oldtime = now;
	botA.Update(dt);
	botB.Update(dt);
}

void Display() {
	// clear rgb and alpha buffers
	glClearColor(.5f, .35f, .65f, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	// set smooth, disable zbuffer
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	UseDrawShader(ScreenMode());
	// a-star nodes and paths
	astar.Draw();
	astar.DrawPaths(1.5f, org);
	DrawPath(path, 3.5f, wht);
	// bots
	botA.Draw(yel);
	botB.Draw(grn);
	// finish
	glFlush();
}

// Callbacks

void MouseButton(float xmouse, float ymouse, bool left, bool down) {
	if (down) {
		int col = (int) ((xmouse-x)/dx), row = (int) ((ymouse-y)/dy);
		astar.nodes[row][col].blocked = !astar.nodes[row][col].blocked;
		// reset astar nodes
		for (int row = 0; row < NROWS; row++)
			for (int col = 0; col < NCOLS; col++) {
				Node &n = astar.nodes[row][col];
				n.open = n.closed = false;
			}
		// compute new path
		astar.ComputePath(start, stop);
		astar.ReconstructPath(astar.goal, path);
		pathLength = PathLength();
	}
}

void Resize(int width, int height) { glViewport(0, 0, width, height); }

// Application

int main(int ac, char **av) {
	// create window
	GLFWwindow *w = InitGLFW(100, 100, appWidth, appHeight, "A-Star");
	RegisterMouseButton(MouseButton);
	RegisterResize(Resize);
	// set blocks
	int blocks[][2] = {
		// verticals
		{2, 2}, {3, 2}, {4, 2},
		{4, 9}, {5, 9}, {6, 9}, {7, 9}, {8, 9}, {9, 9}, {10, 9},
		{7, 11}, {8, 11}, {9, 11}, {10, 11}, {11, 11}, {12, 11},
		// horizontals
		{7, 3}, {7, 4}, {7, 5}, {7, 6},
		{12, 4}, {12, 5}, {12, 6}, {12, 7}, {12, 8}, {12, 9},
		{13, 10}, {13, 11}
	};
	for (int i = 0; i < sizeof(blocks)/(2*sizeof(int)); i++)
		astar.nodes[blocks[i][0]][blocks[i][1]].blocked = true;
	// path-finding
	astar.ComputePath(start, stop);
	astar.ReconstructPath(astar.goal, path);
	pathLength = PathLength();
	// event loop
	while (!glfwWindowShouldClose(w)) {
		Update();
		Display();
		glfwSwapBuffers(w);
		glfwPollEvents();
	}
}
