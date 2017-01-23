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
#include <stdlib.h>
#include <commctrl.h>
#include <time.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "ConfigMain.h"
#include "Util.h"
#include "UserInterface.h"
#include "WinampAL.h"
#include "ALFront.h"
#include "ML.h"
#include "FileInfo.h"
#include "ConfigAboutML.h"
#include "ProfileName.h"

#include "APIHijack.h"
#include <UrlMon.h>

// function prototype from URLMon.cpp
BOOL URLMonLoad();
void URLMonFree();

// timer id
#define TIMERID_ENDOFSONG		0xbebe
#define TIMERID_RESUMEPLAYBACK	0xbeef

//////////////////////////////////////////////////////////////////
// CWinampAL class

CWinampAL::CWinampAL()
{
	m_bInitialized		= FALSE;
	m_bResumePlayback	= FALSE;
	m_nResumeTime		= -1;
	m_nResumePos		= 0;
	m_nResumePaused		= 0;
	memset(m_szResumeFile, 0, MAX_PATH);

	m_bKeepSongPlaying	= FALSE;
	m_bSortByFilename	= FALSE;
	m_bEnqueueDefault	= FALSE;
	m_EncodingLangAL	= langSystemDefault;
	m_EncodingLangPE	= langSystemDefault;
	m_nLastConfig		= 0;
	m_bLoop				= TRUE;
	m_bShowCover		= FALSE;
	m_bAutoAdvance		= FALSE;
	m_bAdvanceRandom	= FALSE;
	m_bCompression		= TRUE;
	m_bVersionCheck		= FALSE;
	m_tCheckTime		= 0;
	m_dwLatestVer		= 0;
	m_bLogFile			= FALSE;
	m_bBuiltInFileInfo	= TRUE;
	m_bWinampFileInfo	= TRUE;
	m_bFreeFormMP3		= FALSE;
	m_bLightningBolt	= FALSE;
	m_nVariousArtistRatio	= 70;
	m_bEmbedML			= FALSE;
	m_bEmbedMLSave		= FALSE;
	m_bPlayAllWarning	= FALSE;
	m_nPlayAllLimit		= 10;
	m_bUpdateMSN		= FALSE;

	m_nAutoAdvanceID	= -1;
	m_pAutoAdvanceUI	= NULL;
	m_pFocusUI			= NULL;
	m_nFocusID			= -1;
	
	m_hThread			= NULL;
	m_dwThreadId		= 0;

	m_bFirstTimer		= FALSE;

	m_nMLTreeItem		= -1;
	m_bTabbedUI			= FALSE;
	m_bTabbedUISave		= FALSE;

	memset(m_szTranslation, 0, MAX_PATH);
	m_nTranslationCodepage	= CP_ACP;
}

CWinampAL::~CWinampAL()
{
	m_pAutoAdvanceUI	= NULL;
	m_nAutoAdvanceID	= -1;
	m_pFocusUI			= NULL;
	m_nFocusID			= -1;

	// remove all user interfaces (profiles)
	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);
		if (pUI)
		{
			delete pUI;
		}
	}
	m_UserInterfaces.RemoveAll();

	// remove all the IPC Menus
	size = m_IPCMenuList.GetSize();
	for (i=0; i<size; ++i)
	{
		CIPCMenu *m = (CIPCMenu *)m_IPCMenuList.GetAt(i);
		if (m) delete m;
	}
	m_IPCMenuList.RemoveAll();
}

BOOL CWinampAL::Create()
{
	WNDCLASSEX wndclassex;
	wndclassex.cbSize = sizeof(wndclassex);
	wndclassex.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
	wndclassex.lpfnWndProc = WindowProc;
	wndclassex.lpszClassName = WINAMP_AL_CLASS;
	wndclassex.cbClsExtra = 0;
	wndclassex.cbWndExtra = 0;
	wndclassex.hInstance = hDllInstance;
	wndclassex.hIcon = NULL;
	wndclassex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndclassex.lpszMenuName = NULL;
	wndclassex.hIconSm = NULL;
	wndclassex.hCursor = NULL;

	if (!RegisterClassEx(&wndclassex))
		return FALSE;

    // Create the main window. 
    m_hWnd = CreateWindowEx(0, WINAMP_AL_CLASS, WINAMP_AL_CLASS, WS_POPUPWINDOW, 0, 0, 10, 10, NULL, NULL, hDllInstance, (LPVOID) this);
	if (!m_hWnd) 
		return FALSE;

	ShowWindow (SW_HIDE);

	return TRUE;
}

// windows messages

int CWinampAL::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	Startup();

	return 0;
}

BOOL CWinampAL::OnDestroy()
{
	return FALSE;
}

BOOL CWinampAL::OnEndSession(BOOL bEnding)
{
	if (bEnding)
	{
		Shutdown();
	}
	return FALSE;
}

BOOL CWinampAL::OnQueryEndSession()
{
	// save our settings
	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);
		if (pUI) pUI->Archive(TRUE /*Write*/);
	}

	Archive(TRUE /*Write*/);

	return TRUE;
}

LRESULT CWinampAL::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// AL front end messages
	if (message == WM_AL_IPC)
	{
		return OnFrontEnd(wParam, lParam);
	}

	// end of song notification
	else if (message == WM_WA_MPEG_EOF)
	{
		// we also get this with the Playlist Separator
		// so wait a second more before proceeding
		// to make sure we're really stopped!
		SetTimer(m_hWnd, TIMERID_ENDOFSONG, 1000, NULL);
		return TRUE;
	}

	// timer messages
	else if (message == WM_TIMER)
	{
		KillTimer(m_hWnd, wParam);

		// end of song timer
		// do our regular end of song processing
		if (wParam == TIMERID_ENDOFSONG)
		{
			return OnEndOfSong();
		}
		else if (wParam == TIMERID_RESUMEPLAYBACK)
		{
			if (lstrcmpi(m_szResumeFile, (LPCTSTR)wndWinamp.SendIPCMessage(m_nResumePos, IPC_GETPLAYLISTFILE)) == 0)
			{
				wndWinamp.SendIPCMessage(m_nResumePos, IPC_SETPLAYLISTPOS);

				// resume play only if it was playing before
				if (m_nResumeTime != -1)
				{
					// IPC_STARTPLAY does not work here, it always plays
					// the first song in the playlist :(
					wndWinamp.SendMessage(WM_COMMAND, WINAMP_BUTTON2, 0);
					wndWinamp.SendIPCMessage(m_nResumeTime, IPC_JUMPTOTIME);

					// pause playback
					if (m_nResumePaused)
					{
						wndWinamp.SendMessage(WM_COMMAND, WINAMP_BUTTON3, 0);
					}
				}
			}
			return TRUE;
		}
	}
	else if (message == WM_AL_POSTINIT)
	{
		return OnPostInit();
	}

	return CWnd::DefWindowProc(message, wParam, lParam);
}

