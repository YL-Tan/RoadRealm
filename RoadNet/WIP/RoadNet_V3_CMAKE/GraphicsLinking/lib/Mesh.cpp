// Mesh.cpp - mesh operations (c) 2019-2023 Jules Bloomenthal

#include "GLXtras.h"
#include "Draw.h"
#include "Misc.h"
#include "Mesh.h"

namespace {

GLuint meshShaderLines = 0, meshShaderNoLines = 0;

// vertex shader
const char *meshVertexShader = R"(
	#version 410 core
	layout (location = 0) in vec3 point;
	layout (location = 1) in vec3 normal;
	layout (location = 2) in vec2 uv;
	layout (location = 3) in mat4 instance; // for use with glDrawArrays/ElementsInstanced
											// uses locations 3,4,5,6 for 4 vec4s = mat4
	layout (location = 7) in vec3 color;	// for instanced color (vec4?)
	out vec3 vPoint;
	out vec3 vNormal;
	out vec2 vUv;
//	out vec3 vColor;
	uniform bool useInstance = false;
	uniform mat4 modelview;
	uniform mat4 persp;
	void main() {
		mat4 m = useInstance? modelview*instance : modelview;
		vPoint = (m*vec4(point, 1)).xyz;
		vNormal = (m*vec4(normal, 0)).xyz;
		gl_Position = persp*vec4(vPoint, 1);
		vUv = uv;
//		vColor = color;
	}
)";

// geometry shader
const char *meshGeometryShader = R"(
	#version 410 core
	layout (triangles) in;
	layout (triangle_strip, max_vertices = 3) out;
	in vec3 vPoint[], vNormal[];
	in vec2 vUv[];
	out vec3 gPoint, gNormal;
	out vec2 gUv;
	noperspective out vec3 gEdgeDistance;
	uniform mat4 vp;
	vec3 ViewPoint(int i) { return vec3(vp*(gl_in[i].gl_Position/gl_in[i].gl_Position.w)); }
	void main() {
		float ha = 0, hb = 0, hc = 0;
		// transform each vertex to viewport space
		vec3 p0 = ViewPoint(0), p1 = ViewPoint(1), p2 = ViewPoint(2);
		// find altitudes ha, hb, hc
		float a = length(p2-p1), b = length(p2-p0), c = length(p1-p0);
		float alpha = acos((b*b+c*c-a*a)/(2.*b*c));
		float beta = acos((a*a+c*c-b*b)/(2.*a*c));
		ha = abs(c*sin(beta));
		hb = abs(c*sin(alpha));
		hc = abs(b*sin(alpha));
		// send triangle vertices and edge distances
		for (int i = 0; i < 3; i++) {
			gEdgeDistance = i==0? vec3(ha, 0, 0) : i==1? vec3(0, hb, 0) : vec3(0, 0, hc);
			gPoint = vPoint[i];
			gNormal = vNormal[i];
			gUv = vUv[i];
			gl_Position = gl_in[i].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
)";

// pixel shader
const char *meshPixelShaderLines = R"(
	#version 410 core
	in vec3 gPoint, gNormal;
	in vec2 gUv;
	noperspective in vec3 gEdgeDistance;
	uniform sampler2D textureImage;
	uniform int nLights = 0;
	uniform vec3 lights[20];
	uniform vec3 defaultLight = vec3(1, 1, 1);
	uniform vec3 color = vec3(1);
	uniform float opacity = 1;
	uniform float ambient = .2;
	uniform bool useLight = true;
	uniform bool useTexture = true;
	uniform bool useTint = false;
	uniform bool fwdFacingOnly = false;
	uniform bool facetedShading = false;
	uniform vec4 outlineColor = vec4(0, 0, 0, 1);
	uniform float outlineWidth = 1;
	uniform float outlineTransition = 1;
	out vec4 pColor;
	float Intensity(vec3 normalV, vec3 eyeV, vec3 point, vec3 light) {
		vec3 lightV = normalize(light-point);		// light vector
		vec3 reflectV = reflect(lightV, normalV);   // highlight vector
		float d = max(0, dot(normalV, lightV));     // one-sided diffuse
		float s = max(0, dot(reflectV, eyeV));      // one-sided specular
		return clamp(d+pow(s, 50), 0, 1);
	}
	void main() {
		vec3 N = normalize(facetedShading? cross(dFdx(gPoint), dFdy(gPoint)) : gNormal);
		if (fwdFacingOnly && N.z < 0)
			discard;
		vec3 E = normalize(gPoint);					// eye vector
		float intensity = useLight? 0 : 1;
		if (useLight) {
			if (nLights == 0)
				intensity += Intensity(N, E, gPoint, defaultLight);
			else
				for (int i = 0; i < nLights; i++)
					intensity += Intensity(N, E, gPoint, lights[i]);
		}
		intensity = clamp(intensity, 0, 1);
		if (useTexture) {
			pColor = vec4(intensity*texture(textureImage, gUv).rgb, opacity);
			if (useTint) {
				pColor.r *= color.r;
				pColor.g *= color.g;
				pColor.b *= color.b;
			}
		}
		else
			pColor = vec4(intensity*color, opacity);
		float minDist = min(gEdgeDistance.x, gEdgeDistance.y);
		minDist = min(minDist, gEdgeDistance.z);
		float t = smoothstep(outlineWidth-outlineTransition, outlineWidth+outlineTransition, minDist);
		// mix edge and surface colors(t=0: edgeColor, t=1: surfaceColor)
		pColor = mix(outlineColor, pColor, t);
	}
)";

