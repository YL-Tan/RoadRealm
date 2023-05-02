// VR.cpp

#include <glad.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <openvr.h>
#include "VRXtras.h"

// see https://github.com/ValveSoftware/openvr/wiki/API-Documentation

using std::string;
using namespace vr;

IVRSystem *vr_context;
TrackedDevicePose_t tracked_device_pose[k_unMaxTrackedDeviceCount];

string deviceType[k_unMaxTrackedDeviceCount];
string deviceNames[] = {"Invalid", "HMD", "Controller", "GenericTracker", "TrackingReference", "DisplayRedirect"};

/* from openvr.h
enum ETrackedDeviceClass {
	TrackedDeviceClass_Invalid = 0,				// the ID was not valid.
	TrackedDeviceClass_HMD = 1,					// Head-Mounted Displays
	TrackedDeviceClass_Controller = 2,			// Tracked controllers
	TrackedDeviceClass_GenericTracker = 3,		// Generic trackers, similar to controllers
	TrackedDeviceClass_TrackingReference = 4,	// Camera and base stations that serve as tracking reference points
	TrackedDeviceClass_DisplayRedirect = 5,		// Accessories that aren't necessarily tracked themselves, but may redirect video output from other tracked devices
	TrackedDeviceClass_Max
}; */

string GetTrackedDeviceType(int type) { return deviceType[type]; }

// string driver_name, driver_serial;
// string GetDriverName() { return driver_name; }
// string GetDriverSerial() { return driver_serial; }

bool VROOM::InitOpenVR() {
//	openVR = VR_IsRuntimeInstalled();
//	hmdPresent = openVR && VR_IsHmdPresent();
//	if (!openVR || !hmdPresent) {
//		printf("No %s\n", !openVR? "VR runtime" : "HMD"  );
//		return false;
//	}
	// check runtime path
	char runtimePath[500] = { 0 };
	uint32_t requiredBufferSize;
	if (VR_GetRuntimePath(runtimePath, 500, &requiredBufferSize))
		printf("runtime installed at %s\n", runtimePath);
	printf("runtimePath = %s\n", runtimePath[0]? runtimePath : "<null>");

	// load SteamVR runtime
	HmdError err;
	vr_context = VR_Init(&err, EVRApplicationType::VRApplication_Other); // VRApplication_Scene);
		// VRApplication_Other		Some other kind of application that isn't covered by the other entries
		// VRApplication_Scene		Application will submit 3D frames
		// VRApplication_Overlay	Application only interacts with overlays
		// VRApplication_Background	Application should not start SteamVR if it's not already running
		// VRApplication_Utility	Init should not try to load any drivers. The application needs access to utility
	if (!vr_context) {
		printf("can't initialize SteamVR runtime: %s\n", VR_GetVRInitErrorAsSymbol(err));
		return false;
	}
	printf("SteamVR runtime successfully initialized\n");

	openVR = VR_IsRuntimeInstalled();
	hmdPresent = openVR && VR_IsHmdPresent();
	if (!openVR || !hmdPresent) {
		printf("no %s\n", !openVR? "VR runtime" : "HMD"  );
		return false;
	}

	// check for devices 
	int base_stations_count = 0;
	for (uint32_t td = k_unTrackedDeviceIndex_Hmd; td < k_unMaxTrackedDeviceCount; td++) {
		if (vr_context->IsTrackedDeviceConnected(td)) {
			ETrackedDeviceClass tracked_device_class = vr_context->GetTrackedDeviceClass(td);
			deviceType[td] = string(td > 5? "unknown" : deviceNames[td]);
			printf("tracking device %i is connected (%s)\n", td, deviceType[td].c_str());
			if (tracked_device_class == ETrackedDeviceClass::TrackedDeviceClass_TrackingReference)
				base_stations_count++;
			if (td == k_unTrackedDeviceIndex_Hmd)
				printf("can't set driver name/serial, but ok?\n");
		}
		else
			printf("tracking device %i not connected\n", td);
	}
	// check for base stations
	if (base_stations_count < 2) {
		printf("%s base station: please double-check\n", base_stations_count? "Only one" : "No");
		return false;
	}
	return true;
}