// private implementation

BOOL CWinampAL::Startup()
{
	Archive(FALSE /*read*/);

	// Post Init
	PostMessage(WM_AL_POSTINIT);

	m_bInitialized = TRUE;

	return TRUE;
}

BOOL CWinampAL::InitProfiles()
{
	// save a copy first
	// m_nAutoAdvanceID & m_nFocusID will get changed in "AddUserInterface"
	int nAutoAdvanceID = m_nAutoAdvanceID;
	CUserInterface *pAutoAdvanceUI = NULL;

	int nFocusID = m_nFocusID;
	CUserInterface *pFocusUI = NULL;

	char sections[512];
	if (GetPrivateProfileSectionNames(sections, 512, iniPlugin) < 512)
	{
		char *ptr = sections;
		while (lstrlen(ptr))
		{
			if (_strnicmp(ptr, "Profile", 7) == 0)
			{
				int nID = atoi(ptr+7);

				CUserInterface *pUI = AddUserInterface(nID);

				if (nID == nAutoAdvanceID)
				{
					pAutoAdvanceUI = pUI;
				}

				if (nID == nFocusID)
				{
					pFocusUI = pUI;
				}
			}
			ptr += lstrlen(ptr) + 1;
		}
	}

	if (m_bFirstTimer && m_UserInterfaces.GetSize() == 0)
	{
		CUserInterface *pUI = AddUserInterface(0);
		if (pUI)
		{
			pUI->ShowPreference(pUI->GetSafeHwnd(), TRUE);
		}
	}

	// now set the auto advance after all the UI are setup
	if (pAutoAdvanceUI)
	{
		SetAutoAdvance(pAutoAdvanceUI);
	}
	// if no auto advance saved from last time
	// pick the first UI
	else if (m_UserInterfaces.GetSize())
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(0);
		SetAutoAdvance(pUI);
	}

	// now do the focus UI
	if (pFocusUI)
	{
		SetFocus(pFocusUI);
	}
	// if nothing saved from last time
	// pick the first UI
	else if (m_UserInterfaces.GetSize())
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(0);
		SetFocus(pUI);
	}

	if (m_bResumePlayback)
	{
		SetTimer(m_hWnd, TIMERID_RESUMEPLAYBACK, 1000, NULL);
	}

	return TRUE;
}

BOOL CWinampAL::OnPostInit()
{
	// Initialize Media Library
	if (m_bEmbedML) InitML();

	InitEncoding();

	InitProfiles();

	if (m_bVersionCheck && wndWinamp.SendIPCMessage(0, IPC_INETAVAILABLE))
	{
		CheckForNewVersion();
	}

	return TRUE;
}

BOOL CWinampAL::Shutdown()
{
	if (!m_bInitialized) return TRUE;

	Archive(TRUE /*write*/);

	UpdateMSN(NULL);

	// go through all user interfaces (profiles)
	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);
		if (pUI)
		{
			pUI->Shutdown();
		}
	}

	m_bInitialized = FALSE;

	return TRUE;
}