const char *meshPixelShaderNoLines = R"(
	#version 410 core
	in vec3 vPoint, vNormal;
	in vec2 vUv;
	uniform sampler2D textureImage;
	uniform int nLights = 0;
	uniform vec3 lights[20];
	uniform vec3 defaultLight = vec3(1, 1, 1);
	uniform vec3 color = vec3(1);
	uniform float opacity = 1;
	uniform float ambient = .2;
	uniform bool useLight = true;
	uniform bool useTexture = true;
	uniform bool useTint = false;
	uniform bool fwdFacingOnly = false;
	uniform bool facetedShading = false;
	out vec4 pColor;
	float Intensity(vec3 normalV, vec3 eyeV, vec3 point, vec3 light) {
		vec3 lightV = normalize(light-point);		// light vector
		vec3 reflectV = reflect(lightV, normalV);   // highlight vector
		float d = max(0, dot(normalV, lightV));     // one-sided diffuse
		float s = max(0, dot(reflectV, eyeV));      // one-sided specular
		return clamp(d+pow(s, 50), 0, 1);
	}
	void main() {
		vec3 N = normalize(facetedShading? cross(dFdx(vPoint), dFdy(vPoint)) : vNormal);
		if (fwdFacingOnly && N.z < 0)
			discard;
		vec3 E = normalize(vPoint);					// eye vector
		float intensity = useLight? 0 : 1;
		if (useLight) {
			if (nLights == 0)
				intensity += Intensity(N, E, vPoint, defaultLight);
			else
				for (int i = 0; i < nLights; i++)
					intensity += Intensity(N, E, vPoint, lights[i]);
		}
//		if (useLight)
//			for (int i = 0; i < nLights; i++)
//				intensity += Intensity(N, E, vPoint, lights[i]);
		intensity = clamp(intensity, 0, 1);
		if (useTexture) {
			pColor = vec4(intensity*texture(textureImage, vUv).rgb, opacity);
			if (useTint) {
				pColor.r *= color.r;
				pColor.g *= color.g;
				pColor.b *= color.b;
			}
		}
		else
			pColor = vec4(intensity*color, opacity);
	}
)";

} // end namespace

GLuint GetMeshShader(bool lines) {
	if (lines) {
		if (!meshShaderLines)
			meshShaderLines = LinkProgramViaCode(&meshVertexShader, NULL, NULL, &meshGeometryShader, &meshPixelShaderLines);
		return meshShaderLines;
	}
	else {
		if (!meshShaderNoLines)
			meshShaderNoLines = LinkProgramViaCode(&meshVertexShader, &meshPixelShaderNoLines);
		return meshShaderNoLines;
	}
}

GLuint UseMeshShader(bool lines) {
	GLuint s = GetMeshShader(lines);
	glUseProgram(s);
	return s;
}

// Mesh Class

void Mesh::SetToWorld() {
	// set toWorld given parent and wrtParent; recurse on children
	toWorld = (parent? parent->toWorld : mat4())*wrtParent;
	for (size_t i = 0; i < children.size(); i++)
		children[i]->SetToWorld();
}

bool Mesh::SetWrtParent() {
	// set wrtParent: toWorld = parent.toWorld*wrtParent
	if (parent != NULL) {
		mat4 inv;
		if (!InverseMatrix4x4((float *) &parent->toWorld, (float *) &inv))
			return false;
		wrtParent = inv*toWorld;
	}
	return true;
}

