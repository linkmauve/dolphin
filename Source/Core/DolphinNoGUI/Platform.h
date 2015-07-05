// Copyright 2015 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <string>

class Platform
{
protected:
	bool m_RendererHasFocus = true;
	bool m_RendererIsFullscreen = false;

public:
	bool m_Running = true;

	virtual void Init() = 0;
	virtual void* GetRenderHandle() = 0;
	virtual void SetTitle(const std::string &title) = 0;
	virtual void MainLoop() = 0;
	virtual void Shutdown() = 0;
	virtual ~Platform() {};

	bool RendererHasFocus()
	{
		return m_RendererHasFocus;
	};

	bool RendererIsFullscreen()
	{
		return m_RendererIsFullscreen;
	};
};