BOOL CWinampAL::Archive(BOOL bWrite)
{
	if (bWrite)
	{
		// reset them first
		m_nResumeTime	= -1;
		m_nResumePos	= 0;
		m_nResumePaused	= 0;
		memset(m_szResumeFile, 0, MAX_PATH);

		if (m_bResumePlayback)
		{
			// save position if we're not stopped
			int nPlayState = 0;
			if ((nPlayState = wndWinamp.SendIPCMessage(0, IPC_ISPLAYING)) != 0)
			{
				m_nResumeTime = wndWinamp.SendIPCMessage(0, IPC_GETOUTPUTTIME);

				m_nResumePaused = (nPlayState == 3);
			}

			m_nResumePos = wndWinamp.SendIPCMessage(0, IPC_GETLISTPOS);

			lstrcpyn(m_szResumeFile, (LPCTSTR)wndWinamp.SendIPCMessage(m_nResumePos, IPC_GETPLAYLISTFILE), MAX_PATH);
		}

		char string[32];
		wsprintf(string, "%ld", m_bResumePlayback);		WritePrivateProfileString("Main", "Resume Playback",	string, iniPlugin);
		wsprintf(string, "%ld", m_nResumeTime);			WritePrivateProfileString("Main", "Resume Time",		string, iniPlugin);
		wsprintf(string, "%ld", m_nResumePos);			WritePrivateProfileString("Main", "Resume Position",	string, iniPlugin);
		wsprintf(string, "%ld", m_nResumePaused);		WritePrivateProfileString("Main", "Resume Paused",		string, iniPlugin);
		WritePrivateProfileString("Main", "Resume File", m_szResumeFile, iniPlugin);

		wsprintf(string, "%ld", m_bKeepSongPlaying);	WritePrivateProfileString("Main", "Keep Song Playing",	string, iniPlugin);
		wsprintf(string, "%ld", m_bSortByFilename);		WritePrivateProfileString("Main", "Sort By Filename",	string, iniPlugin);
		wsprintf(string, "%ld", m_bEnqueueDefault);		WritePrivateProfileString("Main", "Enqueue Default",	string, iniPlugin);
		wsprintf(string, "%ld", m_EncodingLangAL);		WritePrivateProfileString("Main", "Encoding AL",		string, iniPlugin);
		wsprintf(string, "%ld", m_EncodingLangPE);		WritePrivateProfileString("Main", "Encoding PE",		string, iniPlugin);
		wsprintf(string, "%ld", m_bUpdateMSN);			WritePrivateProfileString("Main", "Update MSN",			string, iniPlugin);

		wsprintf(string, "%ld", m_nLastConfig);			WritePrivateProfileString("Main", "Last Config",		string, iniPlugin);
		wsprintf(string, "%ld", m_bLoop);				WritePrivateProfileString("Main", "Loop",				string, iniPlugin);
		wsprintf(string, "%ld", m_bShowCover);			WritePrivateProfileString("Main", "Show Cover",			string, iniPlugin);
		wsprintf(string, "%ld", m_bAutoAdvance);		WritePrivateProfileString("Main", "Auto Advance",		string, iniPlugin);
		wsprintf(string, "%ld", m_bAdvanceRandom);		WritePrivateProfileString("Main", "Advance Random",		string, iniPlugin);
		wsprintf(string, "%ld", m_bCompression);		WritePrivateProfileString("Main", "Compression",		string, iniPlugin);
		wsprintf(string, "%ld", m_bVersionCheck);		WritePrivateProfileString("Main", "Version Check",		string, iniPlugin);
		wsprintf(string, "%ld", m_tCheckTime);			WritePrivateProfileString("Main", "Check Time",			string, iniPlugin);
		wsprintf(string, "%ld", m_dwLatestVer);			WritePrivateProfileString("Main", "Latest Version",		string, iniPlugin);
//		wsprintf(string, "%ld", m_bLogFile);			WritePrivateProfileString("Main", "Log File",			string, iniPlugin);
		wsprintf(string, "%ld", m_bBuiltInFileInfo);	WritePrivateProfileString("Main", "File Info",			string, iniPlugin);
		wsprintf(string, "%ld", m_bWinampFileInfo);		WritePrivateProfileString("Main", "Winamp File Info",	string, iniPlugin);
//		wsprintf(string, "%ld", m_bFreeFormMP3);		WritePrivateProfileString("Main", "Free Format MP3",	string, iniPlugin);
		wsprintf(string, "%ld", m_bLightningBolt);		WritePrivateProfileString("Main", "Lightning Bolt",		string, iniPlugin);
		wsprintf(string, "%ld", m_nAutoAdvanceID);		WritePrivateProfileString("Main", "Auto Advance ID",	string, iniPlugin);
		wsprintf(string, "%ld", m_nFocusID);			WritePrivateProfileString("Main", "Focus ID",			string, iniPlugin);
//		wsprintf(string, "%ld", m_nVariousArtistRatio);	WritePrivateProfileString("Main", "Various Artist Ratio", string, iniPlugin);
		wsprintf(string, "%ld", m_bEmbedMLSave);		WritePrivateProfileString("Main", "Embed ML",			string, iniPlugin);
		wsprintf(string, "%ld", m_bTabbedUISave);		WritePrivateProfileString("Main", "Tabbed UI",			string, iniPlugin);
		wsprintf(string, "%ld", m_bPlayAllWarning);		WritePrivateProfileString("Main", "PlayAll Warning",	string, iniPlugin);
		wsprintf(string, "%ld", m_nPlayAllLimit);		WritePrivateProfileString("Main", "PlayAll Limit",		string, iniPlugin);

		WritePrivateProfileString("Main", "Translation", m_szTranslation, iniPlugin);
	}
	else
	{
		if (GetPrivateProfileInt("Main", "Check Time", -1, iniPlugin) == -1)
			m_bFirstTimer = TRUE;

		m_bResumePlayback	= GetPrivateProfileInt("Main", "Resume Playback",	m_bResumePlayback,	iniPlugin);
		m_nResumeTime		= GetPrivateProfileInt("Main", "Resume Time",		m_nResumeTime,		iniPlugin);
		m_nResumePos		= GetPrivateProfileInt("Main", "Resume Position",	m_nResumePos,		iniPlugin);
		m_nResumePaused		= GetPrivateProfileInt("Main", "Resume Paused",		m_nResumePaused,	iniPlugin);
		GetPrivateProfileString("Main", "Resume File", "", m_szResumeFile, MAX_PATH, iniPlugin);

		m_bKeepSongPlaying	= GetPrivateProfileInt("Main", "Keep Song Playing", m_bKeepSongPlaying,	iniPlugin);
		m_bSortByFilename	= GetPrivateProfileInt("Main", "Sort By Filename",	m_bSortByFilename,	iniPlugin);
		m_bEnqueueDefault	= GetPrivateProfileInt("Main", "Enqueue Default",	m_bEnqueueDefault,	iniPlugin);
		m_EncodingLangAL	= (ALLanguage)GetPrivateProfileInt("Main", "Encoding AL",		m_EncodingLangAL,	iniPlugin);
		m_EncodingLangPE	= (ALLanguage)GetPrivateProfileInt("Main", "Encoding PE",		m_EncodingLangPE,	iniPlugin);
		m_bUpdateMSN		= GetPrivateProfileInt("Main", "Update MSN",		m_bUpdateMSN,		iniPlugin);

		m_nLastConfig		= GetPrivateProfileInt("Main", "Last Config",		m_nLastConfig,		iniPlugin);
		m_bLoop				= GetPrivateProfileInt("Main", "Loop",				m_bLoop,			iniPlugin);
		m_bShowCover		= GetPrivateProfileInt("Main", "Show Cover",		m_bShowCover,		iniPlugin);
		m_bAutoAdvance		= GetPrivateProfileInt("Main", "Auto Advance",		m_bAutoAdvance,		iniPlugin);
		m_bAdvanceRandom	= GetPrivateProfileInt("Main", "Advance Random",	m_bAdvanceRandom,	iniPlugin);
		m_bCompression		= GetPrivateProfileInt("Main", "Compression",		m_bCompression,		iniPlugin);
		m_bVersionCheck		= GetPrivateProfileInt("Main", "Version Check",		m_bVersionCheck,	iniPlugin);
		m_tCheckTime		= GetPrivateProfileInt("Main", "Check Time",		m_tCheckTime,		iniPlugin);
		m_dwLatestVer		= GetPrivateProfileInt("Main", "Latest Version",	m_dwLatestVer,		iniPlugin);
		m_bLogFile			= GetPrivateProfileInt("Main", "Log File",			m_bLogFile,			iniPlugin);
		m_bBuiltInFileInfo	= GetPrivateProfileInt("Main", "File Info",			m_bBuiltInFileInfo,	iniPlugin);
		m_bFreeFormMP3		= GetPrivateProfileInt("Main", "Free Format MP3",	m_bFreeFormMP3,		iniPlugin);
		m_bLightningBolt	= GetPrivateProfileInt("Main", "Lightning Bolt",	m_bLightningBolt,	iniPlugin);
		m_nVariousArtistRatio = GetPrivateProfileInt("Main", "Various Artist Ratio", m_nVariousArtistRatio, iniPlugin);
		m_bWinampFileInfo	= GetPrivateProfileInt("Main", "Winamp File Info",	m_bWinampFileInfo,	iniPlugin);
		m_nAutoAdvanceID	= GetPrivateProfileInt("Main", "Auto Advance ID",	m_nAutoAdvanceID,	iniPlugin);
		m_nFocusID			= GetPrivateProfileInt("Main", "Focus ID",			m_nFocusID,			iniPlugin);
		m_bEmbedML			= GetPrivateProfileInt("Main", "Embed ML",			m_bEmbedML,			iniPlugin);
		m_bEmbedMLSave		= m_bEmbedML;
		m_bTabbedUI			= GetPrivateProfileInt("Main", "Tabbed UI",			m_bTabbedUI,		iniPlugin);
		m_bTabbedUISave		= m_bTabbedUI;
		m_bPlayAllWarning	= GetPrivateProfileInt("Main", "PlayAll Warning",	m_bPlayAllWarning,	iniPlugin);
		m_nPlayAllLimit		= GetPrivateProfileInt("Main", "PlayAll Limit",		m_nPlayAllLimit,	iniPlugin);
		if (m_nPlayAllLimit <= 0) m_nPlayAllLimit = 10;

		GetPrivateProfileString("Main", "Translation", "", m_szTranslation, MAX_PATH, iniPlugin);
		m_nTranslationCodepage = GetPrivateProfileInt("Info", "Codepage", CP_ACP, m_szTranslation);
	}

	return TRUE;
}