void Mesh::Display(Camera camera, int textureUnit, bool lines, bool useGroupColor) {
	size_t nTris = triangles.size(), nQuads = quads.size();
	// enable shader and vertex array object
	int shader = UseMeshShader(lines);
	glBindVertexArray(vao);
	// texture
	bool useTexture = textureName > 0 && uvs.size() > 0 && textureUnit >= 0;
//	if (!textureName || !uvs.size() || textureUnit < 0)
//		SetUniform(shader, "useTexture", false);
//	else {
	SetUniform(shader, "useTexture", useTexture);
	if (useTexture) {
		glActiveTexture(GL_TEXTURE0+textureUnit);
		glBindTexture(GL_TEXTURE_2D, textureName);
		SetUniform(shader, "textureImage", textureUnit); // but app can unset useTexture
	}
	// set matrices
	SetUniform(shader, "modelview", camera.modelview*toWorld);
	SetUniform(shader, "persp", camera.persp);
	if (lines)
		SetUniform(shader, "vp", Viewport());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBufferId);
	if (useGroupColor) {
		int textureSet = 0;
		glGetUniformiv(shader, glGetUniformLocation(shader, "useTexture"), &textureSet);
		// show ungrouped triangles without texture mapping
		int nGroups = triangleGroups.size(), nUngrouped = nGroups? triangleGroups[0].startTriangle : nTris;
		SetUniform(shader, "useTexture", false);
		glDrawElements(GL_TRIANGLES, 3*nUngrouped, GL_UNSIGNED_INT, 0); // triangles.data());
		// show grouped triangles with texture mapping
		SetUniform(shader, "useTexture", textureSet == 1);
		for (int i = 0; i < nGroups; i++) {
			Group g = triangleGroups[i];
			SetUniform(shader, "color", g.color);
			glDrawElements(GL_TRIANGLES, 3*g.nTriangles, GL_UNSIGNED_INT, (void *) (3*g.startTriangle*sizeof(int)));
		}
	}
	else {
		glDrawElements(GL_TRIANGLES, 3*nTris, GL_UNSIGNED_INT, 0);
#ifdef GL_QUADS
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDrawElements(GL_QUADS, 4*nQuads, GL_UNSIGNED_INT, quads.data());
#endif
	}
	glBindVertexArray(0);
}

void Enable(int id, int ncomps, int offset) {
	glEnableVertexAttribArray(id);
	glVertexAttribPointer(id, ncomps, GL_FLOAT, GL_FALSE, 0, (void *) offset);
}

void Mesh::Buffer(vector<vec3> &pts, vector<vec3> *nrms, vector<vec2> *tex) {
	size_t nPts = pts.size(), nNrms = nrms? nrms->size() : 0, nUvs = tex? tex->size() : 0;
	if (!nPts) { printf("Buffer: no points!\n"); return; }
	// create vertex buffer
	if (!vBufferId)
		glGenBuffers(1, &vBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vBufferId);
	// allocate GPU memory for vertex position, texture, normals
	size_t sizePoints = nPts*sizeof(vec3), sizeNormals = nNrms*sizeof(vec3), sizeUvs = nUvs*sizeof(vec2);
	int bufferSize = sizePoints+sizeUvs+sizeNormals;
	glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
	// load vertex buffer
	if (nPts) glBufferSubData(GL_ARRAY_BUFFER, 0, sizePoints, pts.data());
	if (nNrms) glBufferSubData(GL_ARRAY_BUFFER, sizePoints, sizeNormals, nrms->data());
	if (nUvs) glBufferSubData(GL_ARRAY_BUFFER, sizePoints+sizeNormals, sizeUvs, tex->data());
	// create and load element buffer for triangles
	size_t sizeTriangles = sizeof(int3)*triangles.size();
	glGenBuffers(1, &eBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeTriangles, triangles.data(), GL_STATIC_DRAW);
	// create vertex array object for mesh
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// enable attributes
	if (nPts) Enable(0, 3, 0);						// VertexAttribPointer(shader, "point", 3, 0, (void *) 0);
	if (nNrms) Enable(1, 3, sizePoints);			// VertexAttribPointer(shader, "normal", 3, 0, (void *) sizePoints);
	if (nUvs) Enable(2, 2, sizePoints+sizeNormals); // VertexAttribPointer(shader, "uv", 2, 0, (void *) (sizePoints+sizeNormals));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::Clear() {
	points.resize(0);
	normals.resize(0);
	uvs.resize(0);
	triangles.resize(0);
	quads.resize(0);
	triangleGroups.resize(0);
	triangleMtls.resize(0);
}

