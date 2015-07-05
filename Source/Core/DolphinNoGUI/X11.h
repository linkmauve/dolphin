// Copyright 2015 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#if HAVE_X11
#include <string>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "DolphinNoGUI/Platform.h"

namespace X11Utils
{
	class XRRConfiguration;
}

class PlatformX11 : public Platform
{
private:
	Display *dpy;
	Window win;
	Cursor blankCursor = None;
#if defined(HAVE_XRANDR) && HAVE_XRANDR
	X11Utils::XRRConfiguration *XRRConfig;
#endif

public:
	void Init() override;
	void* GetRenderHandle() override;
	void SetTitle(const std::string &string) override;
	void MainLoop() override;
	void Shutdown() override;
};
#endif