int CWinampAL::GetCodepage()
{
	return m_nTranslationCodepage;
}

int CWinampAL::GetEncodingCodepage()
{
	int codepage = CP_ACP;
	int encoding = wndWinampAL.GetSetting(settingEncodingLangAL);
	switch (encoding)
	{
	case langTraditionalChinese:	codepage = 950;	break;
	case langSimplifiedChinese:		codepage = 936;	break;
	case langJapanese:				codepage = 932;	break;
	case langKorean:				codepage = 949;	break;
	}

	return codepage;
}

int CWinampAL::GetSetting(ALSettingInt setting)
{
	switch (setting)
	{
	case settingKeepSongPlaying:
		return m_bKeepSongPlaying;

	case settingSortByFilename:
		return m_bSortByFilename;

	case settingResumePlayback:
		return m_bResumePlayback;

	case settingEnqueueDefault:
		return m_bEnqueueDefault;

	case settingEncodingLangAL:
		return m_EncodingLangAL;

	case settingEncodingLangPE:
		return m_EncodingLangPE;

	case settingLastConfig:
		return m_nLastConfig;

	case settingLoop:
		return m_bLoop;

	case settingShowCover:
		return m_bShowCover;

	case settingAutoAdvance:
		return m_bAutoAdvance;

	case settingCompression:
		return m_bCompression;

	case settingAdvanceRandom:
		return m_bAdvanceRandom;

	case settingVersionCheck:
		return m_bVersionCheck;

	case settingLogFile:
		return m_bLogFile;

	case settingBuiltInFileInfo:
		return m_bBuiltInFileInfo;

	case settingWinampFileInfo:
		return m_bWinampFileInfo;

	case settingFreeFormMP3:
		return m_bFreeFormMP3;

	case settingLightningBolt:
		return m_bLightningBolt;

	case settingVariousArtistRatio:
		return m_nVariousArtistRatio;

	case settingEmbedML:
		return m_bEmbedML;

	case settingEmbedMLSave:
		return m_bEmbedMLSave;

	case settingPlayAllWarning:
		return m_bPlayAllWarning;

	case settingPlayAllLimit:
		return m_nPlayAllLimit;

	case settingMSN:
		return m_bUpdateMSN;

	case settingTabbedUI:
		return m_bTabbedUI;

	case settingTabbedUISave:
		return m_bTabbedUISave;
	}

	return -1;
}

void CWinampAL::SetSetting(ALSettingInt setting, int value)
{
	switch (setting)
	{
	case settingKeepSongPlaying:
		m_bKeepSongPlaying = value;
		break;

	case settingSortByFilename:
		m_bSortByFilename = value;
		break;

	case settingResumePlayback:
		m_bResumePlayback = value;
		break;

	case settingEnqueueDefault:
		m_bEnqueueDefault = value;
		break;

	case settingEncodingLangAL:
		m_EncodingLangAL = (ALLanguage)value;
		break;

	case settingEncodingLangPE:
		m_EncodingLangPE = (ALLanguage)value;
		break;

	case settingLastConfig:
		m_nLastConfig = value;
		break;

	case settingLoop:
		m_bLoop = value;
		break;

	case settingShowCover:
		m_bShowCover = value;
		break;

	case settingAutoAdvance:
		m_bAutoAdvance = value;
		break;

//	case settingCompression:
//		m_bCompression = value;
//		break;

	case settingAdvanceRandom:
		m_bAdvanceRandom = value;
		break;

	case settingVersionCheck:
		m_bVersionCheck = value;
		break;

//	case settingLogFile:
//		m_bLogFile = value;
//		break;

	case settingBuiltInFileInfo:
		m_bBuiltInFileInfo = value;
		break;

	case settingWinampFileInfo:
		m_bWinampFileInfo = value;
		break;

//	case settingFreeFormMP3:
//		m_bFreeFormMP3 = value;
//		break;

	case settingLightningBolt:
		m_bLightningBolt = value;
		break;

//	case settingVariousArtistRatio:
//		m_nVariousArtistRatio = value;
//		break;

	case settingEmbedML:
		m_bEmbedMLSave = value;
		break;

	case settingPlayAllWarning:
		m_bPlayAllWarning = value;
		break;

	case settingPlayAllLimit:
		m_nPlayAllLimit = value;
		break;

	case settingMSN:
		// update MSN now
		if (value)
		{
			m_bUpdateMSN = value;

			int pos = wndWinamp.SendIPCMessage(0,IPC_GETLISTPOS);
			LPCTSTR filename = (LPCTSTR)wndWinamp.SendIPCMessage(pos, IPC_GETPLAYLISTFILE);
			UpdateMSN((LPCTSTR)filename);
		}
		// clear MSN when we turn it off
		else
		{
			UpdateMSN(NULL);

			m_bUpdateMSN = value;
		}
		break;

	case settingTabbedUI:
		m_bTabbedUISave = value;
		break;
	}

	// notification for setting changes?
}

LPCTSTR CWinampAL::GetSetting(ALSettingStr setting)
{
	switch (setting)
	{
	case settingTranslation:
		return m_szTranslation;
	}

	return "";
}

void CWinampAL::SetSetting(ALSettingStr setting, LPCTSTR value)
{
	switch (setting)
	{
	case settingTranslation:
		lstrcpyn(m_szTranslation, value, MAX_PATH);
		m_nTranslationCodepage = GetPrivateProfileInt("Info", "Codepage", CP_ACP, m_szTranslation);
		break;
	}
	// notification for setting changes?
}