void Mesh::Buffer() { Buffer(points, normals.size()? &normals : NULL, uvs.size()? &uvs : NULL); }

void Mesh::Set(vector<vec3> &pts, vector<vec3> *nrms, vector<vec2> *tex, vector<int> *tris, vector<int> *quas) {
	if (tris) {
		triangles.resize(tris->size()/3);
		for (int i = 0; i < (int) triangles.size(); i++)
			triangles[i] = { (*tris)[3*i], (*tris)[3*i+1], (*tris)[3*i+2] };
	}
	if (quas) {
		quads.resize(quas->size()/4);
		for (int i = 0; i < (int) quads.size(); i++)
			quads[i] = { (*quas)[4*i], (*quas)[4*i+1], (*quas)[4*i+2], (*quas)[4*i+3] };
	}
	Buffer(pts, nrms, tex);
}

bool Mesh::Read(string objFile, mat4 *m, bool standardize, bool buffer, bool forceTriangles) {
	if (!ReadAsciiObj((char *) objFile.c_str(), points, triangles, &normals, &uvs, &triangleGroups, &triangleMtls, forceTriangles? NULL : &quads, NULL)) {
		printf("Mesh.Read: can't read %s\n", objFile.c_str());
		return false;
	}
	objFilename = objFile;
	if (standardize)
		Standardize(points.data(), points.size(), 1);
	if (buffer)
		Buffer();
	if (m)
		toWorld = *m;
	return true;
}

bool Mesh::Read(string objFile, string texFile, mat4 *m, bool standardize, bool buffer, bool forceTriangles) {
	if (!Read(objFile, m, standardize, buffer, forceTriangles))
		return false;
	objFilename = objFile;
	texFilename = texFile;
	textureName = ReadTexture((char *) texFile.c_str());
	if (!textureName)
		printf("Mesh.Read: bad texture name\n");
	return textureName > 0;
}

// intersections

vec2 MajPln(vec3 &p, int mp) { return mp == 1? vec2(p.y, p.z) : mp == 2? vec2(p.x, p.z) : vec2(p.x, p.y); }

TriInfo::TriInfo(vec3 a, vec3 b, vec3 c) {
	vec3 v1(b-a), v2(c-b), x = normalize(cross(v1, v2));
	plane = vec4(x.x, x.y, x.z, -dot(a, x));
	float ax = fabs(x.x), ay = fabs(x.y), az = fabs(x.z);
	majorPlane = ax > ay? (ax > az? 1 : 3) : (ay > az? 2 : 3);
	p1 = MajPln(a, majorPlane);
	p2 = MajPln(b, majorPlane);
	p3 = MajPln(c, majorPlane);
}

QuadInfo::QuadInfo(vec3 a, vec3 b, vec3 c, vec3 d) {
	vec3 v1(b-a), v2(c-b), x = normalize(cross(v1, v2));
	plane = vec4(x.x, x.y, x.z, -dot(a, x));
	float ax = fabs(x.x), ay = fabs(x.y), az = fabs(x.z);
	majorPlane = ax > ay? (ax > az? 1 : 3) : (ay > az? 2 : 3);
	p1 = MajPln(a, majorPlane);
	p2 = MajPln(b, majorPlane);
	p3 = MajPln(c, majorPlane);
	p4 = MajPln(d, majorPlane);
}

bool LineIntersectPlane(vec3 p1, vec3 p2, vec4 plane, vec3 *intersection, float *alpha) {
  vec3 normal(plane.x, plane.y, plane.z);
  vec3 axis(p2-p1);
  float pdDot = dot(axis, normal);
  if (fabs(pdDot) < FLT_MIN)
	  return false;
  float a = (-plane.w-dot(p1, normal))/pdDot;
  if (intersection != NULL)
	  *intersection = p1+a*axis;
  if (alpha)
	  *alpha = a;
  return true;
}

static bool IsZero(float d) { return d < FLT_EPSILON && d > -FLT_EPSILON; };

