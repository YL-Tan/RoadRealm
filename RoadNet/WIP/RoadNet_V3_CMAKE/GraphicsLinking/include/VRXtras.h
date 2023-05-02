// VRXtras.h (c) 2021 by Jules Bloomenthal, all rights reserved. Commercial use requires license.

#ifndef VR_XTRAS_HDR
#define VR_XTRAS_HDR

#include "glad.h"
#include "Quaternion.h"
#include "VecMat.h"

class VROOM {
public:
// private:
	GLuint depthBuffer = 0, framebufferTextureName = 0;
	int width = 0, height = 0;
	float *pixels = NULL;
	mat4 appTransformStart, *appTransform = NULL;
public:
	float translationScale = 1;
	Quaternion qStart, qStartConjugate;
	vec3 pStart;
	bool hmdPresent = false, openVR = false;
	GLuint framebuffer = 0;
	bool InitFrameBuffer(int width, int height);
		// build frame buffer for eye rendering
	void CopyFramebufferToEyeTexture(GLuint textureName, GLuint textureUnit);
		// after rendering, copy frame buffer pixels to texture image
	void SubmitOpenGLFrames(GLuint leftTextureUnit, GLuint rightTextureUnit);
		// provide left/right eye texture identifiers for new frame
	bool InitOpenVR();
		// required before any access to OpenVR
	bool StartTransformHMD(mat4 *m);
		// offset raw HMD transform, set m as target (ie, app head) transform
	void UpdateTransformHMD();
		// poll HMD and update target transform
	std::string GetTrackedDeviceType(int type);
	//void ProcessEvent(const vr::VREvent_t & event);
	// std::string GetDriverName();
	// std::string GetDriverSerial();
	~VROOM() { if (pixels) delete [] pixels; }
};

#endif