BOOL CWinampAL::ProcessWinampMenu(int nID)
{
	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);
		if (pUI && pUI->ProcessWinampMenu(nID))
			return TRUE;
	}

	return FALSE;
}

void CWinampAL::SetAutoAdvance(CUserInterface *pUI)
{
	m_pAutoAdvanceUI = pUI;
	m_nAutoAdvanceID = (pUI != NULL) ? pUI->GetProfileID() : -1;
}

void CWinampAL::SetFocus(CUserInterface *pUI)
{
	m_pFocusUI = pUI;
	m_nFocusID = (pUI != NULL) ? pUI->GetProfileID() : -1;
}

void CWinampAL::StopPressed()
{
	// stop pressed by the user
	// cancel any auto advance timer
	KillTimer(m_hWnd, TIMERID_ENDOFSONG);
}

BOOL CWinampAL::OnEndOfSong()
{
	if (m_bAutoAdvance)
	{
		// if it is not playing... it is probably a FULL STOP
		if (wndWinamp.SendIPCMessage(0, IPC_ISPLAYING) == 0)
		{
			if (m_pAutoAdvanceUI)
			{
				if (m_bAdvanceRandom)
					m_pAutoAdvanceUI->IPC_PlayRandomAlbum();
				else
					m_pAutoAdvanceUI->IPC_PlayNextAlbum();
			}
		}
	}

	return TRUE;
}

BOOL CWinampAL::OnFrontEnd(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IPC_GETVERSION)
		return IPC_GetVersion();

	int size = m_UserInterfaces.GetSize();
	if (size == 0) return FALSE;

	// select the profile to use (HIWORD of wParam)
	CUserInterface *pUI = NULL;

	switch (wParam)
	{
	case IPC_SHOWHIDE_CUR:
	case IPC_JUMPTOALBUM_CUR:
	case IPC_PLAYPREVALBUM_CUR:
	case IPC_PLAYNEXTALBUM_CUR:
	case IPC_PLAYPREVALBUMARTIST_CUR:
	case IPC_PLAYNEXTALBUMARTIST_CUR:
	case IPC_PLAYRANDOMALBUM_CUR:
	case IPC_PLAYALLALBUMS_CUR:
	case IPC_PLAYALBUM_CUR:
	case IPC_GETALBUMSIZE_CUR:
	case IPC_SHOWPREFERENCE_CUR:
	case IPC_PLAYALBUM1_CUR:
	case IPC_GETALBUMINDEX_CUR:
	case IPC_PLAYALBUMNAME_CUR:
		pUI = m_pFocusUI;
		break;

	default:
		pUI = (CUserInterface *)m_UserInterfaces.GetAt(HIWORD(wParam));
	}
	
	if (pUI == NULL) return FALSE;

	switch (LOWORD(wParam))
	{
	case IPC_PLAYALBUM:			// play a specific album
	case IPC_PLAYALBUM_CUR:
		return pUI->IPC_PlayAlbum(lParam);
	case IPC_PLAYALBUM1:		// play a specific album
	case IPC_PLAYALBUM1_CUR:
		return pUI->IPC_PlayAlbum(lParam-1);

	case IPC_ENQUEUEALBUM:		// enqueue a specific album
		return pUI->IPC_EnqueueAlbum(lParam);
	case IPC_ENQUEUEALBUM1:		// enqueue a specific album
		return pUI->IPC_EnqueueAlbum(lParam-1);

	case IPC_GETALBUMSIZE:		// get number of albums
	case IPC_GETALBUMSIZE_CUR:
		return pUI->IPC_GetAlbumSize();

	case IPC_PLAYRANDOMALBUM:	// play a random album
	case IPC_PLAYRANDOMALBUM_CUR:
		return pUI->IPC_PlayRandomAlbum();
	case IPC_PLAYPREVALBUM:		// play a previous album
	case IPC_PLAYPREVALBUM_CUR:
		return pUI->IPC_PlayPreviousAlbum();
	case IPC_PLAYNEXTALBUM:		// play a next album
	case IPC_PLAYNEXTALBUM_CUR:
		return pUI->IPC_PlayNextAlbum();
	case IPC_PLAYALLALBUMS:		// play all albums
	case IPC_PLAYALLALBUMS_CUR:
		return pUI->IPC_PlayAllAlbums();
	case IPC_PLAYPREVALBUMARTIST:	// play a previous album (same artist)
	case IPC_PLAYPREVALBUMARTIST_CUR:
		return pUI->IPC_PlayPreviousAlbumArtist();
	case IPC_PLAYNEXTALBUMARTIST:	// play a next album (same artist)
	case IPC_PLAYNEXTALBUMARTIST_CUR:
		return pUI->IPC_PlayNextAlbumArtist();

	case IPC_GETALBUMNAME:		// get album name
		return (LRESULT)pUI->IPC_GetAlbumName(lParam);
	case IPC_GETALBUMNAME1:		// get album name
		return (LRESULT)pUI->IPC_GetAlbumName(lParam-1);

	case IPC_GETALBUMINDEX:		// get current album index
	case IPC_GETALBUMINDEX_CUR:
		return pUI->IPC_GetAlbumIndex();
	case IPC_GETALBUMINDEX1:	// get current album index
		return pUI->IPC_GetAlbumIndex()+1;

	case IPC_GETALBUMYEAR:		// get album year
		return pUI->IPC_GetAlbumYear(lParam);
	case IPC_GETALBUMYEAR1:		// get album year
		return pUI->IPC_GetAlbumYear(lParam-1);

	case IPC_GETALBUMTITLE:		// get album title
		return (LRESULT)pUI->IPC_GetAlbumTitle(lParam);
	case IPC_GETALBUMTITLE1:	// get album title
		return (LRESULT)pUI->IPC_GetAlbumTitle(lParam-1);

	case IPC_GETALBUMARTIST:	// get album artist name
		return (LRESULT)pUI->IPC_GetAlbumArtist(lParam);
	case IPC_GETALBUMARTIST1:	// get album artist name
		return (LRESULT)pUI->IPC_GetAlbumArtist(lParam-1);

	case IPC_SHOWHIDE:			// show/hide Album List
	case IPC_SHOWHIDE_CUR:
		pUI->IPC_ShowHide();
		return TRUE;

	case IPC_JUMPTOALBUM:		// Jump to album
	case IPC_JUMPTOALBUM_CUR:
		pUI->IPC_JumpToAlbum();
		return TRUE;

	case IPC_SHOWPREFERENCE:
	case IPC_SHOWPREFERENCE_CUR:
		pUI->IPC_ShowPreference();
		return TRUE;

	case IPC_PLAYALBUMNAME:			// play a specific album name
	case IPC_PLAYALBUMNAME_CUR:
		return pUI->IPC_PlayAlbumName((LPCTSTR)lParam);
	}

	return FALSE;
}

