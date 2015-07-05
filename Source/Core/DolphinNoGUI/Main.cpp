// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cstdio>
#include <getopt.h>
#include <string>
#include <unistd.h>

#include "Common/Common.h"
#include "Common/Event.h"

#include "Core/BootManager.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/Host.h"
#include "Core/IPC_HLE/WII_IPC_HLE_Device_usb.h"
#include "Core/IPC_HLE/WII_IPC_HLE_WiiMote.h"
#include "Core/PowerPC/PowerPC.h"

#include "UICommon/UICommon.h"

#include "DolphinNoGUI/Platform.h"

static Platform* platform;

void Host_NotifyMapLoaded() {}
void Host_RefreshDSPDebuggerWindow() {}

static Common::Event updateMainFrameEvent;
void Host_Message(int Id)
{
	if (Id == WM_USER_STOP)
		platform->m_Running = false;
}

void* Host_GetRenderHandle()
{
	return platform->GetRenderHandle();
}

void Host_UpdateTitle(const std::string& title)
{
	platform->SetTitle(title);
}

void Host_UpdateDisasmDialog(){}

void Host_UpdateMainFrame()
{
	updateMainFrameEvent.Set();
}

void Host_RequestRenderWindowSize(int width, int height) {}

void Host_RequestFullscreen(bool enable_fullscreen) {}

void Host_SetStartupDebuggingParameters()
{
	SConfig& StartUp = SConfig::GetInstance();
	StartUp.bEnableDebugging = false;
	StartUp.bBootToPause = false;
}

bool Host_UIHasFocus()
{
	return false;
}

bool Host_RendererHasFocus()
{
	return platform->RendererHasFocus();
}

bool Host_RendererIsFullscreen()
{
	return platform->RendererIsFullscreen();
}

void Host_ConnectWiimote(int wm_idx, bool connect)
{
	if (Core::IsRunning() && SConfig::GetInstance().bWii)
	{
		bool was_unpaused = Core::PauseAndLock(true);
		GetUsbPointer()->AccessWiiMote(wm_idx | 0x100)->Activate(connect);
		Host_UpdateMainFrame();
		Core::PauseAndLock(false, was_unpaused);
	}
}

void Host_SetWiiMoteConnectionState(int _State) {}

void Host_ShowVideoConfig(void*, const std::string&, const std::string&) {}

#if HAVE_X11
#include "DolphinNoGUI/X11.h"
#endif

static Platform* GetPlatform()
{
#if HAVE_X11
	return new PlatformX11();
#endif
	return nullptr;
}

int main(int argc, char* argv[])
{
	int ch, help = 0;
	struct option longopts[] = {
		{ "exec",    no_argument, nullptr, 'e' },
		{ "help",    no_argument, nullptr, 'h' },
		{ "version", no_argument, nullptr, 'v' },
		{ nullptr,      0,           nullptr,  0  }
	};

	while ((ch = getopt_long(argc, argv, "eh?v", longopts, 0)) != -1)
	{
		switch (ch)
		{
		case 'e':
			break;
		case 'h':
		case '?':
			help = 1;
			break;
		case 'v':
			fprintf(stderr, "%s\n", scm_rev_str);
			return 1;
		}
	}

	if (help == 1 || argc == optind)
	{
		fprintf(stderr, "%s\n\n", scm_rev_str);
		fprintf(stderr, "A multi-platform GameCube/Wii emulator\n\n");
		fprintf(stderr, "Usage: %s [-e <file>] [-h] [-v]\n", argv[0]);
		fprintf(stderr, "  -e, --exec     Load the specified file\n");
		fprintf(stderr, "  -h, --help     Show this help message\n");
		fprintf(stderr, "  -v, --version  Print version and exit\n");
		return 1;
	}

	platform = GetPlatform();
	if (!platform)
	{
		fprintf(stderr, "No platform found\n");
		return 1;
	}

	UICommon::SetUserDirectory(""); // Auto-detect user folder
	UICommon::Init();

	platform->Init();

	if (!BootManager::BootCore(argv[optind]))
	{
		fprintf(stderr, "Could not boot %s\n", argv[optind]);
		return 1;
	}

	while (!Core::IsRunning())
		updateMainFrameEvent.Wait();

	platform->MainLoop();
	Core::Stop();
	while (PowerPC::GetState() != PowerPC::CPU_POWERDOWN)
		updateMainFrameEvent.Wait();

	Core::Shutdown();
	platform->Shutdown();
	UICommon::Shutdown();

	delete platform;

	return 0;
}