int CompareVs(vec2 &v1, vec2 &v2) {
	if ((v1.y > 0 && v2.y > 0) ||           // edge is fully above query point p'
		(v1.y < 0 && v2.y < 0) ||           // edge is fully below p'
		(v1.x < 0 && v2.x < 0))             // edge is fully left of p'
		return 0;                           // can't cross
	float zcross = v2.y*v1.x-v1.y*v2.x;     // right-handed cross-product
	zcross /= length(v1-v2);
	if (IsZero(zcross) && (v1.x <= 0 || v2.x <= 0))
		return 1;                           // on or very close to edge
	if ((v1.y > 0 || v2.y > 0) && ((v1.y-v2.y < 0) != (zcross < 0)))
		return 2;                           // edge is crossed
	else
		return 0;                           // edge not crossed
}

bool IsInside(const vec2 &p, vector<vec2> &pts) {
	bool odd = false;
	int npts = pts.size();
	vec2 q = p, v2 = pts[npts-1]-q;
	for (int n = 0; n < npts; n++) {
		vec2 v1 = v2;
		v2 = pts[n]-q;
		if (CompareVs(v1, v2) == 2)
			odd = !odd;
	}
	return odd;
}

bool IsInside(const vec2 &p, const vec2 &a, const vec2 &b, const vec2 &c) {
	bool odd = false;
	vec2 q = p, v2 = c-q;
	for (int n = 0; n < 3; n++) {
		vec2 v1 = v2;
		v2 = (n==0? a : n==1? b : c)-q;
		if (CompareVs(v1, v2) == 2)
			odd = !odd;
	}
	return odd;
}

void BuildTriInfos(vector<vec3> &points, vector<int3> &triangles, vector<TriInfo> &triInfos) {
	triInfos.resize(triangles.size());
	for (size_t i = 0; i < triangles.size(); i++)
		triInfos[i] = TriInfo(points[triangles[i].i1], points[triangles[i].i2], points[triangles[i].i3]);
}

void BuildQuadInfos(vector<vec3> &points, vector<int4> &quads, vector<QuadInfo> &quadInfos) {
	quadInfos.resize(quads.size());
	for (size_t i = 0; i < quads.size(); i++)
		quadInfos[i] = QuadInfo(points[quads[i].i1], points[quads[i].i2], points[quads[i].i3], points[quads[i].i4]);
}

int IntersectWithLine(vec3 p1, vec3 p2, vector<TriInfo> &triInfos, float &retAlpha) {
	int picked = -1;
	float alpha, minAlpha = FLT_MAX;
	for (size_t i = 0; i < triInfos.size(); i++) {
		TriInfo &t = triInfos[i];
		vec3 inter;
		if (LineIntersectPlane(p1, p2, t.plane, &inter, &alpha))
			if (alpha < minAlpha) {
				if (IsInside(MajPln(inter, t.majorPlane), t.p1, t.p2, t.p3)) {
					minAlpha = alpha;
					picked = i;
				}
			}
	}
	retAlpha = minAlpha;
	return picked;
}

int IntersectWithLine(vec3 p1, vec3 p2, vector<QuadInfo> &quadInfos, float &retAlpha) {
	int picked = -1;
	float alpha, minAlpha = FLT_MAX;
	for (size_t i = 0; i < quadInfos.size(); i++) {
		QuadInfo &q = quadInfos[i];
		vec3 inter;
		if (LineIntersectPlane(p1, p2, q.plane, &inter, &alpha))
			if (alpha < minAlpha) {
				if (IsInside(MajPln(inter, q.majorPlane), q.p1, q.p2, q.p3) ||
					IsInside(MajPln(inter, q.majorPlane), q.p1, q.p3, q.p4)) {
					minAlpha = alpha;
					picked = i;
				}
			}
	}
	retAlpha = minAlpha;
	return picked;
}

