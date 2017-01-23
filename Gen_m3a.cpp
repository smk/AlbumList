//	Album List for Winamp
//	Copyright (C) 1999-2006 Safai Ma
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include <Shlwapi.h>
#include <ole2.h>
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#include "resource.h"

#define WA_DLG_IMPLEMENT

#include "Gen_m3a.h"
#include "ConfigMain.h"
#include "WinampMain.h"
#include "WinampPE.h"
#include "WinampAL.h"
#include "CoolScroll.h"
#include "CoolSB_detours.h"
#include "APIHijack.h"
#include "ALFront.h"

/////////////////////////////////////////////////////////
// global variables
HINSTANCE hDllInstance = NULL;

/////////////////////////////////////////////////////////
// General Purpose Plugin

void configGP();
void quitGP();
int initGP();

winampGeneralPurposePlugin pluginGP =
{
	GPPHDR_VER,
	"",
	initGP,
	configGP,
	quitGP,
};

char szAppName[64] = "Album List";
char dirPlugin[MAX_PATH];
char iniPlugin[MAX_PATH];

char hotkeyname[7][MAX_PATH] =
{
	"AL: Show/Hide Album List",
	"AL: Jump to album",
	"AL: Play previous album",				
	"AL: Play next album",					
	"AL: Play previous album (different artist)",
	"AL: Play next album (different artist)",	
	"AL: Play random album"
};

genHotkeysAddStruct genHotKeys[] =
{
//	  name										flags	uMsg		wParam					   lParam  id
	{ "AL: Show/Hide Album List",					0,	WM_AL_IPC,	IPC_SHOWHIDE_CUR,				0, "albumlist_sh"	 },
	{ "AL: Jump to album",							0,	WM_AL_IPC,	IPC_JUMPTOALBUM_CUR,			0, "albumlist_jta"	 },
	{ "AL: Play previous album",					0,	WM_AL_IPC,	IPC_PLAYPREVALBUM_CUR,			0, "albumlist_ppa"	 },
	{ "AL: Play next album",						0,	WM_AL_IPC,	IPC_PLAYNEXTALBUM_CUR,			0, "albumlist_pna"	 },
	{ "AL: Play previous album (different artist)",	0,	WM_AL_IPC,	IPC_PLAYPREVALBUMARTIST_CUR,	0, "albumlist_ppada" },
	{ "AL: Play next album (different artist)",		0,	WM_AL_IPC,	IPC_PLAYNEXTALBUMARTIST_CUR,	0, "albumlist_pnada" },
	{ "AL: Play random album",						0,	WM_AL_IPC,	IPC_PLAYRANDOMALBUM_CUR,		0, "albumlist_pra"	 }
};

int IPC_AL_ADDMENU = -1;

BOOL bInitialized = FALSE;

CWinamp wndWinamp;
CWinampPE wndWinampPE;
CWinampAL wndWinampAL;

void configGP()
{
	// winamp version check
	if (!wndWinamp.IsWinamp29())
	{
		MessageBox(pluginGP.hwndParent, ALS("This version requires Winamp 2.9 or above."), szAppName, MB_OK);
		return;
	}

	preference.ShowPage();
}

int initGP()
{
	hDllInstance = pluginGP.hDllInstance;

	// get windows version
	OSVERSIONINFO osi;
	osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osi);

	bWin9x = (osi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	bWinNT = (osi.dwPlatformId == VER_PLATFORM_WIN32_NT);

	SetThreadName(-1, "Main");
	DbgPrint("M3A Init\n");

	wndWinamp.m_hWnd = pluginGP.hwndParent;

	// winamp version check
	if (!wndWinamp.IsWinamp29())
	{
		MessageBox(wndWinamp, ALS("This version requires Winamp 2.9 or above."), szAppName, MB_OK);
		return 0;
	}

	// turn off error mode
	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

	// initialize CoolSB
	if (bWinNT) CoolSB_InitializeApp();

	// subclass winamp main window
	wndWinamp.SubclassWindow(pluginGP.hwndParent);

	// subclass winamp playlist editor
	HWND hwndPE = (HWND)wndWinamp.SendIPCMessage(IPC_GETWND_PE, IPC_GETWND);
	wndWinampPE.SubclassWindow(hwndPE);

	// create description string
	{
		static char c[64] = "Album List";
		int major = 2, minor = 0, build = 1;
		GetVersionInformation(pluginGP.hDllInstance, major, minor, build);

		wsprintf((pluginGP.description=c), "%s v%ld.%02ld", szAppName, major, minor);
	}

	// get plugin directory (c:\program files\winamp\plugins)
	GetModuleFileName(NULL, dirPlugin, MAX_PATH);
	PathRemoveFileSpec(dirPlugin);
	PathAppend(dirPlugin, "Plugins");

	// get gen_m3a.ini
	if (wndWinamp.IsWinamp511())
	{
		lstrcpyn(iniPlugin, (char*)wndWinamp.SendIPCMessage(0,IPC_GETINIFILE), MAX_PATH);
		PathRemoveFileSpec(iniPlugin);
		PathAppend(iniPlugin, "Plugins");
		// create the plugins directory (just in case)
		CreateDirectory(iniPlugin, NULL);
		PathAppend(iniPlugin, "Gen_m3a.ini");
	}
	else
	{
		lstrcpy(iniPlugin, dirPlugin);
		PathAppend(iniPlugin, "Gen_m3a.ini");
	}

	// Add preferences page
	preference.AddPages();

	// Create our UI
	wndWinampAL.Create();

	// Add Global Hotkey
	int genhotkeys_add_ipc = wndWinamp.SendIPCMessage((WPARAM)&"GenHotkeysAdd", IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (genhotkeys_add_ipc > 65536)
	{
		int size = sizeof(genHotKeys) / sizeof(genHotkeysAddStruct);
		for (int i=0; i<size; i++)
		{
			lstrcpyn(hotkeyname[i], (LPTSTR)ALS(genHotKeys[i].name), MAX_PATH);
			genHotKeys[i].name = hotkeyname[i];
			genHotKeys[i].wnd = wndWinampAL;
			wndWinamp.PostIPCMessage((WPARAM)&genHotKeys[i], genhotkeys_add_ipc);
		}
	}

	// get message id
	IPC_AL_ADDMENU = wndWinamp.SendIPCMessage((WPARAM)&"GenAlbumListAdd",IPC_REGISTER_WINAMP_IPCMESSAGE);

	CoInitialize(NULL);

	bInitialized = TRUE;

	return GEN_INIT_SUCCESS;
}

void quitGP()
{
	DbgPrint("M3A Quit\n");
	if (bInitialized)
	{
		wndWinampAL.Shutdown();

		// subclass winamp windows
		wndWinamp.UnsubclassWindow();
		wndWinampPE.UnsubclassWindow();

		// turn back on error mode
		SetErrorMode(0);

		::DestroyWindow(wndWinampAL);

		if (bWinNT) CoolSB_UninitializeApp();

		WADlg_close();

		CoUninitialize();
	}
}

/////////////////////////////////////////////////////////
// exports

extern "C" __declspec( dllexport ) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin()
{
	return &pluginGP;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

