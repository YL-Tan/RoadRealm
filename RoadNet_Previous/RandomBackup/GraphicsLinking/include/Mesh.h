// Mesh.h - 3D mesh of triangles (c) 2019-2022 Jules Bloomenthal

#ifndef MESH_HDR
#define MESH_HDR

#include <vector>
#include "glad.h"
#include "Camera.h"
#include "IO.h"
#include "Quaternion.h"
#include "VecMat.h"

using std::string;
using std::vector;

// Mesh Class and Operations

GLuint GetMeshShader(bool lines = false);
GLuint UseMeshShader(bool lines = false);
	// lines true uses geometry shader to draw lines along triangle edges
	// lines false is slightly more efficient

class Mesh {
public:
	Mesh() { };
	Mesh(const char *filename) { Read(string(filename)); }
	~Mesh() { glDeleteBuffers(1, &vBufferId); };
	string objFilename, texFilename;
	// vertices and facets
	vector<vec3>	points;
	vector<vec3>	normals;
	vector<vec2>	uvs;
	vector<int3>	triangles;
	vector<int4>	quads;
	// ancillary data
	vector<Group>	triangleGroups;
	vector<Mtl>		triangleMtls;
	// position/orientation
	vec3			centerOfRotation;
	mat4			toWorld;		// view = camera.modelview*toWorld
	mat4			wrtParent;		// toWorld = parent->toWorld*wrtParent
	// hierarchy
	Mesh		   *parent = NULL;
	vector<Mesh *>	children;
	// GPU vertex buffer and texture
	GLuint			vao = 0;		// vertex array object
	GLuint			vBufferId = 0;	// vertex buffer
	GLuint			eBufferId = 0;	// element (triangle) buffer
	GLuint			textureName = 0;
	// operations
	void Clear();
	void Buffer();
	void Buffer(vector<vec3> &pts, vector<vec3> *nrms = NULL, vector<vec2> *uvs = NULL);
		// if non-null, nrms and uvs assumed same size as pts
	void Set(vector<vec3> &pts, vector<vec3> *nrms = NULL, vector<vec2> *tex = NULL,
			 vector<int> *tris = NULL, vector<int> *quads = NULL);
	void SetToWorld();
		// for this mesh set toWorld given parent and wrtParent; recurse on children
	bool SetWrtParent();
		// for this mesh set wrtParent given parent and toWorld
	void Display(Camera camera, int textureUnit = -1, bool lines = false, bool useGroupColor = false);
		// texture is enabled if textureUnit >= 0 and textureName set
		// before this call, app must optionally change uniforms from their default, including:
		//     nLights, lights, color, opacity, ambient
		//     useLight, useTint, fwdFacingOnly, facetedShading
		//     outlineColor, outlineWidth, transition
	bool Read(string objFile, mat4 *m = NULL, bool standardize = true, bool buffer = true, bool forceTriangles = false);
		// read in object file (with normals, uvs), initialize matrix, build vertex buffer
	bool Read(string objFile, string texFile, mat4 *m = NULL, bool standardize = true, bool buffer = true, bool forceTriangles = false);
		// read in object file (with normals, uvs) and texture file, initialize matrix, build vertex buffer
};

struct TriInfo {
	vec4 plane;
	int majorPlane = 0; // 0: XY, 1: XZ, 2: YZ
	vec2 p1, p2, p3;    // vertices projected to majorPlane
	TriInfo() { };
	TriInfo(vec3 p1, vec3 p2, vec3 p3);
};

struct QuadInfo {
	vec4 plane;
	int majorPlane = 0;
	vec2 p1, p2, p3, p4;
	QuadInfo() { };
	QuadInfo(vec3 p1, vec3 p2, vec3 p3, vec3 p4);
};

void BuildTriInfos(vector<vec3> &points, vector<int3> &triangles, vector<TriInfo> &triInfos);
	// for interactive selection

void BuildQuadInfos(vector<vec3> &points, vector<int4> &quads, vector<QuadInfo> &quadiInfos);

int IntersectWithLine(vec3 p1, vec3 p2, vector<TriInfo> &triInfos, float &alpha);
	// return triangle index of nearest intersected triangle, or -1 if none
	// intersection = p1+alpha*(p2-p1)

int IntersectWithLine(vec3 p1, vec3 p2, vector<QuadInfo> &quadInfos, float &alpha);

#endif
