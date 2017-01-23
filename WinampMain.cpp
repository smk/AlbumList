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
#include <commctrl.h>
#include <Shlwapi.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "WinampMain.h"

//////////////////////////////////////////////////////////////////
// CWinamp class
CWinamp::CWinamp()
{
	m_bKeepSongPlaying	= FALSE;
	m_bQuitting			= FALSE;
}

CWinamp::~CWinamp()
{
}

LRESULT CWinamp::SendIPCMessage(WPARAM wParam /*=0*/, LPARAM lParam /*=0*/)
{
	return SendMessage(WM_WA_IPC, wParam, lParam);
}

LRESULT CWinamp::SendMLMessage(WPARAM wParam /*=0*/, LPARAM lParam /*=0*/)
{
	// get library get wnd message id
	long libhwndipc = (LONG)SendIPCMessage(WPARAM("LibraryGetWnd"), IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (libhwndipc)
	{
		// now send the message and get the HWND 
		CWnd wndML = (HWND)SendIPCMessage(-1, LPARAM(libhwndipc));
		if (wndML.IsWindow())
		{
			// add to the tree
			return wndML.SendMessage(WM_ML_IPC, wParam, lParam);
		}
	}
	return 0;
}

LRESULT CWinamp::PostIPCMessage(WPARAM wParam /*=0*/, LPARAM lParam /*=0*/)
{
	return PostMessage(WM_WA_IPC, wParam, lParam);
}

BOOL CWinamp::OnCommand(WORD wNotifyCode, WORD wID, CWnd wndCtl)
{
	return wndWinampAL.ProcessWinampMenu(wID);
}

BOOL CWinamp::OnSysCommand(UINT uCmdType, short xPos, short yPos)
{
	return wndWinampAL.ProcessWinampMenu(uCmdType);
}

BOOL CWinamp::OnClose()
{
	wndWinampAL.Shutdown();

	return FALSE;
}

void CWinamp::KeepSongPlaying()
{
	m_bKeepSongPlaying = TRUE;
}

void CWinamp::OnSongChange()
{
	m_bKeepSongPlaying = FALSE;

	int pos = wndWinamp.SendIPCMessage(0,IPC_GETLISTPOS);
	LPCTSTR filename = (LPCTSTR)wndWinamp.SendIPCMessage(pos, IPC_GETPLAYLISTFILE);
	wndWinampAL.UpdateMSN((LPCTSTR)filename);
}

BOOL CWinamp::IsQuitting()
{
	return m_bQuitting;
}

LRESULT CWinamp::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// keep song playing
	if ((message == WM_WA_MPEG_EOF) ||
		((message == WM_KEYDOWN) && (wParam == 0x42 || wParam == VK_NUMPAD6)) ||
		((message == WM_COMMAND) && (LOWORD(wParam) == WINAMP_BUTTON5)))
	{
		if (m_bKeepSongPlaying)
		{
			m_bKeepSongPlaying = FALSE;

			CallWindowProc(pWindowProc, m_hWnd, message, wParam, lParam);

			SendIPCMessage(0, IPC_SETPLAYLISTPOS);
			SendMessage(WM_COMMAND, WINAMP_BUTTON2, 0);

			return TRUE;
		}
	}

	// end-of-song callback
	if (message == WM_WA_MPEG_EOF)
	{
		wndWinampAL.PostMessage(WM_WA_MPEG_EOF);
	}

	// user pressed stop button
	else if ((message == WM_COMMAND) && (LOWORD(wParam) == WINAMP_BUTTON4))
	{
		wndWinampAL.StopPressed();
	}

	// callbacks
	else if (message == WM_WA_IPC)
	{
		if (lParam == IPC_HOOK_OKTOQUIT)
		{
			m_bQuitting = TRUE;
			if (!wndWinampAL.OkToQuit())
				return 0;
		}
		else if (lParam == IPC_CB_RESETFONT)
		{
			wndWinampAL.ResetFont();
		}
		else if (lParam == IPC_AL_ADDMENU)
		{
			addStruct *a = (addStruct*)wParam;

			if (a->flags == ADD_TYPE_MENU)
			{
				return wndWinampAL.AddIPCMenu(a);
			}
		}
		else if (lParam == IPC_PLAYING_FILE)
		{
			wndWinampAL.UpdateMSN((LPCTSTR)wParam);
		}
	}

	// overriding the lightning bolt
	else if ((message == WM_COMMAND) && (LOWORD(wParam) == 40339))
	{
		if (wndWinampAL.GetSetting(settingLightningBolt))
		{
			wndWinampAL.SendMessage(WM_AL_IPC, IPC_SHOWHIDE_CUR, 0);
			return TRUE;
		}
	}

	// menu
	else if (message == WM_INITMENUPOPUP)
	{
		if (GetMenuItemID((HMENU)wParam, 0) == WINAMP_HELP_ABOUT/*40041*/)
		{
			// remove our menu
			wndWinampAL.RemoveFromMainMenu((HMENU)wParam);

			LRESULT lResult = CWnd::DefWindowProc(message, wParam, lParam);

			// add our menu
			wndWinampAL.InsertToMainMenu((HMENU)wParam);

			return lResult;
		}
	}

	return CWnd::DefWindowProc(message, wParam, lParam);
}
