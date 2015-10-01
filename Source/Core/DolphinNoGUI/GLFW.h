// Copyright 2015 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#if HAVE_GLFW
#include <GLFW/glfw3.h>

#include "DolphinNoGUI/Platform.h"

class PlatformGLFW : public Platform
{
private:
	GLFWwindow *window = nullptr;

public:
	void Init() override;
	void* GetRenderHandle() override;
	void SetTitle(const std::string &string) override;
	void MainLoop() override;
	void Shutdown() override;
};
#endif