BOOL CWinampAL::OnDeviceChange(UINT nEventType, DWORD dwData)
{
	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);

		pUI->OnDeviceChange(nEventType, dwData);
	}
	return TRUE;
}

CUserInterface *CWinampAL::AddUserInterface(int nID)
{
	CUserInterface *pUI = new CUserInterface(nID);
	if (pUI)
	{
		if (pUI->Startup() &&
			((m_bEmbedML && pUI->InitML(m_nMLTreeItem)) || pUI->Create()))
		{
			m_UserInterfaces.Add(pUI);
		}
		else
		{
			delete pUI;
			pUI = NULL;
		}
	}
	return pUI;
}

BOOL CWinampAL::RemoveUserInterface(int nID)
{
	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);

		if (nID == pUI->GetProfileID())
		{
			// remove it from the list
			m_UserInterfaces.RemoveAt(i);

			// reset auto advance
			if (m_pAutoAdvanceUI == pUI)
			{
				// default back to profile 0
				SetAutoAdvance((CUserInterface *)m_UserInterfaces.GetAt(0));
			}

			// reset focus UI
			if (m_pFocusUI == pUI)
			{
				// default back to profile 0
				SetFocus((CUserInterface *)m_UserInterfaces.GetAt(0));
			}

			// clean up menu entries and library tree
			pUI->Cleanup();

			// free memory
			delete pUI;

			// delete the profile info in the ini file
			char str[MAX_PATH];
			wsprintf(str, "Profile %ld", nID);
			WritePrivateProfileSection(str, NULL, iniPlugin);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CWinampAL::IsProfileIDUsed(int nID)
{
	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);

		if (nID == pUI->GetProfileID())
			return TRUE;
	}

	return FALSE;
}

BOOL CWinampAL::OkToQuit()
{
	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);

		if (!pUI->OkToQuit())
			return FALSE;
	}

	return TRUE;
}

void CWinampAL::ResetFont()
{
	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);
		if (pUI) pUI->ResetFont();
	}
}

BOOL CWinampAL::AddIPCMenu(addStruct *as)
{
	if (as == NULL) return FALSE;

	CIPCMenu *m = new CIPCMenu;

	lstrcpyn(m->szName, as->name, MAX_PATH);
	m->hWnd = as->wnd;
	m->uMsg = as->uMsg;

	m_IPCMenuList.Add(m);

	return TRUE;
}

BOOL CWinampAL::GetIPCMenu(CPtrArray **a)
{
	if (a == NULL) return FALSE;

	if (m_IPCMenuList.GetSize() == 0) return FALSE;

	*a = &m_IPCMenuList;

	return TRUE;
}

void CWinampAL::RemoveFromMainMenu(HMENU hMenu)
{
	if (m_bEmbedML) return;

	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);
		if (pUI)
		{
			RemoveMenu(hMenu, pUI->GetMenuID(), MF_BYCOMMAND);
		}
	}
}

void CWinampAL::InsertToMainMenu(HMENU hMenu)
{
	if (m_bEmbedML) return;

	char buf[MAX_PATH];
	char title[MAX_PATH];

	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);
		if (pUI)
		{
			lstrcpyn(title, pUI->GetTitle(), MAX_PATH);

			// find menu item "main window"
			for (int i=GetMenuItemCount(hMenu); i>=0; i--)
			{
				if (GetMenuItemID(hMenu, i) == WINAMP_MAIN_WINDOW/*40258*/)
				{
					BOOL bDoNotInsert = FALSE;
					// find the separator
					while (0xFFFFFFFF != GetMenuItemID(hMenu, ++i))
					{
						MENUITEMINFO mii;
						memset(&mii, 0, sizeof(MENUITEMINFO));
						mii.cbSize = sizeof(MENUITEMINFO);
						mii.fMask = MIIM_TYPE;
						mii.dwTypeData = buf;
						mii.cch = MAX_PATH;
						if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
						{
							if (lstrcmp(title, buf) == 0)
							{
								bDoNotInsert = TRUE;
								break;
							}
						}
					}

					// insert menu just before the separator
					if (!bDoNotInsert)
					{
						InsertMenu(hMenu, i-1, MF_BYPOSITION|MF_STRING, pUI->GetMenuID(), title);
						if (IsWindowVisible(pUI->m_hWnd))
						{
							CheckMenuItem(hMenu, pUI->GetMenuID(), MF_CHECKED|MF_BYCOMMAND);
						}
					}
					break;
				}
			}
		}
	}
}

void CWinampAL::UpdateMSN(LPCTSTR szFile)
{
	if (!m_bUpdateMSN) return;

#define MSNMusicString "\\0Music\\0%d\\0%s\\0%s\\0%s\\0%s\\0%s\\0"
#define MSNFormat "{0} - {1}"

	bool bshow = (szFile != NULL) ? 1 : 0;
	
	int cp = CP_ACP;
	switch (m_EncodingLangPE)
	{
	case langTraditionalChinese:	cp = 950;		break;
	case langSimplifiedChinese:		cp = 936;		break;
	case langJapanese:				cp = 932;		break;
	case langKorean:				cp = 949;		break;
	}

	char cmd[500]		= "";
	char title[256]		= "";
	char artist[256]	= "";
	char format[256]	= MSNFormat;

	if (szFile)
	{
		CFileInfo *info = CreateWAExtendedInfo();
		if (info)
		{
			if (info->loadInfo(szFile) == 0)
			{
				info->getTitle(title);
				info->getArtist(artist);
			}
			delete info;
		}

		if (lstrlen(artist) == 0)
		{
			lstrcpy(format, "{0}");
		}
		if (lstrlen(title) == 0)
		{
			lstrcpy(title, szFile);
			PathStripPath(title);
			PathRemoveExtension(title);
		}
	}

	wsprintf(cmd, MSNMusicString, bshow, format, title, artist, "", "");
	
	WCHAR buffer[500];
	MultiByteToWideChar(cp, MB_PRECOMPOSED, cmd, -1, buffer, 500);

	COPYDATASTRUCT msndata;
	msndata.dwData = 0x547;
	msndata.lpData = &buffer;
	msndata.cbData = (lstrlenW(buffer)*2)+2;
	
	HWND msnui = NULL;
	while (msnui = FindWindowEx(NULL, msnui, "MsnMsgrUIManager", NULL))
	{
		::SendMessage(msnui, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&msndata);
	}
}