/* Wayside

const char *OLDmeshVertexShader = R"(
	#version 330
	layout (location = 0) in vec3 point;
	layout (location = 1) in vec3 normal;
	layout (location = 2) in vec2 uv;
	layout (location = 3) in mat4 instance; // for use with glDrawArrays/ElementsInstanced
											// uses locations 3,4,5,6 for 4 vec4s = mat4
	layout (location = 7) in vec3 color;	// for instanced color (vec4?)
	out vec3 vPoint;
	out vec3 vNormal;
	out vec2 vUv;
	out vec3 vColor;
	uniform bool useInstance = false;
	uniform mat4 modelview;
	uniform mat4 persp;
	void main() {
		mat4 m = useInstance? modelview*instance : modelview;
		vPoint = (m*vec4(point, 1)).xyz;
		vNormal = (m*vec4(normal, 0)).xyz;
		gl_Position = persp*vec4(vPoint, 1);
		vUv = uv;
		vColor = color;
	}
)";
const char *OLDmeshPixelShader = R"(
	#version 330
	in vec3 vPoint;
	in vec3 vNormal;
	in vec2 vUv;
	in vec3 vColor;
	out vec4 pColor;
	uniform int nLights = 0;
	uniform vec3 lights[20];
	uniform bool useLight = true;
	uniform sampler2D textureUnit;
	uniform vec3 defaultColor = vec3(1);
	uniform bool useDefaultColor = true;
	uniform float opacity = 1;
	uniform bool useTexture = false;
	uniform bool useTint = false;
	uniform bool fwdFacing = false;
	uniform bool facetedShading = false;
	float Intensity(vec3 normalV, vec3 eyeV, vec3 point, vec3 light) {
		vec3 lightV = normalize(light-point);		// light vector
		vec3 reflectV = reflect(lightV, normalV);   // highlight vector
		float d = max(0, dot(normalV, lightV));     // one-sided diffuse
		float s = max(0, dot(reflectV, eyeV));      // one-sided specular
		return clamp(d+pow(s, 50), 0, 1);
	}
	void main() {
		vec3 N = normalize(facetedShading? cross(dFdx(vPoint), dFdy(vPoint)) : vNormal);
		if (fwdFacing && N.z < 0) discard;
		vec3 E = normalize(vPoint);					// eye vector
		float intensity = useLight? 0 : 1;
		if (useLight) {
			for (int i = 0; i < nLights; i++)
				intensity += Intensity(N, E, vPoint, lights[i]);
			intensity = clamp(intensity, 0, 1);
		}
		vec3 color = useTexture? texture(textureUnit, vUv).rgb : useDefaultColor? defaultColor : vColor;
		if (useTexture && useTint) {
			color.r *= defaultColor.r;
			color.g *= defaultColor.g;
			color.b *= defaultColor.b;
		}
		pColor = vec4(intensity*color, opacity);
	}
)";

// if not using wrtParent:
void RotateTransform(Mesh *m, Quaternion qrot, vec3 *center) {
	// set m.transform, recurse on m.children
	// rotate selected mesh and child meshes by qrot (returned by Arcball::Drag)
	//   apply qrot to rotation elements of m->transform (upper left 3x3)
	//   if non-null center, rotate origin of m about center
	// recursive routine initially called with null center
	Quaternion qq = m->frameDown.orientation*qrot;
		// arcball:use=Camera(?) works (qrot*m->qstart Body? fails)
	// rotate m
	qq.SetMatrix(m->toWorld, m->frameDown.scale);
	if (center) {
		// this is a child mesh: rotate origin of mesh around center
		mat4 rot = qrot.GetMatrix();
		mat4 x = Translate((*center))*rot*Translate(-(*center));
		vec4 xbase = x*vec4(m->frameDown.position, 1);
		SetMatrixOrigin(m->toWorld, vec3(xbase.x, xbase.y, xbase.z));
	}
	// recurse on children
	for (int i = 0; i < (int) m->children.size(); i++)
		RotateTransform(m->children[i], qrot, center? center : &m->frameDown.position);
			// rotate descendant children around initial mesh base
}
void TranslateTransform(Mesh *m, vec3 pDif) {
	SetMatrixOrigin(m->toWorld, m->frameDown.position+pDif);
	for (int i = 0; i < (int) m->children.size(); i++)
		TranslateTransform(m->children[i], pDif);
}
void MeshFramer::Drag(int x, int y, mat4 modelview, mat4 persp) {
	if (moverPicked) {
		vec3 pDif = mover.Drag(x, y, modelview, persp);
		SetMatrixOrigin(mesh->toWorld, mesh->frameDown.position);
		for (int i = 0; i < (int) mesh->children.size(); i++)
			TranslateTransform(mesh->children[i], pDif);
		arcball.SetCenter(ScreenPoint(mesh->frameDown.position, persp*modelview));
	}
	else {
		Quaternion qrot = arcball.Drag(x, y);
		RotateTransform(mesh, qrot, NULL);
	}
	mesh->SetWrtParent();
	for (size_t i = 0; i < mesh->children.size(); i++)
		mesh->children[i]->SetToWorld();
}
*/