// debug, errors

const char *GetCompositorError(int err) {
	return
		err == VRCompositorError_None?							"None" :
		err == VRCompositorError_RequestFailed?					"RequestFailed" :
		err == VRCompositorError_IncompatibleVersion?			"IncompatibleVersion" :
		err == VRCompositorError_DoNotHaveFocus?				"DoNotHaveFocus" :
		err == VRCompositorError_InvalidTexture?				"InvalidTexture" :
		err == VRCompositorError_IsNotSceneApplication?			"IsNotSceneApplication" :
		err == VRCompositorError_TextureIsOnWrongDevice?		"TextureIsOnWrongDevice" :
		err == VRCompositorError_TextureUsesUnsupportedFormat?	"TextureUsesUnsupportedFormat" :
		err == VRCompositorError_SharedTexturesNotSupported?	"SharedTexturesNotSupported" :
		err == VRCompositorError_IndexOutOfRange?				"IndexOutOfRange" :
		err == VRCompositorError_AlreadySubmitted?				"AlreadySubmitted" :
		err == VRCompositorError_InvalidBounds?					"InvalidBounds" :
		err == VRCompositorError_AlreadySet?					"AlreadySet" :
																"Unknown";
}

string TrackingResultName(int r) {
	return string(r==1? "uninitialized" : r==100? "calib in progress" : r==101? "calib out of range" :
				  r==200? "running ok" : r==201? "running out of range" : r==300? "fallback rotation only" : "unknown");
}

void PrintMatrix(HmdMatrix34_t m, int pose) {
	printf("matrix for pose %i:\n", pose);
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 4; col++)
			printf("%3.2f  %3.2f  %3.2f  %3.2f\n", m.m[row][0], m.m[row][1], m.m[row][2], m.m[row][3]);
}

// get HMD pose

bool GetHMD(mat4 &hmd, bool print = false) {
	TrackedDevicePose_t pRenderPoseArray[k_unMaxTrackedDeviceCount], pGamePoseArray[k_unMaxTrackedDeviceCount];
	EVRCompositorError err = VRCompositor()->WaitGetPoses(pRenderPoseArray, k_unMaxTrackedDeviceCount, pGamePoseArray, k_unMaxTrackedDeviceCount);
	if (err)
		printf("VRCompositor:WaitGetPoses: %s\n", GetCompositorError(err));
	else
		for (int i = 0; i < k_unMaxTrackedDeviceCount; i++) {
			TrackedDevicePose_t pose = pGamePoseArray[i];
			if (print) {
				string trackResult = TrackingResultName(pose.eTrackingResult);
				printf("pose device %i: pose is %svalid, device is %sconnected, tracking result is %s\n",
					i, pose.bPoseIsValid? "" : "not ", pose.bDeviceIsConnected? "" : "not ", trackResult.c_str());
			}
			if (pose.bPoseIsValid && pose.bDeviceIsConnected) {
				HmdMatrix34_t m = pose.mDeviceToAbsoluteTracking;
				if (print) PrintMatrix(m, i);
				hmd = mat4(vec4(m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3]),
						   vec4(m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3]),
						   vec4(m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3]),
						   vec4(0, 0, 0, 1));
				return true;
			}
		}
	return false;
}

// initialize/update application transform

bool VROOM::StartTransformHMD(mat4 *m) {
	// calibrate raw HMD transform, set m as target (ie, application) HMD transform
	appTransform = m;
	appTransformStart = *m;
	mat4 hmd;
	if (VRCompositor() && GetHMD(hmd)) {
		qStart = Quaternion(hmd);
		qStartConjugate = Quaternion(-qStart.x, -qStart.y, -qStart.z, qStart.w);
		pStart = vec3(hmd[0][3], hmd[1][3], hmd[2][3]);
		return true;
	}
	return false;
}

void VROOM::UpdateTransformHMD() {
	mat4 r;
	if (GetHMD(r)) {
		Quaternion qNow(r), qDif = qNow*qStartConjugate;
		vec3 pNow(r[0][3], r[1][3], r[2][3]), pDif = translationScale*(pNow-pStart);
		// build matrix that is difference between start and present HMD poses
		mat4 mDif = qDif.GetMatrix();
		mDif[0][3] = pDif.x; mDif[1][3] = pDif.y; mDif[2][3] = pDif.z;
		*appTransform = mDif*(appTransformStart);
	}
}