//////////////////////////////////////////////////////////////////
// Version Check functions

DWORD CWinampAL::GetLatestVersion()
{
	if (m_dwLatestVer == 0)
	{
		int maj = 0, min = 0, bld = 0;
		GetVersionInformation(hDllInstance, maj, min, bld);
		m_dwLatestVer = MAKELONG(bld, MAKEWORD(min, maj));
	}
	return m_dwLatestVer;
}

void CWinampAL::CheckForNewVersion()
{
	time_t t;
	time(&t);

	// check for update every 5 days...
	if ((t - m_tCheckTime) > 432000/*5days*/)
	{
		m_hThread = CreateThread(NULL, 0, CheckVerThreadProc, this, CREATE_SUSPENDED, &m_dwThreadId);
		SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
		ResumeThread(m_hThread);
	}
}

DWORD WINAPI CWinampAL::CheckVerThreadProc(LPVOID lpData)
{
	SetThreadName(-1, "CheckVerThreadProc");

	CWinampAL *pAL = (CWinampAL *)lpData;
	if (pAL)
	{
#if 1
		if (URLMonLoad())
		{
			char *url = "http://albumlist.sourceforge.net/alupdate.txt";

			// create temperary update.txt
			char tempfile[MAX_PATH] = "";
			if (GetTempPath(MAX_PATH, tempfile) == 0)
				return 0;
			PathAppend(tempfile, "alupdate.txt");

			if (pAL->DownloadUpdate(url, tempfile))
			{
				pAL->ProcessVerUpdate(tempfile);

				DeleteFile(tempfile);
			}

			URLMonFree();
		}
#else
		// local testing of update function
		pAL->ProcessVerUpdate("D:\\Projects\\Winamp\\Gen_m3a.web\\htdocs\\alupdate.txt");
#endif

		// close handle and stuff
		CloseHandle(pAL->m_hThread);
		pAL->m_hThread		= NULL;
		pAL->m_dwThreadId	= NULL;
	}

	return 0;
}

BOOL CWinampAL::DownloadUpdate(LPCTSTR url, LPCTSTR file)
{
	return SUCCEEDED(URLDownloadToFile(NULL, url, file, 0, NULL));
}

void CWinampAL::ProcessVerUpdate(LPCTSTR file)
{
	FILE *f = fopen(file, "rt");
	if (f != NULL)
	{
		BOOL bValid = FALSE;
		BOOL bNewVersion = FALSE;
		int major = 0, minor = 0;
		char download_url[256] = "http://albumlist.sourceforge.net";

		CStringArray added;
		CStringArray fixed;

		char line[512];
		DWORD newVer = 0;
		while (fgets(line, 512, f))
		{
			int len = lstrlen(line);
			if (line[len-1] == 0xa) line[len-1] = 0;

			// need valid file header first
			// GEN_M3A
			if (!bValid && (0 == lstrcmpi(line, "#GEN_M3A")))
			{
				bValid = TRUE;
				continue;
			}

			// no valid file header yet? keep trying to find it
			if (!bValid) continue;

			// get version number
			if (0 == strnicmp(line, "#VERSION:", 9))
			{
				int maj = 0, min = 0, bld = 0;
				GetVersionInformation(hDllInstance, maj, min, bld);

				DWORD curVer = MAKELONG(bld, MAKEWORD(min, maj));

				sscanf(line + 9, "%ld.%ld.%ld", &major, &minor, &bld);
				newVer = MAKELONG(bld, MAKEWORD(minor, major));

				bNewVersion = (BOOL)(newVer > curVer);

				// same version, stop parsing
				if (!bNewVersion) break;
			}
			// added features
			else if (0 == strnicmp(line, "#ADDED:", 7))
			{
				added.Add(line+7);
			}
			// fixed bugs
			else if (0 == strnicmp(line, "#FIXED:", 7))
			{
				fixed.Add(line+7);
			}
			// url for download
			else if (0 == strnicmp(line, "#URL:", 5))
			{
				lstrcpyn(download_url, line+5, 256);
			}
		}
		// close the file handle
		fseek(f, 0, SEEK_END);
		LONGLONG file_size = 0;
		fgetpos(f, &file_size);
		fclose(f);

		time_t t;
		time(&t);

		// show dialog every 10 days...
		if ((newVer > GetLatestVersion()) ||
			(bNewVersion && ((t - m_tCheckTime) > 864000/*10days*/)))
		{
			m_tCheckTime = t;
			m_dwLatestVer = newVer;

			// show dialog
			char *message = new char [(DWORD)file_size+1024];
			if (message)
			{
				wsprintf(message, ALS("Album List v%ld.%02ld is now available for download!"), major, minor);
				lstrcat(message, "\n");
				lstrcat(message, ALS("In the latest version of Album List:"));
				lstrcat(message, "\n\n");

				int size = added.GetSize();
				for (int i=0; i<size; ++i)
				{
					lstrcat(message, added[i]);
					lstrcat(message, "\n");
				}
				size = fixed.GetSize();
				for (i=0; i<size; ++i)
				{
					lstrcat(message, fixed[i]);
					lstrcat(message, "\n");
				}

				lstrcat(message, "\n");
				lstrcat(message, ALS("Would you like to upgrade to the latest version?"));

				if (MessageBox(NULL, message, ALS("New Album List Available"), MB_YESNO) == IDYES)
				{
					GotoURL(download_url, SW_SHOWNORMAL);
				}

				delete [] message;
			}
		}

	}
}

//////////////////////////////////////////////////////////////////
// IPC functions

DWORD CWinampAL::IPC_GetVersion()
{
	int major, minor, build;
	GetVersionInformation(hDllInstance, major, minor, build);

	return MAKELONG(MAKEWORD(build, 0), MAKEWORD(minor, major));
}

//////////////////////////////////////////////////////////////////
// Encoding functions

typedef int (WINAPI *DrawTextA_Type)(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat);
int WINAPI MyDrawTextA(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat);

enum
{
    UserFN_DrawTextA = 0
};

SDLLHook User32Hook = 
{
    "USER32.DLL",
    false, NULL,		// Default hook disabled, NULL function pointer.
    {
		{ "DrawTextA", MyDrawTextA },
        { NULL, NULL }
    }
};

