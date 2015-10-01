// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <string>
#include <GLFW/glfw3.h>

#include "Common/GL/GLInterfaceBase.h"

class cInterfaceGLFW : public cInterfaceBase
{
private:
	GLFWwindow *window = nullptr;

public:
	void SwapInterval(int Interval);
	void Swap();
	void SetMode(u32 mode) { s_opengl_mode = mode; }
	void* GetFuncAddress(const std::string& name);
	bool Create(void *window_handle, bool core);
	bool MakeCurrent();
	bool ClearCurrent();
	void Shutdown();

	// XXX
	GLFWwindow* GetWindow();
};
