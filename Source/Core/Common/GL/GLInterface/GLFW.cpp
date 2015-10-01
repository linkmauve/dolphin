// Copyright 2012 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <array>
#include <cstdlib>

#include "Common/GL/GLInterface/GLFW.h"
#include "Common/Logging/Log.h"

GLFWwindow* cInterfaceGLFW::GetWindow()
{
	return window;
}

// Show the current FPS
void cInterfaceGLFW::Swap()
{
	glfwSwapBuffers(window);
}
void cInterfaceGLFW::SwapInterval(int Interval)
{
	glfwSwapInterval(Interval);
}

void* cInterfaceGLFW::GetFuncAddress(const std::string& name)
{
	return (void*)glfwGetProcAddress(name.c_str());
}

// Create rendering window.
// Call browser: Core.cpp:EmuThread() > main.cpp:Video_Initialize()
bool cInterfaceGLFW::Create(void *window_handle, bool core)
{
	// attributes for a visual in RGBA format with at least
	// 8 bits per color
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);

	if (s_opengl_mode == MODE_DETECT)
		s_opengl_mode = MODE_OPENGL;

	switch (s_opengl_mode)
	{
		case MODE_OPENGL:
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		break;
		case MODE_OPENGLES2:
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		break;
		case MODE_OPENGLES3:
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		break;
		default:
			ERROR_LOG(VIDEO, "Unknown opengl mode set\n");
			return false;
		break;
	}

	window = glfwCreateWindow(640, 528, "Dolphin!", nullptr, nullptr);
	if (!window)
	{
		INFO_LOG(VIDEO, "Error: glfwCreateWindow failed\n");
		return false;
	}

	//s = eglQueryString(egl_dpy, EGL_VERSION);
	//INFO_LOG(VIDEO, "EGL_VERSION = %s\n", s);

	//s = eglQueryString(egl_dpy, EGL_VENDOR);
	//INFO_LOG(VIDEO, "EGL_VENDOR = %s\n", s);

	//s = eglQueryString(egl_dpy, EGL_EXTENSIONS);
	//INFO_LOG(VIDEO, "EGL_EXTENSIONS = %s\n", s);

	//s = eglQueryString(egl_dpy, EGL_CLIENT_APIS);
	//INFO_LOG(VIDEO, "EGL_CLIENT_APIS = %s\n", s);

	return true;
}

bool cInterfaceGLFW::MakeCurrent()
{
	glfwMakeContextCurrent(window);
	return true;
}

bool cInterfaceGLFW::ClearCurrent()
{
	glfwMakeContextCurrent(nullptr);
	return true;
}

// Close backend
void cInterfaceGLFW::Shutdown()
{
	glfwDestroyWindow(window);
}
