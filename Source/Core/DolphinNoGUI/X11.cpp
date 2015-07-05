// Copyright 2015 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#if HAVE_X11
#include <cstdlib>
#include <unistd.h>
#include <X11/keysym.h>

#include "Common/MsgHandler.h"

#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/State.h"

#include "DolphinWX/X11Utils.h"

#include "DolphinNoGUI/X11.h"

void PlatformX11::Init()
{
	XInitThreads();
	dpy = XOpenDisplay(nullptr);
	if (!dpy)
	{
		PanicAlert("No X11 display found");
		exit(1);
	}

	win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
				  SConfig::GetInstance().iRenderWindowXPos,
				  SConfig::GetInstance().iRenderWindowYPos,
				  SConfig::GetInstance().iRenderWindowWidth,
				  SConfig::GetInstance().iRenderWindowHeight,
				  0, 0, BlackPixel(dpy, 0));
	XSelectInput(dpy, win, KeyPressMask | FocusChangeMask);
	Atom wmProtocols[1];
	wmProtocols[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(dpy, win, wmProtocols, 1);
	XMapRaised(dpy, win);
	XFlush(dpy);

	if (SConfig::GetInstance().bDisableScreenSaver)
		X11Utils::InhibitScreensaver(dpy, win, true);

#if defined(HAVE_XRANDR) && HAVE_XRANDR
	XRRConfig = new X11Utils::XRRConfiguration(dpy, win);
#endif

	if (SConfig::GetInstance().bHideCursor)
	{
		// make a blank cursor
		Pixmap Blank;
		XColor DummyColor;
		char ZeroData[1] = { 0 };
		Blank = XCreateBitmapFromData(dpy, win, ZeroData, 1, 1);
		blankCursor = XCreatePixmapCursor(dpy, Blank, Blank, &DummyColor, &DummyColor, 0, 0);
		XFreePixmap(dpy, Blank);
		XDefineCursor(dpy, win, blankCursor);
	}
}

void* PlatformX11::GetRenderHandle()
{
	return (void*)win;
}

void PlatformX11::SetTitle(const std::string &string)
{
	XStoreName(dpy, win, string.c_str());
}

void PlatformX11::MainLoop()
{
	bool fullscreen = SConfig::GetInstance().bFullscreen;

	if (fullscreen)
	{
		m_RendererIsFullscreen = X11Utils::ToggleFullscreen(dpy, win);
#if defined(HAVE_XRANDR) && HAVE_XRANDR
		XRRConfig->ToggleDisplayMode(True);
#endif
	}

	// The actual loop
	while (m_Running)
	{
		XEvent event;
		KeySym key;
		for (int num_events = XPending(dpy); num_events > 0; num_events--)
		{
			XNextEvent(dpy, &event);
			switch (event.type)
			{
			case KeyPress:
				key = XLookupKeysym((XKeyEvent*)&event, 0);
				if (key == XK_Escape)
				{
					if (Core::GetState() == Core::CORE_RUN)
					{
						if (SConfig::GetInstance().bHideCursor)
							XUndefineCursor(dpy, win);
						Core::SetState(Core::CORE_PAUSE);
					}
					else
					{
						if (SConfig::GetInstance().bHideCursor)
							XDefineCursor(dpy, win, blankCursor);
						Core::SetState(Core::CORE_RUN);
					}
				}
				else if ((key == XK_Return) && (event.xkey.state & Mod1Mask))
				{
					fullscreen = !fullscreen;
					X11Utils::ToggleFullscreen(dpy, win);
#if defined(HAVE_XRANDR) && HAVE_XRANDR
					XRRConfig->ToggleDisplayMode(fullscreen);
#endif
				}
				else if (key >= XK_F1 && key <= XK_F8)
				{
					int slot_number = key - XK_F1 + 1;
					if (event.xkey.state & ShiftMask)
						State::Save(slot_number);
					else
						State::Load(slot_number);
				}
				else if (key == XK_F9)
					Core::SaveScreenShot();
				else if (key == XK_F11)
					State::LoadLastSaved();
				else if (key == XK_F12)
				{
					if (event.xkey.state & ShiftMask)
						State::UndoLoadState();
					else
						State::UndoSaveState();
				}
				break;
			case FocusIn:
				m_RendererHasFocus = true;
				if (SConfig::GetInstance().bHideCursor &&
				    Core::GetState() != Core::CORE_PAUSE)
					XDefineCursor(dpy, win, blankCursor);
				break;
			case FocusOut:
				m_RendererHasFocus = false;
				if (SConfig::GetInstance().bHideCursor)
					XUndefineCursor(dpy, win);
				break;
			case ClientMessage:
				if ((unsigned long) event.xclient.data.l[0] == XInternAtom(dpy, "WM_DELETE_WINDOW", False))
					m_Running = false;
				break;
			}
		}
		if (!fullscreen)
		{
			Window winDummy;
			unsigned int borderDummy, depthDummy;
			XGetGeometry(dpy, win, &winDummy,
				     &SConfig::GetInstance().iRenderWindowXPos,
				     &SConfig::GetInstance().iRenderWindowYPos,
				     (unsigned int *)&SConfig::GetInstance().iRenderWindowWidth,
				     (unsigned int *)&SConfig::GetInstance().iRenderWindowHeight,
				     &borderDummy, &depthDummy);
			m_RendererIsFullscreen = false;
		}
		usleep(100000);
	}
}

void PlatformX11::Shutdown()
{
#if defined(HAVE_XRANDR) && HAVE_XRANDR
	delete XRRConfig;
#endif

	if (SConfig::GetInstance().bHideCursor)
		XFreeCursor(dpy, blankCursor);

	XCloseDisplay(dpy);
}
#endif
