// Copyright 2015 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#if HAVE_GLFW
#include <unistd.h>
#include <GLFW/glfw3.h>

#include "Common/MsgHandler.h"

#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/State.h"

#include "DolphinNoGUI/GLFW.h"

static void OnError(int error, const char *description)
{
	fprintf(stderr, "GLFW: 0x%08x: %s\n", error, description);
}

void PlatformGLFW::Init()
{
	glfwSetErrorCallback(OnError);

	if (!glfwInit())
	{
		PanicAlert("GLFW initialization failed");
		exit(1);
	}

	// We want to manage our context ourself.
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(SConfig::GetInstance().iRenderWindowWidth,
	                          SConfig::GetInstance().iRenderWindowHeight,
	                          "Dolphin", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		PanicAlert("GLFW window creation failed");
		exit(2);
	}
}

void* PlatformGLFW::GetRenderHandle()
{
	return (void*)window;
}

void PlatformGLFW::SetTitle(const std::string &string)
{
	glfwSetWindowTitle(window, string.c_str());
}

void PlatformGLFW::MainLoop()
{
	// The actual loop
	while (m_Running)
	{
		usleep(100000);
	}
}

void PlatformGLFW::Shutdown()
{
	if (window)
		glfwDestroyWindow(window);
	glfwTerminate();
}
#endif
