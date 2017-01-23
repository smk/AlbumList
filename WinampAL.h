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
#ifndef __WINAMP_AL_H__
#define __WINAMP_AL_H__

#include "Util.h"
#include "ALFront.h"
#include "ConfigAboutML.h"
#include "TabbedUI.h"

#define WINAMP_AL_CLASS		"Winamp AL"

enum ALSettingInt
{
	settingKeepSongPlaying = 1000,
	settingSortByFilename,
	settingResumePlayback,
	settingEnqueueDefault,
	settingEncodingLangAL,
	settingEncodingLangPE,
	settingLastConfig,
	settingLoop,
	settingShowCover,
	settingAutoAdvance,
	settingCompression,
	settingAdvanceRandom,
	settingVersionCheck,
	settingLogFile,
	settingBuiltInFileInfo,
	settingWinampFileInfo,
	settingFreeFormMP3,
	settingLightningBolt,
	settingVariousArtistRatio,
	settingEmbedML,
	settingEmbedMLSave,
	settingPlayAllWarning,
	settingPlayAllLimit,
	settingMSN,
	settingTabbedUI,
	settingTabbedUISave
};

enum ALSettingStr
{
	settingTranslation = 1200
};

enum ALLanguage
{
	langSystemDefault = 3000,
	langTraditionalChinese,
	langSimplifiedChinese,
	langJapanese,
	langKorean
};

class CIPCMenu
{
public:
	char szName[MAX_PATH];
	HWND hWnd;
	UINT uMsg;
};

//////////////////////////////////////////////////////////////////
// CWinampAL class

class CConfigAboutML;
class CUserInterface;
class CWinampAL : public CWnd
{
	friend class CTabbedUI;
public:
	CWinampAL();
	virtual ~CWinampAL();

	CPtrArray		m_UserInterfaces;			// list of profiles

	BOOL Create					();
	BOOL Startup				();
	BOOL Shutdown				();

	BOOL Archive				(BOOL bWrite);

	BOOL ProcessWinampMenu		(int nID);

	CUserInterface *AddUserInterface(int nID);
	BOOL RemoveUserInterface	(int nID);
	BOOL IsProfileIDUsed		(int nID);

	// set/get settings
	int  GetSetting				(ALSettingInt setting);
	void SetSetting				(ALSettingInt setting, int value);

	LPCTSTR GetSetting			(ALSettingStr setting);
	void SetSetting				(ALSettingStr setting, LPCTSTR value);

	int  GetCodepage			();
	int  GetEncodingCodepage	();

	void SetAutoAdvance			(CUserInterface *pUI);
	void SetFocus				(CUserInterface *pUI);

	// encoding functions
	int  DrawTextUnicode		(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat);

	BOOL OkToQuit				();
	void ResetFont				();
	void StopPressed			();
	void UpdateMSN				(LPCTSTR szFile);

	BOOL AddIPCMenu				(addStruct *as);
	BOOL GetIPCMenu				(CPtrArray **a);

	void RemoveFromMainMenu		(HMENU hMenu);
	void InsertToMainMenu		(HMENU hMenu);

	int  GetTreeItemID			();

	DWORD GetLatestVersion		();

protected:
	virtual int  OnCreate		(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnDestroy		();
	virtual BOOL OnEndSession	(BOOL bEnding);
	virtual BOOL OnQueryEndSession();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnFrontEnd		(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnEndOfSong	();
	virtual BOOL OnDeviceChange	(UINT nEventType, DWORD dwData);

	BOOL OnPostInit				();

private:
	BOOL			m_bInitialized;
	BOOL			m_bResumePlayback;			// resume playback
	int				m_nResumeTime;				// resume playback (track time)
	int				m_nResumePos;				// resume playback (playlist index)
	int				m_nResumePaused;			// resume playback (paused?)
	char			m_szResumeFile[MAX_PATH];	// resume playback (filename)
	BOOL			m_bKeepSongPlaying;			// keep song playing
	BOOL			m_bSortByFilename;			// sort by filename (not by track #)
	BOOL			m_bEnqueueDefault;			// enqueue when double-click/enter
	ALLanguage		m_EncodingLangAL;			// encoding language for the AL window
	ALLanguage		m_EncodingLangPE;			// encoding language for the PE window
	char			m_szTranslation[MAX_PATH];	// translation language pathname
	int				m_nTranslationCodepage;
	int				m_nLastConfig;				// last active config page index
	BOOL			m_bLoop;					// loop next/prev album
	BOOL			m_bShowCover;				// show cover in minibrowser
	BOOL			m_bAutoAdvance;				// automatically advance to next album
	BOOL			m_bAdvanceRandom;			// advance to a random album
	BOOL			m_bCompression;				// use compression in the cache file
	BOOL			m_bVersionCheck;			// check for new versions
	DWORD			m_tCheckTime;
	DWORD			m_dwLatestVer;
	BOOL			m_bLogFile;					// Log file
	BOOL			m_bBuiltInFileInfo;			// Use built-in file info class
	BOOL			m_bWinampFileInfo;			// Use winamp's file info
	BOOL			m_bFreeFormMP3;				// Get MP3 freeform reader
	BOOL			m_bLightningBolt;			// Override Lightning Bolt
	int				m_nVariousArtistRatio;		// Ratio [ 0 - 100 ]
	BOOL			m_bEmbedML;
	BOOL			m_bEmbedMLSave;
	BOOL			m_bPlayAllWarning;
	int				m_nPlayAllLimit;
	BOOL			m_bUpdateMSN;

	int				m_nAutoAdvanceID;			// Auto advance ID
	CUserInterface *m_pAutoAdvanceUI;			// user interface to advance
	int				m_nFocusID;					// Focus ID
	CUserInterface *m_pFocusUI;					// user interface in focus

	HANDLE			m_hThread;					// thread handle
	DWORD			m_dwThreadId;				// thread ID

	BOOL			m_bFirstTimer;				// First timer?

	int				m_nMLTreeItem;				// ML Tree Item

	BOOL			m_bTabbedUI;
	BOOL			m_bTabbedUISave;

	CMapStringToString m_AlbumColorMap;

	CPtrArray		m_IPCMenuList;
	CConfigAboutML	m_ConfigAboutML;
	CTabbedUI		m_TabbedUI;

	// IPC functions
	DWORD 	IPC_GetVersion		();

	BOOL	InitProfiles		();
	BOOL	InitML				();
	BOOL	InitEncoding		();

	// version checking stuff
	void	CheckForNewVersion	();
	BOOL	DownloadUpdate		(LPCTSTR url, LPCTSTR file);
	void	ProcessVerUpdate	(LPCTSTR file);
	static	DWORD WINAPI CheckVerThreadProc(LPVOID lpData);

public:
	winampMediaLibraryPlugin m_MediaLibraryPlugin;	// plugin struct for ML
	INT_PTR MLPluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);
};

#endif /* __WINAMP_AL_H__ */