int WINAPI MyDrawTextA(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
#if 0
	DrawTextA_Type OldFn = 
		(DrawTextA_Type)User32Hook.Functions[UserFN_DrawTextA].OrigFn;
	int n = OldFn(hDC, lpString, nCount, lpRect, uFormat);
	wndWinampAL.DrawTextUnicode(hDC, lpString, nCount, lpRect, uFormat);
	return n;
#else
	int nRet = wndWinampAL.DrawTextUnicode(hDC, lpString, nCount, lpRect, uFormat);
	if (nRet == 0)
	{
		DrawTextA_Type OldFn = 
			(DrawTextA_Type)User32Hook.Functions[UserFN_DrawTextA].OrigFn;
		return OldFn(hDC, lpString, nCount, lpRect, uFormat);
	}
	return nRet;
#endif
}

BOOL CWinampAL::InitEncoding()
{
	User32Hook.Functions[UserFN_DrawTextA].HookFn = MyDrawTextA;

	// hack the api
	HookAPICalls(&User32Hook);

	return TRUE;
}

int CWinampAL::DrawTextUnicode(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
	int cp = 0;

	if (WindowFromDC(hDC) != wndWinampPE) return 0;

	switch (m_EncodingLangPE)
	{
	case langSystemDefault:
		return 0;
			
	case langTraditionalChinese:	cp = 950;	break;
	case langSimplifiedChinese:		cp = 936;	break;
	case langJapanese:				cp = 932;	break;
	case langKorean:				cp = 949;	break;

	default:
		return 0;
	}

	if (nCount == -1) nCount = lstrlen(lpString);

	WCHAR *lpStringW = new WCHAR [nCount];

	nCount = MultiByteToWideChar(cp, MB_PRECOMPOSED, lpString, nCount, lpStringW, nCount);

	int nRet = DrawTextW(hDC, lpStringW, nCount, lpRect, uFormat);

	delete [] lpStringW;

	return nRet;
}

//////////////////////////////////////////////////////////////////
// Media Library Stuff

static int ml_init()
{
	return 0;
}

static void ml_quit()
{
}

INT_PTR ml_PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	return wndWinampAL.MLPluginMessageProc(message_type, param1, param2, param3);
}

BOOL CWinampAL::InitML()
{
	// check for media library support
	long libhwndipc = (LONG)wndWinamp.SendIPCMessage(WPARAM("LibraryGetWnd"), IPC_REGISTER_WINAMP_IPCMESSAGE);

	// Now send the message and get the HWND
	CWnd wndML = (HWND)wndWinamp.SendIPCMessage(-1, LPARAM(libhwndipc));

	// Disable use of Media Library if Media Library is not available
	if (!wndML.GetSafeHwnd()) m_bEmbedML = FALSE;

	// Add to Media Library
	static winampMediaLibraryPlugin plugin;	// plugin struct for ML

	plugin.version			= MLHDR_VER;
	plugin.description		= pluginGP.description;
	plugin.init				= ml_init;
	plugin.quit				= ml_quit;
	plugin.MessageProc		= ml_PluginMessageProc;
	plugin.hwndWinampParent	= 0;
	plugin.hwndLibraryParent= 0;
	plugin.hDllInstance		= pluginGP.hDllInstance;

	wndML.SendMessage(WM_ML_IPC, (WPARAM)&plugin, ML_IPC_ADD_PLUGIN);

	// add to media library's tree
	mlAddTreeItemStruct mla = { 0, "Album List", 1,  };

	wndML.SendMessage(WM_ML_IPC, (WPARAM)&mla, ML_IPC_ADDTREEITEM);

	m_nMLTreeItem = mla.this_id;

	return TRUE;
}

INT_PTR CWinampAL::MLPluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	// check for any global messages here
	if (message_type >= ML_MSG_TREE_BEGIN && message_type <= ML_MSG_TREE_END)
	{
		if (param1 == wndWinampAL.m_nMLTreeItem)
		{
			// local messages for a tree item
			switch (message_type)
			{
			case ML_MSG_TREE_ONCREATEVIEW:
				if (m_bTabbedUI)
				{
					if (m_TabbedUI.Create(&CWnd((HWND)param2)))
					{
						m_TabbedUI.ShowWindow(SW_SHOW);
						return (int)m_TabbedUI.GetSafeHwnd();
					}
				}
				else
				{
					if (m_ConfigAboutML.Create(&CWnd((HWND)param2)))
					{
						m_ConfigAboutML.ShowWindow(SW_SHOW);
						return (int)m_ConfigAboutML.GetSafeHwnd();
					}
				}
				return 0;

			case ML_MSG_TREE_ONCLICK:
				if (param2 == ML_ACTION_RCLICK)
				{
					HWND hwndParent = (HWND)param3;

					CMenu menu;
					menu.CreatePopupMenu();

					menu.AppendMenu(MF_STRING, IDC_NEW_PROFILE, ALS("New Profile"));
					menu.AppendMenu(MF_STRING, IDC_PREFERENCE, ALS("Pre&ferences..."));

					POINT pt;
					GetCursorPos(&pt);
					DWORD dwFlags = TPM_RETURNCMD|TPM_NONOTIFY|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_LEFTBUTTON;
					UINT nRet = menu.TrackPopupMenu(dwFlags, pt.x, pt.y, &CWnd(hwndParent), NULL);

					menu.DestroyMenu();

					int nID = 0;
					CUserInterface *pUI = NULL;
					CProfileName dlgName;
					switch (nRet)
					{
					case IDC_NEW_PROFILE:
						if (dlgName.DoModal(&CWnd(hwndParent)) == IDOK)
						{
							while (wndWinampAL.IsProfileIDUsed(nID)) nID++;
							pUI = wndWinampAL.AddUserInterface(nID);
							pUI->SetProfileName(dlgName.GetName());
							wndWinamp.SendMLMessage(pUI->GetTreeItemID(), ML_IPC_SETCURTREEITEM);
						}
						break;
					case IDC_PREFERENCE:
						preference.ShowPage();
						break;
					}

					return 1;
				}
				return 0;
			}
		}
	}
	else if (message_type == ML_MSG_CONFIG)
	{
		preference.ShowPage();
		return 1;
	}

	// go through all user interfaces (profiles)
	int size = m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)m_UserInterfaces.GetAt(i);
		if (pUI)
		{
			INT_PTR nRet = pUI->MLPluginMessageProc(message_type, param1, param2, param3);

			if (nRet) return nRet;
		}
	}

	return 0;
}

int CWinampAL::GetTreeItemID()
{
	return m_nMLTreeItem;
}