// transfer images to HMD

void VROOM::SubmitOpenGLFrames(GLuint leftTextureUnit, GLuint rightTextureUnit) {
	Texture_t leftEyeTexture = {(void *) leftTextureUnit, TextureType_OpenGL, ColorSpace_Auto}; // Linear};
	Texture_t rightEyeTexture = {(void *) rightTextureUnit, TextureType_OpenGL, ColorSpace_Auto}; // Linear};
	EVRCompositorError err = VRCompositorError_None;
	// glBindTexture(GL_TEXTURE_2D, leftTextureUnit); // ?
	err = VRCompositor()->Submit(Eye_Left, &leftEyeTexture);
	if (err) printf("VRCompositor:Submit(left): %s\n", GetCompositorError(err));
	// glBindTexture(GL_TEXTURE_2D, rightTextureUnit); // ?
	err = VRCompositor()->Submit(Eye_Right, &rightEyeTexture);
	if (err) printf("VRCompositor:Submit(right): %s\n", GetCompositorError(err));
	VRCompositor()->PostPresentHandoff();
}

bool multisample = false; // *** fails ***

bool VROOM::InitFrameBuffer(int w, int h) {
	width = w;
	height = h;
	pixels = (float *) new float[4*width*height]; // deleted in destructor
	// make new frame buffer of texture and depth buffer
	glGenFramebuffers(1, &framebuffer);
	glGenTextures(1, &framebufferTextureName);
	glGenRenderbuffers(1, &depthBuffer);
	// setup texture
	glActiveTexture(GL_TEXTURE0+framebufferTextureName);
	if (multisample) {
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferTextureName);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, width, height, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, framebufferTextureName);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	// depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	if (multisample)
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	// configure frame buffer with depth buffer and color attachment
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	if (multisample)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferTextureName, 0);
	else
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebufferTextureName, 0);
	// enable drawing
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("bad frame buffer status\n");
	return framebuffer > 0 && depthBuffer > 0;
}

void VROOM::CopyFramebufferToEyeTexture(GLuint textureName, GLuint textureUnit) {
	// read from framebuffer
	glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, pixels);
	// store pixels as GL texture
	glActiveTexture(GL_TEXTURE0+textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureName); // bind active texture to textureName
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);     // accommodate width not multiple of 4
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, pixels); // ?
}

// following of no value?

const char *ButtonName(const VREvent_t &e) {
	uint32_t t = e.eventType;
	if (t==VREvent_ButtonPress || t==VREvent_ButtonUnpress || t==VREvent_ButtonTouch || t==VREvent_ButtonUntouch) {
		VREvent_Controller_t controller_data = e.data.controller;
		return vr_context->GetButtonIdNameFromEnum((EVRButtonId) controller_data.button);
	}
	return NULL;
}

/* void VROOM::ProcessEvent(const VREvent_t &e) {
	switch(e.eventType) {
		case VREvent_TrackedDeviceActivated:
			printf("Event: device %i activated\n", e.trackedDeviceIndex);
			break;
		case VREvent_TrackedDeviceDeactivated:
			printf("Event: device %i deactivated\n", e.trackedDeviceIndex);
			break;
		case VREvent_TrackedDeviceUpdated:
			printf("Event: device %i updated\n", e.trackedDeviceIndex);
			break;
		case VREvent_ButtonPress:
			printf("Event: device %i, %s button pressed\n", e.trackedDeviceIndex, ButtonName(e));
			break;
		case VREvent_ButtonUnpress:
			printf("Event: device %i, %s button un-pressed\n", e.trackedDeviceIndex, ButtonName(e));
			break;
		case VREvent_ButtonTouch:
			printf("Event: device %i, %s button touched\n", e.trackedDeviceIndex, ButtonName(e));
			break;
		case VREvent_ButtonUntouch:
			printf("Event: device %i, %s button un-touched\n", e.trackedDeviceIndex, ButtonName(e));
			break;
	}
} */
