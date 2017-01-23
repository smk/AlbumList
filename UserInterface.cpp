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
#include <ShellAPI.h>
#include <ShlObj.H>
#include <CommCtrl.h>
#include <stdlib.h>
#include <ole2.h>
#include <olectl.h>
#include <dbt.h>
#include <mbstring.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "ConfigPath.h"
#include "ConfigNaming.h"
#include "ConfigCover.h"
#include "ConfigOption.h"
#include "ConfigLanguage.h"
#include "ConfigHistory.h"
#include "ConfigAbout.h"
#include "UserInterface.h"
#include "JumpToAlbum.h"
#include "CoolScroll.h"
#include "CoolSB_detours.h"
#include "Preference.h"
#include "ProfileName.h"
#include "AutoWide.h"
#include "AutoChar.h"

// {53944428-CD90-4c8b-ABD4-4C8A4C96CE00}
static const GUID WADLG_GUID = { 0x53944428, 0xcd90, 0x4c8b, { 0xab, 0xd4, 0x4c, 0x8a, 0x4c, 0x96, 0xce, 0x00 } };

// timer id
#define TIMERID_KEYINPUT	0xbeef

int IPC_LIBRARY_SENDTOMENU = 0;

HRESULT CreateDropSource(IDropSource **ppDropSource);
HRESULT CreateDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmeds, UINT count, IDataObject **ppDataObject);

#if (_WIN32_IE < 0x0500) // 14/03/2007 smk
//////////////////////////////////////////////////////////////////
// IDropTargetHelper
struct IDropTargetHelper : public IUnknown
{
    // IUnknown methods
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

    // IDropTargetHelper
    STDMETHOD (DragEnter)(THIS_ HWND hwndTarget, IDataObject* pDataObject,
                          POINT* ppt, DWORD dwEffect) PURE;
    STDMETHOD (DragLeave)(THIS) PURE;
    STDMETHOD (DragOver)(THIS_ POINT* ppt, DWORD dwEffect) PURE;
    STDMETHOD (Drop)(THIS_ IDataObject* pDataObject, POINT* ppt,
                     DWORD dwEffect) PURE;
    STDMETHOD (Show)(THIS_ BOOL fShow) PURE;
};

// {4657278A-411B-11d2-839A-00C04FD918D0}
extern "C" const GUID __declspec(selectany) CLSID_DragDropHelper = 
    { 0x4657278a, 0x411b, 0x11d2, { 0x83, 0x9a, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0xd0 }};

// {4657278B-411B-11d2-839A-00C04FD918D0}
extern "C" const GUID __declspec(selectany) IID_IDropTargetHelper = 
    { 0x4657278b, 0x411b, 0x11d2, { 0x83, 0x9a, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0xd0 }};
#endif // (_WIN32_IE < 0x0500)

//////////////////////////////////////////////////////////////////
// CUserInterface class

CUserInterface::CUserInterface(int id)
	: CDialog()
{
	m_bSaveCache			= FALSE;
	m_bSaveCoverCache		= FALSE;
	m_bInitialized			= FALSE;
	m_hFont					= NULL;
	m_hEmptyMask			= NULL;
	m_hImageList			= NULL;
	m_hImageListIcon		= NULL;
	m_nProfileID			= id;
	memset(&wa_wnd, 0, sizeof(embedWindowState));
	memset(m_szCacheFile, 0, MAX_PATH);
	memset(m_szProfileName, 0, MAX_PATH);
	m_hAccTable				= NULL;
	memset(&ntis, 0, sizeof(NxSThingerIconStruct));
	
	lstrcpy(m_ColumnWidths, "0=190 1=50");

	m_nMenuShow				= IDC_MENU_ALBUMLIST + id;
	m_nTimeDisplay			= 0;
	m_bAutoSizeColumns		= TRUE;
	m_wndWinampGen.m_pObj	= this;
	m_bShow					= TRUE;
	m_nLastConfig			= 0;
	m_bShowIndexNum			= TRUE;
	m_nShowFullPath			= TRUE;
	m_bAutoHideHeader		= FALSE;
	m_bShowHeader			= TRUE;
	m_bShowStatus			= TRUE;
	m_bShowLabel			= FALSE;
	m_nRandomDup			= 50;
	m_nKey					= 0;
	m_nMLTreeItem			= -1;
	m_bEmbedML				= FALSE;

	m_bCoverView			= FALSE;
	m_nCoverWidth			= 100;
	m_nCoverHeight			= 100;
	m_nBorderWidth			= 3;
	m_bCustomBorderColor	= TRUE;
	m_clrCoverBorder		= RGB(0,255,0);
	m_clrCoverText			= RGB(255,255,255);
	m_bCoverShadow			= FALSE;
	m_bDrawTitle			= TRUE;
	m_bCacheCovers			= FALSE;
	memset(m_szDefaultCover, 0, sizeof(m_szDefaultCover));
	wcscpy(m_szCoverSearchExt, L"*front*.jpg;*cover*.jpg;*folder*.jpg;*.jpg;*.jpeg;*.gif;*.bmp;*.png");
	m_bSearchFolder			= TRUE;
	m_bSearchAltFolder		= FALSE;
	m_bOverrideDefCover		= FALSE;
	m_bSearchMP3			= TRUE;
	memset(m_szAltFolder, 0, sizeof(m_szAltFolder));

	m_hCoverThread			= NULL;
	m_dwCoverThreadId		= 0;
	m_hCoverEvent			= NULL;
	m_bShuttingDown			= FALSE;

	// config path
	wcscpy(m_szExtList, L"MP3, MPC, MP+, MPP, WMA, WAV, APE, MAC, SPC, M4A");
	m_bStartupScan			= TRUE;
	m_bScanSubDir			= TRUE;
	m_bScanCDROM			= FALSE;
	m_bIgnorePL				= FALSE;
	m_bIgnoreRootPL			= FALSE;
	m_bPLOnly				= FALSE;

	// config naming
	m_nDirStyle				= 0;
    m_bFixTitles			= TRUE;
	m_bMultiDiscFix			= FALSE;
	m_bUseID3				= TRUE;
	wcscpy(m_szMultiDiscNames, L"cd, disc");
	m_bIgnoreTHE			= TRUE;

	// drag & drop
	m_pIDropHelper			= NULL;
	m_hGlobal				= NULL;
}

CUserInterface::~CUserInterface()
{
}

BOOL CUserInterface::Create()
{
	if (wndWinamp.IsWinamp531())
	{
		GUID guid = WADLG_GUID;
		guid.Data4[7] = m_nProfileID;
		SET_EMBED_GUID((&wa_wnd), guid);
	}

	CWnd wa2 = (HWND)wndWinamp.SendIPCMessage((WPARAM)&wa_wnd, IPC_GET_EMBEDIF);
	if (wa2.IsWindow())
	{
		return CDialog::Create(IDD_ALBUMLIST, &wa2);
	}
	return FALSE;
}

HWND CUserInterface::CreateInML(CWnd wndParent)
{
	m_bEmbedML = TRUE;
	if (CDialog::Create(IDD_ALBUMLIST, &wndParent))
	{
		return m_hWnd;
	}
	return NULL;
}

BOOL CUserInterface::OnRclickML(CWnd wndParent)
{
	CMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, IDC_PLAY, ALS("&Play"));
	menu.AppendMenu(MF_STRING, IDC_RENAME_PROFILE, ALS("&Rename"));
	menu.AppendMenu(MF_STRING, IDC_DELETE_PROFILE, ALS("&Delete"));
	menu.AppendMenu(MF_STRING, IDC_PREFERENCE, ALS("Pre&ferences..."));

	POINT pt;
	GetCursorPos(&pt);

	DWORD dwFlags = TPM_RETURNCMD|TPM_NONOTIFY|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_LEFTBUTTON;
	UINT nRet = menu.TrackPopupMenu(dwFlags, pt.x, pt.y, &wndParent, NULL);

	menu.DestroyMenu();

	int nIndex = 0;
	CProfileName dlgName;
	switch (nRet)
	{
	case IDC_PLAY:
		nIndex = m_AlbumList.GetCurAlbumIndex();
		if (nIndex != -1)
		{
			CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(nIndex);
			if (pAlbum)
			{
				pAlbum->Play();
				pAlbum->Release();
			}
		}
		else
		{
			CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(0);
			if (pAlbum)
			{
				pAlbum->Play();
				pAlbum->Release();
			}
		}
		return TRUE;
	case IDC_RENAME_PROFILE:
		dlgName.SetName(m_szProfileName);
		if (dlgName.DoModal(&wndParent) == IDOK)
		{
			SetProfileName(dlgName.GetName());
		}
		return TRUE;
	case IDC_DELETE_PROFILE:
		DestroyWindow();
		wndWinampAL.RemoveUserInterface(GetProfileID());
		return TRUE;
	case IDC_PREFERENCE:
		ShowPreference(wndParent, FALSE);
		return TRUE;
	}

	return FALSE;
}

int CUserInterface::GetTreeItemID()
{
	return m_nMLTreeItem;
}

BOOL CUserInterface::InitML(int nMLTreeItem)
{
	// check for media library support
	if (wndWinampAL.GetSetting(settingTabbedUI))
	{
		m_bEmbedML = TRUE;
		return TRUE;
	}
	else
	{
		long libhwndipc = (LONG)wndWinamp.SendIPCMessage(WPARAM("LibraryGetWnd"), IPC_REGISTER_WINAMP_IPCMESSAGE);
		if (libhwndipc)
		{
			// now send the message and get the HWND 
			CWnd wndML = (HWND)wndWinamp.SendIPCMessage(-1, LPARAM(libhwndipc));

			// add to the tree
			mlAddTreeItemStruct mla={ nMLTreeItem, (char*)GetMLTitle(), 1, };
			wndML.SendMessage(WM_ML_IPC, (WPARAM)&mla, ML_IPC_ADDTREEITEM);

			m_nMLTreeItem = mla.this_id;

			m_bEmbedML = TRUE;

			return TRUE;
		}
	}
	return FALSE;
}

int CUserInterface::GetMenuID()
{
	return m_nMenuShow;
}

int CUserInterface::GetProfileID()
{
	return m_nProfileID;
}

LPCTSTR CUserInterface::GetProfileName()
{
	return m_szProfileName;
}

void CUserInterface::SetProfileName(LPCTSTR name)
{
	lstrcpy(m_szProfileName, name);

	if (m_bEmbedML)
	{
		// add to the tree
		mlAddTreeItemStruct mla={ 0, (char*)GetMLTitle(), 0, m_nMLTreeItem, };
		wndWinamp.SendMLMessage((WPARAM)&mla, ML_IPC_SETTREEITEM);
	}
	else
	{
		// we set the window's title
		CWnd wndParent = GetParent();
		wndParent.SetWindowText(GetTitle());

		if (m_bShow)
		{
			IPC_ShowHide();
		}
	}
}

BOOL CUserInterface::Startup()
{
	m_bInitialized = TRUE;

	Archive(FALSE /*read*/);

	// request notification
	m_AlbumList.RequestNotification(Notify, this);

	// initialize the list
	wcsncpy(m_AlbumList.m_szExtList, m_szExtList, MAX_PATH);
	m_AlbumList.m_bScanSubDir		= m_bScanSubDir;
	m_AlbumList.m_bIgnorePL			= m_bIgnorePL;
	m_AlbumList.m_bIgnoreRootPL		= m_bIgnoreRootPL;
	m_AlbumList.m_bPLOnly			= m_bPLOnly;
	m_AlbumList.m_nDirStyle			= m_nDirStyle;
    m_AlbumList.m_bFixTitles		= m_bFixTitles;
	m_AlbumList.m_bMultiDiscFix		= m_bMultiDiscFix;
	m_AlbumList.m_bUseID3			= m_bUseID3;
	m_AlbumList.m_bIgnoreTHE		= m_bIgnoreTHE;
	m_AlbumList.m_nRandomDup		= m_nRandomDup;
	m_AlbumList.m_clrCoverText		= m_clrCoverText;
	m_AlbumList.m_bCoverShadow		= m_bCoverShadow;
	m_AlbumList.m_bDrawTitle		= m_bDrawTitle;
	m_AlbumList.m_bSearchFolder		= m_bSearchFolder;
	m_AlbumList.m_bSearchAltFolder	= m_bSearchAltFolder;
	m_AlbumList.m_bOverrideDefCover	= m_bOverrideDefCover;
	m_AlbumList.m_bSearchMP3		= m_bSearchMP3;
	wcsncpy(m_AlbumList.m_szMultiDiscNames, m_szMultiDiscNames, MAX_PATH);
	wcsncpy(m_AlbumList.m_szDefaultCover, m_szDefaultCover, MAX_PATH);
	wcsncpy(m_AlbumList.m_szCoverSearchExt, m_szCoverSearchExt, MAX_PATH);
	wcsncpy(m_AlbumList.m_szAltFolder, m_szAltFolder, MAX_PATH);

	m_AlbumList.Startup();

	// add search paths
	int size = m_SearchPaths.GetSize();
	for (int i=0; i<size; ++i)
	{
		m_AlbumList.AddPath(m_SearchPaths[i]);
	}

	// add CD-ROM
	if (m_bScanCDROM)
	{
		m_AlbumList.AddCDROMs();
	}

	// read cache file
	m_AlbumList.Load(m_szCacheFile);

	// start a quick scan
	if (m_bStartupScan)
	{
		m_AlbumList.QuickScan();
	}

	// set current album
	m_AlbumList.SetCurAlbum(m_AlbumList.GetCurAlbum());

	// load accelerator
	m_hAccTable = LoadAccelerators(hDllInstance, MAKEINTRESOURCE(IDR_ACCELERATOR_AL));

	// drag & drop support
	OleInitialize(NULL);
    CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (LPVOID*)&m_pIDropHelper);

	// Cover thread
	InitializeCriticalSection(&m_csCover);
	m_hCoverEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hCoverThread = CreateThread(NULL, 0, CoverThreadProc, this, 0, &m_dwCoverThreadId);
	SetThreadPriority(m_hCoverThread, THREAD_PRIORITY_BELOW_NORMAL);


	return TRUE;
}

BOOL CUserInterface::Shutdown()
{
	if (!m_bInitialized) return TRUE;

	if (m_hCoverThread)
	{
		m_bShuttingDown = TRUE;
		SetEvent(m_hCoverEvent);
		WaitForSingleObject(m_hCoverThread, 1000);
		TerminateThread(m_hCoverThread, 0);
		CloseHandle(m_hCoverThread);
	}

	if (m_hCoverEvent)
	{
		CloseHandle(m_hCoverEvent);
		m_hCoverEvent = NULL;
	}

	if (ntis.uIconId)
	{
		int iThingerIPC = wndWinamp.SendIPCMessage((WPARAM)&NXSTHINGER_MSGSTR, IPC_REGISTER_WINAMP_IPCMESSAGE);
		if (iThingerIPC > 65536)
		{
			ntis.dwFlags = NTIS_DELETE;
			wndWinamp.SendIPCMessage((WPARAM)&ntis, (LPARAM)iThingerIPC);
		}
	}

	// save the image cache for next session
	if ((m_hImageListIcon) && (m_bSaveCoverCache))
	{
		DbgPrint("Saving image list\n");
		SaveImageList(m_hImageListIcon);
	}

	// reset cover indexes
	m_AlbumList.RequestNotification(NULL, NULL);
	m_AlbumList.StopScan();
	if (m_bSaveCache)
	{
		DbgPrint("Saving cache %s\n", m_szCacheFile);
		m_AlbumList.Save(m_szCacheFile);
	}
	m_AlbumList.Shutdown();

	if (m_hImageList)	ImageList_Destroy(m_hImageList);
	m_hImageList = NULL;

	if (m_hImageListIcon)	ImageList_Destroy(m_hImageListIcon);
	m_hImageListIcon = NULL;

	Archive(TRUE /*write*/);

	if (m_pIDropHelper)
	{
		m_pIDropHelper->Release();
		m_pIDropHelper = NULL;
	}
	OleUninitialize();
	DeleteCriticalSection(&m_csCover);

	m_bInitialized = FALSE;

	return TRUE;
}

BOOL CUserInterface::Archive(BOOL bWrite)
{
	char profile[64];
	wsprintf(profile, "Profile %ld", m_nProfileID);

	if (bWrite)
	{
		char string[64];

		WritePrivateProfileString(profile, "CacheFile", m_szCacheFile, iniPlugin);
		WritePrivateProfileString(profile, "Name", m_szProfileName, iniPlugin);

		// write show
		wsprintf(string, "%ld", m_bShow);			WritePrivateProfileString(profile, "Show", string, iniPlugin);

		// write new position
		wsprintf(string, "%ld", wa_wnd.r.left);		WritePrivateProfileString(profile, "Left",	string, iniPlugin);
		wsprintf(string, "%ld", wa_wnd.r.top);		WritePrivateProfileString(profile, "Top",	string, iniPlugin);
		wsprintf(string, "%ld", wa_wnd.r.right);	WritePrivateProfileString(profile, "Right",	string, iniPlugin);
		wsprintf(string, "%ld", wa_wnd.r.bottom);	WritePrivateProfileString(profile, "Bottom",string, iniPlugin);

		// write current album id
		GUID riid = m_AlbumList.GetCurAlbum();
		wsprintf(string, "%08lX%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
			riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2],
			riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);
		WritePrivateProfileString(profile, "CurrentAlbum", string, iniPlugin);

		// write album sort
		memset(string, 0, 64);
		int size = m_AlbumList.m_SortOrders.GetSize();
		for (int i=0; i<size; ++i)
		{
			if (i != 0) lstrcat(string, " ");
			char str[16] = "";
			lstrcat(string, itoa(m_AlbumList.m_SortOrders[i], str, 10));
		}
		WritePrivateProfileString(profile, "SortOrder", string, iniPlugin);

		// write time display
		wsprintf(string, "%ld", m_nTimeDisplay);		WritePrivateProfileString(profile, "TimeDisplay", string, iniPlugin);

		// write auto size columns
		wsprintf(string, "%ld", m_bAutoSizeColumns);	WritePrivateProfileString(profile, "AutoSizeColumns", string, iniPlugin);
		wsprintf(string, "%ld", m_bAutoHideHeader);		WritePrivateProfileString(profile, "AutoHideHeader", string, iniPlugin);
		wsprintf(string, "%ld", m_bShowHeader);			WritePrivateProfileString(profile, "Show Header", string, iniPlugin);
		wsprintf(string, "%ld", m_bShowLabel);			WritePrivateProfileString(profile, "Show Label", string, iniPlugin);

		wsprintf(string, "%ld", m_nLastConfig);			WritePrivateProfileString(profile, "Last Config", string, iniPlugin);

		wsprintf(string, "%ld", m_bShowIndexNum);		WritePrivateProfileString(profile, "Show Index Num", string, iniPlugin);
		wsprintf(string, "%ld", m_nShowFullPath);		WritePrivateProfileString(profile, "Show Full Path", string, iniPlugin);

		wsprintf(string, "%ld", m_bShowStatus);			WritePrivateProfileString(profile, "Show Status", string, iniPlugin);

		wsprintf(string, "%ld", m_nCoverWidthSave);		WritePrivateProfileString(profile, "Cover Width", string, iniPlugin);
		wsprintf(string, "%ld", m_nCoverHeightSave);	WritePrivateProfileString(profile, "Cover Height", string, iniPlugin);
		wsprintf(string, "%ld", m_nBorderWidthSave);	WritePrivateProfileString(profile, "Border Width", string, iniPlugin);
		wsprintf(string, "%ld", m_clrCoverBorder);		WritePrivateProfileString(profile, "Cover Border Color", string, iniPlugin);
		wsprintf(string, "%ld", m_bCustomBorderColor);	WritePrivateProfileString(profile, "Custom Border Color", string, iniPlugin);
		wsprintf(string, "%ld", m_clrCoverText);		WritePrivateProfileString(profile, "Cover Text Color", string, iniPlugin);
		wsprintf(string, "%ld", m_bCoverShadow);		WritePrivateProfileString(profile, "Cover Draw Shadow", string, iniPlugin);
		wsprintf(string, "%ld", m_bDrawTitle);			WritePrivateProfileString(profile, "Draw Text Over Cover", string, iniPlugin);
		wsprintf(string, "%ld", m_bCacheCovers);		WritePrivateProfileString(profile, "Cache Covers", string, iniPlugin);
		WritePrivateProfileString(profile, "Default Cover", m_szDefaultCover, iniPlugin);
		WritePrivateProfileString(profile, "Cover Search", m_szCoverSearchExt, iniPlugin);

		wsprintf(string, "%ld", m_bCoverView);			WritePrivateProfileString(profile, "Cover View", string, iniPlugin);
		wsprintf(string, "%ld", m_bSearchFolder);		WritePrivateProfileString(profile, "Search Folder", string, iniPlugin);
		wsprintf(string, "%ld", m_bSearchAltFolder);	WritePrivateProfileString(profile, "Search Alt Folder", string, iniPlugin);
		wsprintf(string, "%ld", m_bOverrideDefCover);	WritePrivateProfileString(profile, "Override Default Cover", string, iniPlugin);
		wsprintf(string, "%ld", m_bSearchMP3);			WritePrivateProfileString(profile, "Search MP3", string, iniPlugin);
		WritePrivateProfileString(profile, "Alternate Folder", m_szAltFolder, iniPlugin);
		
		// write column widths
		CWnd wndHeader = m_wndList.GetDlgItem(0);
		if (wndHeader.GetSafeHwnd())
		{
			LockWindowUpdate(m_wndList);
			BOOL bCoverView = m_bCoverView;
			ListView();	// switch back to list view first

			memset(m_ColumnWidths, 0, 64);
			size = Header_GetItemCount(wndHeader);
			for (i=0; i<size; ++i)
			{
				if (i != 0) lstrcat(m_ColumnWidths, " ");

				int index = Header_OrderToIndex(wndHeader, i);

				char str[16] = "";
				HDITEM hdi;
				memset(&hdi, 0, sizeof(HDITEM));
				hdi.mask = HDI_WIDTH|HDI_LPARAM;
				wndHeader.SendMessage(HDM_GETITEM, index, (LPARAM)&hdi);
				wsprintf(str, "%ld=%ld", hdi.lParam, hdi.cxy);
				lstrcat(m_ColumnWidths, str);
			}

			if (bCoverView) CoverView();	// switch back to cover view
			LockWindowUpdate(NULL);
		}
		WritePrivateProfileString(profile, "ColumnWidths", m_ColumnWidths, iniPlugin);

		// config path
		size = m_SearchPaths.GetSize();
		for (i=0; i<size; ++i)
		{
			char paths[32];
			wsprintf(paths, "Path %ld", i);
			WritePrivateProfileString(profile, paths, m_SearchPaths[i], iniPlugin);
		}
		// delete the rest (if any)
		while (1)
		{
			char al_path[MAX_PATH];
			char paths[32];
			wsprintf(paths, "Path %ld", i++);
			DWORD dwRet = GetPrivateProfileString(profile, paths, "", al_path, MAX_PATH, iniPlugin);
			if (dwRet == 0)	break;
			WritePrivateProfileString(profile, paths, (LPCTSTR)NULL, iniPlugin);
		}

		WritePrivateProfileString(profile, "Extensions", m_szExtList, iniPlugin);
		wsprintf(string, "%ld", m_bStartupScan);	WritePrivateProfileString(profile, "Startup Quick Scan", string, iniPlugin);
		wsprintf(string, "%ld", m_bScanSubDir);		WritePrivateProfileString(profile, "Scan SubDir", string, iniPlugin);
		wsprintf(string, "%ld", m_bScanCDROM);		WritePrivateProfileString(profile, "Scan CDROM", string, iniPlugin);
		wsprintf(string, "%ld", m_bIgnorePL);		WritePrivateProfileString(profile, "Ignore Playlist", string, iniPlugin);
		wsprintf(string, "%ld", m_bIgnoreRootPL);	WritePrivateProfileString(profile, "Ignore Root Playlist", string, iniPlugin);
		wsprintf(string, "%ld", m_bPLOnly);			WritePrivateProfileString(profile, "Playlists Only", string, iniPlugin);

		// config naming
		wsprintf(string, "%ld", m_nDirStyle);		WritePrivateProfileString(profile, "Directory Style", string, iniPlugin);
		wsprintf(string, "%ld", m_bFixTitles);		WritePrivateProfileString(profile, "Fix Titles", string, iniPlugin);
		wsprintf(string, "%ld", m_bMultiDiscFix);	WritePrivateProfileString(profile, "Multi Disc Fix", string, iniPlugin);
		wsprintf(string, "%ld", m_bUseID3);			WritePrivateProfileString(profile, "Use ID3", string, iniPlugin);
		wsprintf(string, "%ld", m_bIgnoreTHE);		WritePrivateProfileString(profile, "Ignore THE", string, iniPlugin);
		WritePrivateProfileString(profile, "Multi Disc Names", m_szMultiDiscNames, iniPlugin);
	}
	else
	{
		// default cache file name
		char filename[MAX_PATH] = "Gen_m3a.dat";
		if (m_nProfileID)
			wsprintf(filename, "Gen_m3a%ld.dat", m_nProfileID);
		lstrcpy(m_szCacheFile, iniPlugin);
		PathRemoveFileSpec(m_szCacheFile);
		PathAppend(m_szCacheFile, filename);

		GetPrivateProfileString(profile, "CacheFile", m_szCacheFile, m_szCacheFile, MAX_PATH, iniPlugin);
		GetPrivateProfileString(profile, "Name", m_szProfileName, m_szProfileName, MAX_PATH, iniPlugin);

		m_bShow	= GetPrivateProfileInt(profile, "Show",	m_bShow, iniPlugin);

		m_nCoverWidth		= GetPrivateProfileInt(profile, "Cover Width",			m_nCoverWidth, iniPlugin);
		m_nCoverHeight		= GetPrivateProfileInt(profile, "Cover Height",			m_nCoverHeight, iniPlugin);
		m_nBorderWidth		= GetPrivateProfileInt(profile, "Border Width",			m_nBorderWidth, iniPlugin);
		m_bCustomBorderColor= GetPrivateProfileInt(profile, "Custom Border Color",	m_bCustomBorderColor, iniPlugin);
		m_clrCoverBorder	= GetPrivateProfileInt(profile, "Cover Border Color",	m_clrCoverBorder, iniPlugin);
		m_clrCoverText		= GetPrivateProfileInt(profile, "Cover Text Color",		m_clrCoverText, iniPlugin);
		m_bCoverShadow		= GetPrivateProfileInt(profile, "Cover Draw Shadow",	m_bCoverShadow, iniPlugin);
		m_bDrawTitle		= GetPrivateProfileInt(profile, "Draw Text Over Cover",	m_bDrawTitle, iniPlugin);
		m_bCacheCovers		= GetPrivateProfileInt(profile, "Cache Covers",			m_bCacheCovers, iniPlugin);
		m_bSearchFolder		= GetPrivateProfileInt(profile, "Search Folder",		m_bSearchFolder, iniPlugin);
		m_bSearchAltFolder	= GetPrivateProfileInt(profile, "Search Alt Folder",	m_bSearchAltFolder, iniPlugin);
		m_bOverrideDefCover	= GetPrivateProfileInt(profile, "Override Default Cover",	m_bOverrideDefCover, iniPlugin);
		m_bSearchMP3		= GetPrivateProfileInt(profile, "Search MP3",			m_bSearchMP3, iniPlugin);
		GetPrivateProfileString(profile, "Default Cover", m_szDefaultCover, m_szDefaultCover, MAX_PATH, iniPlugin);
		GetPrivateProfileString(profile, "Cover Search", m_szCoverSearchExt, m_szCoverSearchExt, MAX_PATH, iniPlugin);
		GetPrivateProfileString(profile, "Alternate Folder", m_szAltFolder, m_szAltFolder, MAX_PATH, iniPlugin);

		m_nCoverWidthSave = m_nCoverWidth;
		m_nCoverHeightSave = m_nCoverHeight;
		m_nBorderWidthSave = m_nBorderWidth;

		m_bCoverView	= GetPrivateProfileInt(profile, "Cover View",			m_bCoverView, iniPlugin);

		// get previous position
		wa_wnd.r.left	= GetPrivateProfileInt(profile, "Left",		0,	iniPlugin);
		wa_wnd.r.top	= GetPrivateProfileInt(profile, "Top",		0,	iniPlugin);
		wa_wnd.r.right	= GetPrivateProfileInt(profile, "Right",	0,	iniPlugin);
		wa_wnd.r.bottom	= GetPrivateProfileInt(profile, "Bottom",	0,	iniPlugin);

		// read current album id
		char id[64];
		GetPrivateProfileString(profile, "CurrentAlbum", "00000000000000000000000000000000", id, 64, iniPlugin);
		GUID riid;
		int d1, d2, d3, d40, d41, d42, d43, d44, d45, d46, d47;
		sscanf(id, "%08lX%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
			&d1, &d2, &d3, &d40, &d41, &d42, &d43, &d44, &d45, &d46, &d47);

		riid.Data1		= d1;
		riid.Data2		= d2;
		riid.Data3		= d3;
		riid.Data4[0]	= d40;
		riid.Data4[1]	= d41;
		riid.Data4[2]	= d42;
		riid.Data4[3]	= d43;
		riid.Data4[4]	= d44;
		riid.Data4[5]	= d45;
		riid.Data4[6]	= d46;
		riid.Data4[7]	= d47;

		m_AlbumList.SetCurAlbum(riid);

		// read album sort
		char szSortOrder[256];
		GetPrivateProfileString(profile, "SortOrder", "", szSortOrder, 256, iniPlugin);
		CTokens order(szSortOrder, " ");
		for (int i=0; i<order.GetSize(); ++i)
		{
			m_AlbumList.m_SortOrders.Add(atoi(order[i]));
		}

		// read time display
		m_nTimeDisplay	= GetPrivateProfileInt(profile, "TimeDisplay",	m_nTimeDisplay,	iniPlugin);

		// read auto size columns
		m_bAutoSizeColumns	= GetPrivateProfileInt(profile, "AutoSizeColumns",	m_bAutoSizeColumns,	iniPlugin);
		m_bAutoHideHeader	= GetPrivateProfileInt(profile, "AutoHideHeader", m_bAutoHideHeader, iniPlugin);
		m_bShowHeader		= GetPrivateProfileInt(profile, "Show Header", m_bShowHeader, iniPlugin);
		m_bShowLabel		= GetPrivateProfileInt(profile, "Show Label", m_bShowLabel, iniPlugin);

		m_nLastConfig	= GetPrivateProfileInt(profile, "Last Config",	m_nLastConfig,	iniPlugin);

		m_bShowIndexNum	= GetPrivateProfileInt(profile, "Show Index Num", m_bShowIndexNum, iniPlugin);
		m_nShowFullPath	= GetPrivateProfileInt(profile, "Show Full Path", m_nShowFullPath, iniPlugin);

		m_bShowStatus	= GetPrivateProfileInt(profile, "Show Status", m_bShowStatus, iniPlugin);

		m_nRandomDup	= GetPrivateProfileInt(profile, "Random Dup", m_nRandomDup, iniPlugin);

		// read column widths
		GetPrivateProfileString(profile, "ColumnWidths", m_ColumnWidths, m_ColumnWidths, 64, iniPlugin);

		// config path
		i = 0;
		while (1)
		{
			wchar_t al_path[MAX_PATH] = L"";
			char paths[40] = "al_path";
			wsprintf(paths, "Path %ld", i++);
			DWORD dwRet = GetPrivateProfileString(profile, paths, L"", al_path, MAX_PATH, iniPlugin);
			if (dwRet == 0 && i != 0)	break;
			if (wcslen(al_path))
			{
				m_SearchPaths.Add(al_path);
			}
		}

		GetPrivateProfileString(profile, "Extensions", m_szExtList, m_szExtList, MAX_PATH, iniPlugin);
		m_bStartupScan	= GetPrivateProfileInt(profile, "Startup Quick Scan", m_bStartupScan, iniPlugin);
		m_bScanSubDir	= GetPrivateProfileInt(profile, "Scan SubDir", m_bScanSubDir, iniPlugin);
		m_bScanCDROM	= GetPrivateProfileInt(profile, "Scan CDROM", m_bScanCDROM, iniPlugin);
		m_bIgnorePL		= GetPrivateProfileInt(profile, "Ignore Playlist", m_bIgnorePL, iniPlugin);
		m_bIgnoreRootPL	= GetPrivateProfileInt(profile, "Ignore Root Playlist", m_bIgnoreRootPL, iniPlugin);
		m_bPLOnly		= GetPrivateProfileInt(profile, "Playlists Only", m_bPLOnly, iniPlugin);

		// config naming
		m_nDirStyle		= GetPrivateProfileInt(profile, "Directory Style", m_nDirStyle, iniPlugin);
		m_bFixTitles	= GetPrivateProfileInt(profile, "Fix Titles", m_bFixTitles, iniPlugin);
		m_bMultiDiscFix	= GetPrivateProfileInt(profile, "Multi Disc Fix", m_bMultiDiscFix, iniPlugin);
		m_bUseID3		= GetPrivateProfileInt(profile, "Use ID3", m_bUseID3, iniPlugin);
		m_bIgnoreTHE	= GetPrivateProfileInt(profile, "Ignore THE", m_bIgnoreTHE, iniPlugin);
		GetPrivateProfileString(profile, "Multi Disc Names", m_szMultiDiscNames, m_szMultiDiscNames, MAX_PATH, iniPlugin);
	}

	return TRUE;
}

BOOL CUserInterface::ProcessWinampMenu(int nID)
{
	if (nID == m_nMenuShow)
	{
		if (m_bShow)
		{
			GetParent().ShowWindow(SW_HIDE);
			m_bShow = FALSE;
		}
		else
		{
			::ShowWindow(wa_wnd.me, SW_SHOW);
			m_bShow = TRUE;
		}
		return TRUE;
	}

	return FALSE;
}

LPCTSTR CUserInterface::GetTitle()
{
	lstrcpy(m_szTitle, "Album List");
	if (lstrlen(m_szProfileName))
	{
		wsprintf(m_szTitle, "AL - %s", m_szProfileName);
	}
	else if (m_nProfileID)
	{
		wsprintf(m_szTitle, "Album List - %ld", m_nProfileID);
	}

	return m_szTitle;
}

LPCTSTR CUserInterface::GetMLTitle()
{
	if (lstrlen(m_szProfileName))
	{
		lstrcpy(m_szTitle, m_szProfileName);
	}
	else
	{
		wsprintf(m_szTitle, ALS("Profile %ld"), m_nProfileID);
	}

	return m_szTitle;
	
}

void CUserInterface::Cleanup()
{
	if (m_bEmbedML)
	{
		// set to a different page before deleting
		int nCurTreeItemID = wndWinamp.SendMLMessage(0, ML_IPC_GETCURTREEITEM);
		if (nCurTreeItemID == m_nMLTreeItem)
		{
			wndWinamp.SendMLMessage(wndWinampAL.GetTreeItemID(), ML_IPC_SETCURTREEITEM);
		}
		wndWinamp.SendMLMessage(m_nMLTreeItem, ML_IPC_DELTREEITEM);
	}
	else
	{
		HMENU hMenu = (HMENU)wndWinamp.SendIPCMessage(0, IPC_GET_HMENU);
		RemoveMenu(hMenu, m_nMenuShow, MF_BYCOMMAND);
	}
}

void CALLBACK CUserInterface::Notify(ListNotification ln, WPARAM wParam, LPARAM lParam, LPVOID pData)
{
	if (pData == NULL) return;
	CUserInterface *pUI = (CUserInterface *)pData;

	switch (ln)
	{
	case notifyListLoaded:
		pUI->ListSizeChanged();
		break;

	case notifyListSizeChanged:
		pUI->ListSizeChanged();
		pUI->m_bSaveCache = TRUE;
		pUI->m_bSaveCoverCache = TRUE;
		break;

	case notifyAlbumChanged:
		pUI->m_bSaveCache = TRUE;
		break;

	case notifyCoverChanged:
		pUI->m_bSaveCoverCache = TRUE;
		break;

	case notifyContentChanged:
		pUI->Redraw();
		break;

	case notifyCurAlbumChanged:
		pUI->CurAlbumChanged(wParam);
		break;

	case notifyFinishedQuickScan:
		pUI->FinishedQuickScan();
	}
}

BOOL CUserInterface::OnInitDialog()
{
	// call the internal window initialisation - makes sure that
	// the internal controls will be winamp styled
	WADlg_init(wndWinamp);

	if (!m_bEmbedML) 
	{
		CWnd wndParent = GetParent();

		// subclass winamp gen window
		m_wndWinampGen.SubclassWindow(wndParent);

		// here we set the window's title
		wndParent.SetWindowText(GetTitle());
	}

	// hide buttons?
	if (!m_bShowStatus)
	{
		GetDlgItem(IDC_PLAYBTN).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ENQUEUEBTN).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TOTALALBUMTIME).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PREVALBUMBTN).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NEXTALBUMBTN).ShowWindow(SW_HIDE);
	}

	m_wndList.SubclassWindow(GetDlgItem(IDC_ALBUMLIST));
	if (m_wndList)
	{
		if (m_bCoverView)	CoverView();
		else				ListView();

		// set icons for the 'cover' view
		if (m_bShowLabel)
		{
			ListView_SetIconSpacing(m_wndList, m_nCoverWidth + 3, m_nCoverHeight + 40);
		}
		else
		{
			ListView_SetIconSpacing(m_wndList, m_nCoverWidth + 3, m_nCoverHeight + 3);
		}

		// create mask for use in ImageList_Add
		SIZE size = { m_nCoverWidth, m_nCoverHeight };
		CreateEmptyMask(size);

		if (m_hImageListIcon == NULL)
		{
			if ((m_hImageListIcon = LoadImageList()) == NULL)
			{
				// reset all album's icon index
				int size = m_AlbumList.GetSize();
				for (int i=0; i<size; i++)
				{
					CAlbum *p = (CAlbum *)m_AlbumList.GetAlbum(i);
					if (p)
					{
						p->SetIconIndex(-1);
						p->Release();
						m_bSaveCache = TRUE;
					}
				}

				// create image list
				m_hImageListIcon = CreateImageList();
			}
		}

		ListView_SetImageList(m_wndList, m_hImageListIcon, LVSIL_NORMAL);
		ListView_SetCallbackMask(m_wndList, LVIS_OVERLAYMASK);
		
		m_wndList.m_pUI = this;

		m_wndHeader.SubclassWindow(m_wndList.GetDlgItem(0));
		m_wndHeader.m_pUI = this;

		// resize the listctrl
		LayoutControls();

		// configure the custom scroll bars
		if (bWinNT)
		{
			InitializeCoolSB(m_wndList);
			CoolSB_SetSize(m_wndList, SB_BOTH, 14, 14);
		}

		CreateListFont();

		DWORD dwFlags = LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT|0x00010000/*LVS_EX_DOUBLEBUFFER*/|LVS_EX_INFOTIP;
		ListView_SetBkColor(m_wndList, WADlg_getColor(WADLG_ITEMBG));
		ListView_SetTextColor(m_wndList, WADlg_getColor(WADLG_ITEMFG));
		ListView_SetTextBkColor(m_wndList, WADlg_getColor(WADLG_ITEMBG));
		ListView_SetExtendedListViewStyleEx(m_wndList, dwFlags, dwFlags);
		ListView_SetItemCountEx(m_wndList, m_AlbumList.GetSize(), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

		// add columns from last session
		CTokens columns(m_ColumnWidths, " ");
		BOOL bColumnAdded = FALSE;
		for (int i=0; i<columns.GetSize(); ++i)
		{
			int lParam, nWidth;
			sscanf(columns[i], "%ld=%ld", &lParam, &nWidth);

			switch (lParam)
			{
			case columnName:
				InsertColumn(m_wndList, ALS("Name"), nWidth, LVCFMT_LEFT,  columnName, TRUE/*insertlast*/);
				bColumnAdded = TRUE;
				break;

			case columnTime:
				InsertColumn(m_wndList, ALS("Time"), nWidth,  LVCFMT_RIGHT, columnTime, TRUE/*insertlast*/);
				bColumnAdded = TRUE;
				break;

			case columnYear:
				InsertColumn(m_wndList, ALS("Year"), nWidth,  LVCFMT_RIGHT, columnYear, TRUE/*insertlast*/);
				bColumnAdded = TRUE;
				break;

			case columnTrack:
				InsertColumn(m_wndList, ALS("Tracks"), nWidth,  LVCFMT_RIGHT, columnTrack, TRUE/*insertlast*/);
				bColumnAdded = TRUE;
				break;

			case columnPath:
				InsertColumn(m_wndList, ALS("Path"), nWidth,  LVCFMT_LEFT, columnPath, TRUE/*insertlast*/);
				bColumnAdded = TRUE;
				break;

			case columnArtist:
				InsertColumn(m_wndList, ALS("Artist"), nWidth,  LVCFMT_LEFT, columnArtist, TRUE/*insertlast*/);
				bColumnAdded = TRUE;
				break;

			case columnAlbum:
				InsertColumn(m_wndList, ALS("Album"), nWidth,  LVCFMT_LEFT, columnAlbum, TRUE/*insertlast*/);
				bColumnAdded = TRUE;
				break;

			case columnGenre:
				InsertColumn(m_wndList, ALS("Genre"), nWidth,  LVCFMT_LEFT, columnGenre, TRUE/*insertlast*/);
				bColumnAdded = TRUE;
				break;

			case columnMultiDisc:
				InsertColumn(m_wndList, ALS("MultiDisc"), nWidth, LVCFMT_LEFT, columnMultiDisc, TRUE/*insertlast*/);
				break;
			}
		}

		// we should at least have the name column
		if (!bColumnAdded)
		{
			InsertColumn(m_wndList, "Name", 190, LVCFMT_LEFT,  columnName);
		}

		if (m_bAutoHideHeader || !m_bShowHeader)
		{
			// hide header
			m_wndList.ModifyStyle(0, LVS_NOCOLUMNHEADER);
			AutoSizeColumns();
		}

		// update current album
		int index = m_AlbumList.GetCurAlbumIndex();
		EnsureVisible(index);
	}

	UpdateStatus();

	// shows the owner window and this dialog
	if (!m_bEmbedML) ShowWindow(SW_SHOWNORMAL);

	RegisterDragDrop(GetSafeHwnd(), this);

	PostMessage(WM_AL_POSTINIT);
	
	return TRUE;
}

BOOL CUserInterface::OnPostInit()
{
	// show the window
	if (!m_bEmbedML && m_bShow) GetParent().ShowWindow(SW_SHOWNORMAL);

	// Add NxS Thinger Icon
	if (!m_bEmbedML)
	{
		int iThingerIPC = wndWinamp.SendIPCMessage((WPARAM)&NXSTHINGER_MSGSTR, IPC_REGISTER_WINAMP_IPCMESSAGE);
		if (iThingerIPC > 65536)
		{
			ntis.dwFlags = NTIS_ADD|NTIS_BITMAP;
			ntis.lpszDesc = (LPTSTR) GetTitle();
			ntis.hBitmap = LoadBitmap(hDllInstance, MAKEINTRESOURCE(IDB_ICON_AL));
			ntis.hBitmapHighlight = LoadBitmap(hDllInstance, MAKEINTRESOURCE(IDB_ICON_AL_H));
			ntis.hWnd = GetSafeHwnd();
			ntis.uMsg = WM_AL_IPC;
			ntis.wParam = IPC_SHOWHIDE;
			ntis.lParam = 0;
			ntis.uIconId = wndWinamp.SendIPCMessage((WPARAM)&ntis, (LPARAM)iThingerIPC);
		}
	}

	// Get the Library SendTo Menu API
	IPC_LIBRARY_SENDTOMENU = wndWinamp.SendIPCMessage((WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);

	if (m_bAutoSizeColumns)
		AutoSizeColumns();

	// update current album
	int index = m_AlbumList.GetCurAlbumIndex();
	EnsureVisible(index);

	if (m_bEmbedML) wndWinampAL.SetFocus(this);

	return TRUE;
}

BOOL CUserInterface::OnPaint(HDC hdc)
{
	// this is a list of controls to winampise
	int tabs[] = {IDC_ALBUMLIST|DCW_SUNKENBORDER};

	WADlg_DrawChildWindowBorders(GetSafeHwnd(),tabs,sizeof(tabs)/sizeof(tabs[0]));

	return TRUE;
}

BOOL CUserInterface::OnDeviceChange(UINT nEventType, DWORD dwData)
{
	DEV_BROADCAST_VOLUME *hdr = (DEV_BROADCAST_VOLUME *)dwData;

	switch (nEventType)
	{
	case DBT_DEVICEQUERYREMOVE:
	case DBT_DEVICEREMOVECOMPLETE:
		if (hdr->dbcv_devicetype == DBT_DEVTYP_VOLUME)
		{
			int pos = (int)wndWinamp.SendIPCMessage(0, IPC_GETLISTPOS);
			char *name = (char *)wndWinamp.SendIPCMessage(pos, IPC_GETPLAYLISTFILE);

			if (name != NULL)
			{
				
				DWORD unitmask = 1 << (toupper(name[0]) - 'A');

				if (hdr->dbcv_unitmask == unitmask)
				{
					// stop the player
					wndWinamp.SendMessage(WM_COMMAND, WINAMP_BUTTON4);

				}
			}
			int i=0;
			while (hdr->dbcv_unitmask = (hdr->dbcv_unitmask >> 1)) i++;

			DbgPrint("CD-ROM removed from drive %c\n", i + 'A');

			if (m_bScanCDROM)
			{
				wchar_t str[32];
				swprintf(str, L"%c:\\", i + L'A');
				m_AlbumList.CleanupPath(str);
			}
		}
		return TRUE;

	case DBT_DEVICEARRIVAL:
		// is this a volume change?
		if (hdr->dbcv_devicetype == DBT_DEVTYP_VOLUME)
		{
			//ANDREAS: for some reason the 32nd bit was set on my machine. Should be zero
			hdr->dbcv_unitmask &= 0x7fffff;

			char i=0;
			while (hdr->dbcv_unitmask = (hdr->dbcv_unitmask >> 1)) i++;

			DbgPrint("CD-ROM inserted in drive %c\n", i + 'A');

			if (m_bScanCDROM)
			{
				wchar_t str[32];
				swprintf(str, L"%c:\\", i + L'A');
				m_AlbumList.ScanPath(str);
			}
		}
		return TRUE;
	}
	return FALSE;
}

void CUserInterface::Search(LPCTSTR szInputBufferA)
{
	DbgPrint("CUserInterface::Search(%s)\n", szInputBufferA);

	int size = ListView_GetItemCount(m_wndList);
	int nCurSel = ListView_GetSelectionMark(m_wndList);

	AutoWide szInputBuffer(szInputBufferA);

	int len = wcslen(szInputBuffer);
	if (nCurSel == -1) nCurSel = 0;

	// find first searchable column
	int searchColumn = columnName;
	CWnd wndHeader = m_wndList.GetDlgItem(0);
	int szHeader = Header_GetItemCount(wndHeader);
	for (int i=0; i<szHeader; i++)
	{
		int index = Header_OrderToIndex(wndHeader, i);

		HDITEM hdi;
		memset(&hdi, 0, sizeof(HDITEM));
		hdi.mask = HDI_LPARAM;
		wndHeader.SendMessage(HDM_GETITEM, index, (LPARAM)&hdi);
		if ((hdi.lParam == columnName) ||
			(hdi.lParam == columnAlbum) ||
			(hdi.lParam == columnArtist))
		{
			searchColumn = hdi.lParam;
			break;
		}
	}

	BOOL bFound = FALSE;
	while (!bFound)
	{
		// 1. search string still good for the current album?
		if (len > 1)
		{
			LPCWSTR szColumnText = NULL;

			CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(nCurSel);
			if (pAL)
			{
				switch (searchColumn)
				{
				case columnAlbum:	szColumnText = pAL->GetAlbum(); break;
				case columnArtist:	szColumnText = pAL->GetArtist(); break;
				case columnName:
				default:			szColumnText = pAL->GetTitle(); break;
				}
				pAL->Release();
			}

			if (_wcsnicmp(szInputBuffer, szColumnText, len) == 0)
			{
				bFound = TRUE;
			}
		}

		// 2. search from the next position
		if (!bFound)
		{
			for (int i=nCurSel+1; i<size; i++)
			{
				LPCWSTR szColumnText = NULL;

				CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(i);
				if (pAL)
				{
					switch (searchColumn)
					{
					case columnAlbum:	szColumnText = pAL->GetAlbum(); break;
					case columnArtist:	szColumnText = pAL->GetArtist(); break;
					case columnName:
					default:			szColumnText = pAL->GetTitle(); break;
					}
					pAL->Release();
				}

				if (_wcsnicmp(szInputBuffer, szColumnText, len) == 0)
				{
					m_wndList.SetCurSel(i);
					bFound = TRUE;
					break;
				}
			}
		}

		// 3. search from the beginning again
		if (!bFound)
		{
			for (int i=0; i<nCurSel; i++)
			{
				LPCWSTR szColumnText = NULL;

				CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(i);
				if (pAL)
				{
					switch (searchColumn)
					{
					case columnAlbum:	szColumnText = pAL->GetAlbum(); break;
					case columnArtist:	szColumnText = pAL->GetArtist(); break;
					case columnName:
					default:			szColumnText = pAL->GetTitle(); break;
					}
					pAL->Release();
				}

				if (_wcsnicmp(szInputBuffer, szColumnText, len) == 0)
				{
					m_wndList.SetCurSel(i);
					bFound = TRUE;
					break;
				}
			}
		}

		// 3. try to do the search with 1 less character
		if (!bFound)
		{
			len --;

			// quit the loop if we tried all combinations 
			// and pretend we found something
			if (len == 0) bFound = TRUE;
		}
	}
}

BOOL CUserInterface::OnSize(UINT nType, int cx, int cy)
{
	LayoutControls();
	return TRUE;
}

void CUserInterface::LayoutControls()
{
	RECT rc;
	GetClientRect(&rc);

	int nGap		= 0;
	int nBtnHeight	= 0;
	int nVGap		= m_bEmbedML ? 3 : 0;

	HDWP hWinPosInfo = BeginDeferWindowPos(7);

	if (m_bShowStatus)
	{
		RECT rcPlay;
		CWnd btnPlay  = GetDlgItem(IDC_PLAYBTN);
		btnPlay.GetWindowRect(&rcPlay);
		ScreenToClient(&rcPlay);
		nBtnHeight	= rcPlay.bottom - rcPlay.top;
		nGap		= 4;

		// play button
		int left = 0;
		if (hWinPosInfo)
			hWinPosInfo = btnPlay.DeferWindowPos(hWinPosInfo, NULL, left, rc.bottom - nBtnHeight - nVGap, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

		// enqueue button
		RECT rcEnqueue;
		CWnd btnEnqueue  = GetDlgItem(IDC_ENQUEUEBTN);
		btnEnqueue.GetWindowRect(&rcEnqueue);

		left += rcPlay.right - rcPlay.left + 4/*gap*/;
		if (hWinPosInfo)
			hWinPosInfo = btnEnqueue.DeferWindowPos(hWinPosInfo, NULL, left, rc.bottom - nBtnHeight - nVGap, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

		// prev button
		RECT rcPrev;
		CWnd btnPrev  = GetDlgItem(IDC_PREVALBUMBTN);
		btnPrev.GetWindowRect(&rcPrev);

		left += rcEnqueue.right - rcEnqueue.left + 4/*gap*/;
		if (hWinPosInfo)
			hWinPosInfo = btnPrev.DeferWindowPos(hWinPosInfo, NULL, left, rc.bottom - nBtnHeight - nVGap, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

		// prev button
		RECT rcNext;
		CWnd btnNext  = GetDlgItem(IDC_NEXTALBUMBTN);
		btnNext.GetWindowRect(&rcNext);

		left += rcPrev.right - rcPrev.left + 4/*gap*/;
		if (hWinPosInfo)
			hWinPosInfo = btnNext.DeferWindowPos(hWinPosInfo, NULL, left, rc.bottom - nBtnHeight - nVGap, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

		// status
		CWnd stsTotal  = GetDlgItem(IDC_TOTALALBUMTIME);

		left += rcNext.right - rcNext.left + 4/*gap*/;
		if (hWinPosInfo)
			hWinPosInfo = stsTotal.DeferWindowPos(hWinPosInfo, NULL, left, rc.bottom - nBtnHeight - nVGap, rc.right - left, nBtnHeight, SWP_NOZORDER|SWP_NOACTIVATE);
	}

	CWnd wndList = GetDlgItem(IDC_ALBUMLIST);

	// get old width of list control
	// auto size column first if we're going smaller
	// to prevent the scrollbar from showing up
	// for a brief second
	RECT rcOldW, rcOldC;
	wndList.GetWindowRect(&rcOldW);
	wndList.GetClientRect(&rcOldC);
	int scrollWidth = rcOldW.right - rcOldW.left - rcOldC.right;
	if (m_bAutoSizeColumns && (rc.right-1 < rcOldW.right-rcOldW.left))
	{
		AutoSizeColumns(rc.right-1-scrollWidth);
	}

	if (hWinPosInfo)
		hWinPosInfo = wndList.DeferWindowPos(hWinPosInfo, NULL, 0, 0, rc.right-1, rc.bottom - nBtnHeight - 2 - nVGap, SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);

	if (hWinPosInfo)
		EndDeferWindowPos(hWinPosInfo);

	// auto size column after if we're going bigger
	if (m_bAutoSizeColumns && (rc.right > rcOldW.right-rcOldW.left))
	{
		AutoSizeColumns();
	}
}

void CUserInterface::ReloadAlbumList()
{
	EnterCriticalSection(&m_csCover);

	// clear the cover array
	int size = m_AlbumList.GetSize();
	for (int i=size-1; i>=0; i--)
	{
		m_CoverArray.Add(i);
	}

	m_AlbumList.Reload();

	if (m_hImageListIcon)	ImageList_Destroy(m_hImageListIcon);
	m_hImageListIcon = NULL;

	m_hImageListIcon = CreateImageList();

	ListView_SetImageList(m_wndList, m_hImageListIcon, LVSIL_NORMAL);
	ListView_SetCallbackMask(m_wndList, LVIS_OVERLAYMASK);

	LeaveCriticalSection(&m_csCover);
}

BOOL CUserInterface::OnDisplayChange(int cBitsPerPixel, int cx, int cy)
{
	WADlg_init(wndWinamp);

	// this is a list of controls to winampise
	int tabs[] = {IDC_ALBUMLIST|DCW_SUNKENBORDER};
	WADlg_DrawChildWindowBorders(GetSafeHwnd(),tabs,sizeof(tabs)/sizeof(tabs[0]));

	ListView_SetBkColor(GetDlgItem(IDC_ALBUMLIST), WADlg_getColor(WADLG_ITEMBG));
	ListView_SetTextColor(GetDlgItem(IDC_ALBUMLIST), WADlg_getColor(WADLG_ITEMFG));
	ListView_SetTextBkColor(GetDlgItem(IDC_ALBUMLIST), WADlg_getColor(WADLG_ITEMBG));

	CreateListFont();

	// insert cover mask
	SIZE size = { m_nCoverWidth, m_nCoverHeight };
	HBITMAP hBitmapMask = CreateOverlayImage(size);
	if (hBitmapMask)
	{
		int i = ImageList_AddMasked(m_hImageListIcon, hBitmapMask, RGB(0,0,0));
		ImageList_SetOverlayImage(m_hImageListIcon, i, 1);
		DeleteObject(hBitmapMask);
	}

	// if winamp changes skin then it sends a dummy WM_DISPLAYCHANGE
	// message. on receive do a quick hide and reshow
	// (simple hack to ensure the window is shown correctly with
	//  the newly loaded skin)
	if (!cBitsPerPixel && !cx && !cy)
	{
		ShowWindow(SW_HIDE);
		ShowWindow(SW_SHOW);
	}
	return TRUE;
}

BOOL CUserInterface::OnClose()
{
	if (!m_bEmbedML) GetParent().ShowWindow(SW_HIDE);
	m_bShow = FALSE;
	return TRUE;
}

BOOL CUserInterface::OnDestroy()
{
	RevokeDragDrop(GetSafeHwnd());

	// update column width here also
	// in archive, it may be too late
	// especially in ml mode
	CWnd wndHeader = m_wndList.GetDlgItem(0);
	if (wndHeader.GetSafeHwnd())
	{
		BOOL bCoverView = m_bCoverView;
		ListView();

		memset(m_ColumnWidths, 0, 64);
		int size = Header_GetItemCount(wndHeader);
		for (int i=0; i<size; ++i)
		{
			if (i != 0) lstrcat(m_ColumnWidths, " ");

			int index = Header_OrderToIndex(wndHeader, i);

			char str[16] = "";
			HDITEM hdi;
			memset(&hdi, 0, sizeof(HDITEM));
			hdi.mask = HDI_WIDTH|HDI_LPARAM;
			wndHeader.SendMessage(HDM_GETITEM, index, (LPARAM)&hdi);
			wsprintf(str, "%ld=%ld", hdi.lParam, hdi.cxy);
			lstrcat(m_ColumnWidths, str);
		}

		if (bCoverView) CoverView();
	}

	if (m_hFont)		DeleteObject(m_hFont);
	m_hFont = NULL;

	if (m_hEmptyMask)	DeleteObject(m_hEmptyMask);
	m_hEmptyMask = NULL;

	return TRUE;
}

BOOL CUserInterface::OnCommand(WORD wNotifyCode, WORD wID, CWnd wndCtl)
{
	DbgPrint("OnCommand(%ld, %ld, %ld)\n", wNotifyCode, wID, wndCtl.m_hWnd);

	// album independent commands
	switch (wID)
	{
	case IDC_PLAY_ALL_ALBUMS:
		return IPC_PlayAllAlbums();

	case IDC_ENQUEUE_ALL_ALBUMS:
		return IPC_EnqueueAllAlbums();

	case IDC_EXPORT_ALBUM_LIST:
		m_AlbumList.GenerateHTMLList();
		return TRUE;

	case IDC_RELOAD_ALBUM:
		ReloadAlbumList();
		m_bSaveCache = TRUE;
		return TRUE;

	case IDC_QUICK_SCAN_ALBUM:
		m_AlbumList.QuickScan();
		m_bSaveCache = TRUE;
		return TRUE;

	case IDC_PLAY_RANDOM_ALBUM:
		return IPC_PlayRandomAlbum();

	case IDC_ENQUEUE_RANDOM_ALBUM:
		return IPC_EnqueueRandomAlbum();

	case IDC_SHOW_PROPERTIES:
		ShowPreference(GetSafeHwnd(), TRUE/*show optional*/);
		return TRUE;

	case IDC_SORT_PATH:
		m_AlbumList.Sort(orderPath);
		return TRUE;

	case IDC_SORT_NAME:
		m_AlbumList.Sort(orderName);
		return TRUE;

	case IDC_SORT_ARTIST:
		m_AlbumList.Sort(orderArtist);
		return TRUE;

	case IDC_SORT_ALBUM:
		m_AlbumList.Sort(orderAlbum);
		return TRUE;

	case IDC_SORT_GENRE:
		m_AlbumList.Sort(orderGenre);
		return TRUE;

	case IDC_SORT_YEAR:
		m_AlbumList.Sort(orderYear);
		return TRUE;

	case IDC_SORT_NUMOFSONGS:
		m_AlbumList.Sort(orderNumOfSongs);
		return TRUE;

	case IDC_SORT_TOTALTIME:
		m_AlbumList.Sort(orderTotalTime);
		return TRUE;

	case IDC_SORT_ARTIST_YEAR:
		m_AlbumList.Sort(orderArtist, orderYear);
		return TRUE;

	case IDC_SORT_SHUFFLE:
		m_AlbumList.Shuffle();
		return TRUE;

	case IDC_TIMEFORMAT1:
		SetTimeFormat(0);
		return TRUE;

	case IDC_TIMEFORMAT2:
		SetTimeFormat(1);
		return TRUE;

	case IDC_HEADER_TITLE:
		ToggleColumn(m_wndList, columnName);
		return TRUE;

	case IDC_HEADER_TIME:
		ToggleColumn(m_wndList, columnTime);
		return TRUE;

	case IDC_HEADER_YEAR:
		ToggleColumn(m_wndList, columnYear);
		return TRUE;

	case IDC_HEADER_NUMOFSONGS:
		ToggleColumn(m_wndList, columnTrack);
		return TRUE;

	case IDC_HEADER_MULTIDISC:
		ToggleColumn(m_wndList, columnMultiDisc);
		return TRUE;

	case IDC_HEADER_PATH:
		ToggleColumn(m_wndList, columnPath);
		return TRUE;

	case IDC_HEADER_ARTIST:
		ToggleColumn(m_wndList, columnArtist);
		return TRUE;
	
	case IDC_HEADER_ALBUM:
		ToggleColumn(m_wndList, columnAlbum);
		return TRUE;

	case IDC_HEADER_GENRE:
		ToggleColumn(m_wndList, columnGenre);
		return TRUE;

	case IDC_AUTOSIZECOLUMN:
		ToggleAutoSizeColumns();
		return TRUE;

	case IDC_AUTOHIDEHEADER:
		m_bAutoHideHeader = !m_bAutoHideHeader;
		if (!m_bAutoHideHeader && m_bShowHeader)
		{
			m_wndList.ModifyStyle(0, LVS_NOCOLUMNHEADER);
			m_wndList.ModifyStyle(LVS_NOCOLUMNHEADER, 0);
		}
		else
			m_wndList.ModifyStyle(0, LVS_NOCOLUMNHEADER);
		AutoSizeColumns();
		return TRUE;

	case IDC_SHOWINDEXNUM:
		m_bShowIndexNum = !m_bShowIndexNum;
		Redraw();
		return TRUE;

	case IDC_SHOWFULLPATH:
		m_nShowFullPath = !m_nShowFullPath;
		Redraw();
		return TRUE;

	case IDC_JUMP_TO_ALBUM:
		JumpToAlbum();
		return TRUE;

	case IDC_TOGGLE_VIEW:
		ToggleView();
		return TRUE;

	case IDC_COVER_VIEW:
		CoverView();
		return TRUE;

	case IDC_LIST_VIEW:
		ListView();
		return TRUE;

	case IDC_SHOW_STATUS:
		ToggleShowStatus();
		return TRUE;

	case IDC_SHOW_HEADER:
		ToggleShowHeader();
		return TRUE;

	case IDC_SHOW_LABEL:
		ToggleShowLabel();
		return TRUE;

	case IDC_NEXT_ALBUM:
	case IDC_NEXTALBUMBTN:
		return IPC_PlayNextAlbum();

	case IDC_PREV_ALBUM:
	case IDC_PREVALBUMBTN:
		return IPC_PlayPreviousAlbum();

	case IDC_KEYPAD0:
	case IDC_KEYPAD1:	
	case IDC_KEYPAD2:
	case IDC_KEYPAD3:
	case IDC_KEYPAD4:
	case IDC_KEYPAD5:
	case IDC_KEYPAD6:
	case IDC_KEYPAD7:
	case IDC_KEYPAD8:
	case IDC_KEYPAD9:
		OnKey(wID - IDC_KEYPAD0);
		return TRUE;
	}

	// go through all the selected items
	int nPos = -1;
	int i = 0;
	CDWordArray removelist;
	BOOL bRedraw = FALSE;

	if (!DoLongOperation(ListView_GetSelectedCount(m_wndList))) return FALSE;

	while ((nPos = m_wndList.SendMessage(LVM_GETNEXTITEM, nPos, MAKELPARAM(LVIS_SELECTED, 0))) != -1)
	{
		CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(nPos);
		if (pAL)
		{
			BOOL bRet = (BOOL)-1;
			switch (wID)
			{
			case IDC_PLAYBTN:
			case IDC_PLAY_ALBUM:
				if (i == 0)
					pAL->Play();
				else
					pAL->Enqueue();
				break;

			case IDC_PLAY_MENU:
				if (i == 0)
				{
					if (wndWinampAL.GetSetting(settingEnqueueDefault))
						pAL->Enqueue();
					else
						pAL->Play();
				}
				else
					pAL->Enqueue();
				break;

			case IDC_ENQUEUEBTN:
			case IDC_ENQUEUE_ALBUM:
				pAL->Enqueue();
				break;

			case IDC_ENQUEUE_N_PLAY:
				if (i == 0)
				{
					pAL->EnqueueAndPlay();
				}
				else
				{
					pAL->Enqueue();
				}
				break;

//			case IDC_ENQUEUE_AFTER_CUR:
//				return pAL->EnqueueAfterCurrent();
				
			case IDC_SHOW_ALBUM_COVER:
				// only works for the first one (if multiples are selected)
				bRet = pAL->ShowCover();
				break;

			case IDC_WRITE_PLAYLIST:
				pAL->WritePlaylist();
				break;

			case IDC_REFRESH_ALBUM:
				if (!pAL->Refresh())
				{
					pAL->AddRef();
					removelist.Add(nPos);
				}
				else
				{
					bRedraw = TRUE;
				}
				break;

			case IDC_SHOW_ALBUM_INFO:
				// only works for the first one (if multiples are selected)
				if (pAL->ShowInfo(GetSafeHwnd()))
				{
					ListView_RedrawItems(m_wndList, nPos, nPos);
					m_bSaveCache = TRUE;
				}
				bRet = TRUE;
				break;

			case IDC_EXPLORE:
				bRet = pAL->Explore();
				break;

			case IDC_PLAY_ALBUM_BY_ARTIST:
				if (!DoLongOperation(m_AlbumList.GetArtistAlbumCount(pAL->GetArtist())))
					bRet = FALSE;
				else
					bRet = m_AlbumList.PlayAlbumByArtist(pAL->GetArtist());
				break;

			case IDC_ENQUEUE_ALBUM_BY_ARTIST:
				if (!DoLongOperation(m_AlbumList.GetArtistAlbumCount(pAL->GetArtist())))
					bRet = FALSE;
				else
					bRet = m_AlbumList.EnqueueAlbumByArtist(pAL->GetArtist());
				break;

			case IDC_PLAY_RANDOM_ALBUM_BY_ARTIST:
				bRet = m_AlbumList.PlayRandomAlbumByArtist(pAL->GetArtist());
				break;

			case IDC_ENQUEUE_RANDOM_ALBUM_BY_ARTIST:
				bRet = m_AlbumList.EnqueueRandomAlbumByArtist(pAL->GetArtist());
				break;

			default:
				// custom IPC menu
				if ((IPC_ADD_MENU <= wID) && (wID <= IPC_ADD_MENU_LAST))
				{
					CPtrArray *p = NULL;
					if (wndWinampAL.GetIPCMenu(&p))
					{
						CIPCMenu *m = (CIPCMenu *)p->GetAt(wID - IPC_ADD_MENU);
						if (m)
						{
							AutoChar m3u(pAL->GetM3U());
							AutoChar path(pAL->GetPath());
							AutoChar title(pAL->GetTitle());
							AutoChar artist(pAL->GetArtist());
							AutoChar album(pAL->GetAlbum());

							albuminfoStruct a;
							a.title	= title;
							a.artist= artist;
							a.album	= album;
							a.path	= pAL->IsPlaylistBased() ? m3u : path;
							a.year	= pAL->GetYear();
							a.tracks= pAL->GetTrackCount();

							CWnd(m->hWnd).SendMessage(m->uMsg, (WPARAM)&a, 0);
						}
					}
					bRet = TRUE;
					break;
				}

				// SendTo menu?
				if (sendto.mode == 2)
				{
					sendto.menu_id = wID;
					if (wndWinamp.SendIPCMessage((WPARAM)&sendto, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
					{
						pAL->LibrarySendTo(sendto);
					}
				}
			}
			pAL->Release();
			if (bRet != (BOOL)-1) return bRet;
		}
		++i;
	}

    if (sendto.mode) 
    {
		sendto.mode = 4;
		wndWinamp.SendIPCMessage((WPARAM)&sendto, IPC_LIBRARY_SENDTOMENU); // cleanup
		DbgPrint("Cleanup SendTo\n");
		sendto.mode = 0;
    }

	// remove any failed album from refresh
	if (removelist.GetSize())
	{
		for (int a=removelist.GetSize()-1; a>=0; --a)
		{
			CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAt(removelist[a]);
			if (pAlbum) pAlbum->Release();
			m_AlbumList.RemoveAt(removelist[a]);
		}

		// update size
		ListView_SetItemCountEx(m_wndList, m_AlbumList.GetSize(), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);
	}

	if (bRedraw)
		Redraw();

	return FALSE;
}

LRESULT CUserInterface::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_HEADER_COLUMN_RCLICK)
		return OnHeaderRClickAlbumList(wParam, lParam);

	else if (message == WM_SETFOCUS)
		wndWinampAL.SetFocus(this);

	else if (message == WM_TIMER)
	{
		if (wParam == TIMERID_KEYINPUT)
			OnKey(-1);
	}

	else if (message == WM_AL_IPC)
	{
		if (wParam == IPC_SHOWHIDE)
		{
			IPC_ShowHide();
		}
	}

	else if (message == WM_AL_POSTINIT)
	{
		return OnPostInit();
	}

	else if (message == (WM_USER+2020))
	{
//		CAlbum *pAL = (CAlbum *)wParam;
//		SIZE size = { m_nCoverWidth, m_nCoverHeight };
//		pAL->SetIconIndex(AddImage(pAL->GetCover(size, m_bCacheCover)));
//		InvalidateRect(m_wndList, NULL, FALSE);
	}

	else if (message == WM_ML_CHILDIPC)
	{
		if (lParam == ML_CHILDIPC_DROPITEM && wParam)
			return 1;
	}

	LRESULT lRet = WADlg_handleDialogMsgs(m_hWnd, message, wParam, lParam);
	if (lRet) return lRet;

	if ((message == WM_KEYDOWN) ||
		(message == WM_SYSKEYDOWN))
	{
		return ProcessAccelerator(m_hWnd, message, wParam, lParam);
	}

	return CDialog::DefWindowProc(message, wParam, lParam);
}

BOOL CUserInterface::OkToQuit()
{
	return m_AlbumList.OkToQuit();
}

void CUserInterface::ResetFont()
{
	CreateListFont();
}

BOOL CUserInterface::OnNotify(UINT nID, NMHDR *pnmh, LRESULT *pResult)
{
	switch (pnmh->idFrom)
	{
	case IDC_ALBUMLIST:
		switch(pnmh->code)
		{
		case NM_CUSTOMDRAW:
			return OnCustomDrawAlbumList(pnmh, pResult);

		case NM_COOLSB_CUSTOMDRAW:
			return OnCoolSBCustomDrawAlbumList(pnmh, pResult);

		case NM_DBLCLK:
			return OnDblClkAlbumList(pnmh, pResult);

		case NM_CLICK:
			return OnLClickAlbumList(pnmh, pResult);

		case NM_RCLICK:
			return OnRClickAlbumList(pnmh, pResult);

		case LVN_GETDISPINFO:
			return OnGetDispInfoAlbumList(pnmh, pResult);

		case LVN_GETDISPINFOW:
			return OnGetDispInfoUnicodeAlbumList(pnmh, pResult);

		case LVN_GETINFOTIP:
			return OnGetInfoTipAlbumList(pnmh, pResult);

		case LVN_GETINFOTIPW:
			return OnGetInfoTipUnicodeAlbumList(pnmh, pResult);

		case LVN_COLUMNCLICK:
			return OnColumnClickAlbumList(pnmh, pResult);

		case LVN_BEGINDRAG:
			return OnBeginDragAlbumList(pnmh, pResult);
		}
		break;
	}

	return FALSE;
}

void EnableHighlighting(HWND hWnd, int row, bool bHighlight)
{
	ListView_SetItemState(hWnd, row, bHighlight? LVIS_SELECTED: 0, LVIS_SELECTED);
}

bool IsRowSelected(HWND hWnd, int row)
{
	return ListView_GetItemState(hWnd, row, LVIS_SELECTED) != 0;
}

bool IsRowHighlighted(HWND hWnd, int row)
{
	return IsRowSelected(hWnd, row) /*&& (::GetFocus() == hWnd)*/;
}

BOOL CUserInterface::OnCustomDrawAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW *)pnmh;

	static bool bHighlighted = false;

	// Take the default processing unless we set this to something else below.
	*pResult = CDRF_DODEFAULT;

	CWnd wndList = pLVCD->nmcd.hdr.hwndFrom;

	BOOL bIconView = ((GetWindowLong(wndList, GWL_STYLE) & LVS_TYPEMASK) == LVS_ICON);
	if (bIconView)
	{
		// First thing - check the draw stage. If it's the control's prepaint
		// stage, then tell Windows we want messages for every item.
/*		if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
		{
//			*pResult = CDRF_NOTIFYITEMDRAW;
//			DbgPrint("PrePaint\n");
		}
		else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
		{
	        int iRow = (int)pLVCD->nmcd.dwItemSpec;
			bHighlighted = IsRowHighlighted(wndList, iRow);
			if (bHighlighted)
			{
				pLVCD->clrText   = WADlg_getColor(WADLG_SELBAR_FGCOLOR);
				pLVCD->clrTextBk = WADlg_getColor(WADLG_SELBAR_BGCOLOR);
				
				EnableHighlighting(wndList, iRow, false);
			}
			*pResult = CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
			DbgPrint("ItemPrePaint (%ld, %ld)\n", pLVCD->nmcd.dwItemSpec, bHighlighted);
		}
		else if ( CDDS_ITEMPOSTPAINT == pLVCD->nmcd.dwDrawStage )
		{
			if (bHighlighted)
			{
				int  iRow = (int)pLVCD->nmcd.dwItemSpec;
				
				EnableHighlighting(wndList, iRow, true);
			}
			
			*pResult = CDRF_DODEFAULT;
			DbgPrint("ItemPostPaint (%ld, %ld)\n", pLVCD->nmcd.dwItemSpec, bHighlighted);
		}*/
	}
	else
	{
		// First thing - check the draw stage. If it's the control's prepaint
		// stage, then tell Windows we want messages for every item.
		if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
//			DbgPrint("PrePaint\n");
		}
		else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
		{
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
		
			if (pLVCD->nmcd.dwItemSpec == (DWORD)m_AlbumList.GetCurAlbumIndex())
			{
				*pResult |= CDRF_NOTIFYPOSTPAINT;
			}
//			DbgPrint("ItemPrePaint\n");
		}
		else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
		{
			if ((int)pLVCD->nmcd.dwItemSpec < ListView_GetTopIndex(wndList))
				return TRUE;

			// get the subitem's rectangle
			RECT rc;
			ListView_GetSubItemRect(wndList, pLVCD->nmcd.dwItemSpec, pLVCD->iSubItem, LVIR_LABEL, &rc);

			// fixes the "icon spacing" on the left of the first column
			RECT rcHeader;
			Header_GetItemRect(wndList.GetDlgItem(0), pLVCD->iSubItem, &rcHeader);
			rc.left = rc.right - (rcHeader.right - rcHeader.left);

			// get the subitem's string/state
			LVITEM lvi;
			lvi.mask = LVIF_STATE;
			lvi.iItem = pLVCD->nmcd.dwItemSpec;		// row
			lvi.iSubItem = pLVCD->iSubItem;			// column
			lvi.stateMask = LVIS_SELECTED;
			ListView_GetItem(wndList, &lvi);

			// background/hilite color
			int fgcolor = ((lvi.state  & LVIS_SELECTED) == LVIS_SELECTED) ? WADLG_SELBAR_FGCOLOR : WADLG_ITEMFG;
			int bgcolor = ((lvi.state  & LVIS_SELECTED) == LVIS_SELECTED) ? WADLG_SELBAR_BGCOLOR : WADLG_ITEMBG;

			RECT rc1 = { 0, 0, rc.right - rc.left, rc.bottom - rc.top };

			// create offscreen surface (reduces flickering)
			HDC			hDC		= CreateCompatibleDC(pLVCD->nmcd.hdc);;
			HBITMAP		hBitmap	= CreateCompatibleBitmap(pLVCD->nmcd.hdc, rc1.right, rc1.bottom);
			HBITMAP		oBitmap	= (HBITMAP)SelectObject(hDC, hBitmap);

			// fill with background color
			HBRUSH hbr = CreateSolidBrush(WADlg_getColor(bgcolor));
			FillRect(hDC, &rc1, hbr);
			DeleteObject(hbr);

			// get text aligment from header control
			HDITEM hdi;
			hdi.mask = HDI_FORMAT;
			Header_GetItem(wndList.GetDlgItem(0), pLVCD->iSubItem, &hdi);
			int nAlign = ((hdi.fmt & HDF_JUSTIFYMASK) == LVCFMT_RIGHT) ? DT_RIGHT : DT_LEFT;

			// add a bit of margin
			RECT rcItem = rc1;
			rcItem.left += 0;
			rcItem.right -= 5;
			rcItem.top ++;

			// set text attributes
			HFONT oFont = (HFONT)SelectObject(hDC, m_hFont);
			SetTextColor(hDC, WADlg_getColor(fgcolor));
			SetBkMode(hDC, TRANSPARENT);

			// simulate bold by draw it twice
/*			if (pLVCD->nmcd.dwItemSpec == (DWORD)m_AlbumList.GetCurAlbumIndex())
			{
				RECT rc = rcItem;
				rc.left += (nAlign == DT_LEFT) ? 1 : -1;
				rc.right += (nAlign == DT_LEFT) ? 1 : -1;
				// draw the text (shifted by 1 pixel)
				DrawTextUnicode(hDC, lvi.pszText, -1, &rc, nAlign | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
			}
*/
			// draw the text
			if (ListView_GetUnicodeFormat(wndList))
			{
				WCHAR name[MAX_PATH+1] = L" ";
				LVITEMW lvi;
				lvi.mask = LVIF_TEXT;
				lvi.iItem = pLVCD->nmcd.dwItemSpec;		// row
				lvi.iSubItem = pLVCD->iSubItem;			// column
				lvi.pszText = name+1;
				lvi.cchTextMax = MAX_PATH;
				wndList.SendMessage(LVM_GETITEMW, 0, (LPARAM)&lvi);
				DrawTextW(hDC, name, -1, &rcItem, nAlign | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
			}
			else
			{
				char name[MAX_PATH+1] = " ";
				LVITEM lvi;
				lvi.mask = LVIF_TEXT;
				lvi.iItem = pLVCD->nmcd.dwItemSpec;		// row
				lvi.iSubItem = pLVCD->iSubItem;			// column
				lvi.pszText = name+1;
				lvi.cchTextMax = MAX_PATH;
				wndList.SendMessage(LVM_GETITEM, 0, (LPARAM)&lvi);
				DrawTextUnicode(hDC, name, -1, &rcItem, nAlign | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
			}

			// copy the offscreen drawing to onscreen
			BitBlt(pLVCD->nmcd.hdc, rc.left, rc.top, rc1.right, rc1.bottom, hDC, 0, 0, SRCCOPY);

			// cleanup
			SelectObject(hDC, oFont);
			SelectObject(hDC, oBitmap);
			DeleteObject(hBitmap);
			DeleteDC(hDC);

			*pResult = CDRF_SKIPDEFAULT;

//			DbgPrint("SubItemPrePaint\n");
		}
		else if ( CDDS_ITEMPOSTPAINT == pLVCD->nmcd.dwDrawStage )
		{
			HPEN hPen = CreatePen(PS_SOLID, 0, WADlg_getColor(WADLG_ITEMFG));
			HPEN hOldPen = (HPEN)SelectObject(pLVCD->nmcd.hdc, hPen);

			RECT rc;
			ListView_GetItemRect(wndList, pLVCD->nmcd.dwItemSpec, &rc, LVIR_BOUNDS);

			// draw the selection rectangle
			MoveToEx(pLVCD->nmcd.hdc, rc.left, rc.top, NULL);
			LineTo(pLVCD->nmcd.hdc, rc.left, rc.bottom-1);
			LineTo(pLVCD->nmcd.hdc, rc.right-1, rc.bottom-1);
			LineTo(pLVCD->nmcd.hdc, rc.right-1, rc.top);
			LineTo(pLVCD->nmcd.hdc, rc.left, rc.top);

			SelectObject(pLVCD->nmcd.hdc, hOldPen);
			DeleteObject(hPen);
//			DbgPrint("ItemPostPaint\n");
		}
	}

	return TRUE;
}

typedef struct 
{
	int x, y;
	int width, height;
} CustomDrawTable;

CustomDrawTable cdt_horz_normal[] = 
{
	{ 0,  45, 14, 14 },	//left arrow  NORMAL
	{ 14, 45, 14, 14 }, //right arrow NORMAL
	{ WADLG_SCROLLBAR_FGCOLOR, WADLG_SCROLLBAR_BGCOLOR }, //page left   NORMAL
	{ WADLG_SCROLLBAR_FGCOLOR, WADLG_SCROLLBAR_BGCOLOR }, //page right  NORMAL
	
	{ -1, -1, -1, -1 },	//padding

	{ 84,  31, 4,  4  }, //horz thumb (top,left)
	{ 84,  35, 4,  6  }, //horz thumb (middle,left)
	{ 84,  41, 4,  4  }, //horz thumb (bottom,left)

	{ 88,  31, 20, 4  }, //horz thumb (top,middle)
	{ 88,  35, 5,  6  }, //horz thumb (middle,middle,left)
	{ 103, 35, 5,  6  }, //horz thumb (middle,middle,right)
	{ 88,  41, 20, 4  }, //horz thumb (bottom,middle)

	{ 108, 31, 4,  4  }, //horz thumb (top,right)
	{ 108, 35, 4,  6  }, //horz thumb (middle,right)
	{ 108, 41, 4,  4  }, //horz thumb (bottom,right)

	{ 94, 35, 8,  6  }, //horz thumb (middle,middle,middle)
};

CustomDrawTable cdt_horz_active[] = 
{
	{ 28, 45, 14, 14 }, //left arrow  ACTIVE
	{ 42, 45, 14, 14 }, //right arrow ACTIVE
	{ WADLG_SCROLLBAR_INV_FGCOLOR, WADLG_SCROLLBAR_INV_BGCOLOR }, //page left   ACTIVE
	{ WADLG_SCROLLBAR_INV_FGCOLOR, WADLG_SCROLLBAR_INV_BGCOLOR }, //page right  ACTIVE

	{ -1, -1, -1, -1 },	//padding

	{ 84,  45, 4,  4  }, //horz thumb (top,left)
	{ 84,  49, 4,  6  }, //horz thumb (middle,left)
	{ 84,  55, 4,  4  }, //horz thumb (bottom,left)

	{ 88,  45, 20, 4  }, //horz thumb (top,middle)
	{ 88,  49, 5,  6  }, //horz thumb (middle,middle,left)
	{ 103, 49, 5,  6  }, //horz thumb (middle,middle,right)
	{ 88,  56, 20, 3  }, //horz thumb (bottom,middle)

	{ 108, 45, 4,  4  }, //horz thumb (top,right)
	{ 108, 49, 4,  6  }, //horz thumb (middle,right)
	{ 108, 55, 4,  4  }, //horz thumb (bottom,right)

	{ 94, 49, 9,  7  }, //horz thumb (middle,middle,middle)
};

CustomDrawTable cdt_vert_normal[] = 
{
	{ 0,  31, 14, 14 }, //up arrow   NORMAL
	{ 14, 31, 14, 14 }, //down arrow NORMAL
	{ WADLG_SCROLLBAR_FGCOLOR, WADLG_SCROLLBAR_BGCOLOR }, //page up	 NORMAL
	{ WADLG_SCROLLBAR_FGCOLOR, WADLG_SCROLLBAR_BGCOLOR }, //page down  NORMAL

	{ -1, -1, -1, -1 },	//padding

	{ 56, 31, 4,  4  }, //vert thumb (top,left)
	{ 60, 31, 6,  4  }, //vert thumb (top,middle)
	{ 66, 31, 4,  4  }, //vert thumb (top,right)
	{ 56, 35, 4, 20  }, //vert thumb (middle,left)
	{ 60, 35, 6,  5  }, //vert thumb (middle,middle,top)
	{ 60, 50, 6,  5  }, //vert thumb (middle,middle,bottom)
	{ 66, 35, 4, 20  }, //vert thumb (middle,right)
	{ 56, 55, 4,  4  }, //vert thumb (bottom,left)
	{ 60, 55, 6,  4  }, //vert thumb (bottom,middle)
	{ 66, 55, 4,  4  }, //vert thumb (bottom,right)
	{ 60, 41, 6,  8  }, //vert thumb (middle,middle,middle)
};

CustomDrawTable cdt_vert_active[] = 
{
	{ 28, 31,  14, 14 }, //up arrow   ACTIVE
	{ 42, 31, 14, 14 }, //down arrow ACTIVE
	{ WADLG_SCROLLBAR_INV_FGCOLOR, WADLG_SCROLLBAR_INV_BGCOLOR }, //page up	 ACTIVE
	{ WADLG_SCROLLBAR_INV_FGCOLOR, WADLG_SCROLLBAR_INV_BGCOLOR }, //page down  ACTIVE

	{ -1, -1, -1, -1 },	//padding

	{ 70, 31, 4,  4  }, //vert thumb (left)
	{ 74, 31, 6,  4  }, //vert thumb (middle)
	{ 80, 31, 4,  4  }, //vert thumb (right)
	{ 70, 35, 4, 20  }, //vert thumb (middle,left)
	{ 74, 35, 6,  5  }, //vert thumb (middle,middle,top)
	{ 74, 50, 6,  5  }, //vert thumb (middle,middle,bottom)
	{ 81, 35, 3, 20  }, //vert thumb (middle,right)
	{ 70, 55, 4,  4  }, //vert thumb (bottom,left)
	{ 74, 55, 6,  4  }, //vert thumb (bottom,middle)
	{ 80, 55, 4,  4  }, //vert thumb (bottom,right)
	{ 74, 41, 7,  9  }, //vert thumb (middle,middle,middle)
};

BOOL CUserInterface::OnCoolSBCustomDrawAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	NMCSBCUSTOMDRAW *pCSCD = (NMCSBCUSTOMDRAW *)pnmh;

	// Take the default processing unless we set this to something else below.
	*pResult = 0;

	if (pCSCD->dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_SKIPDEFAULT;
		return TRUE;
	}
	
	if(pCSCD->dwDrawStage == CDDS_POSTPAINT)
	{
		
	}

	RECT *rc;
	CustomDrawTable *cdt;

	//the sizing gripper in the bottom-right corner
	if (pCSCD->nBar == SB_BOTH)	
	{
		HBRUSH hbr = CreateSolidBrush(WADlg_getColor(WADLG_SCROLLBAR_DEADAREA_COLOR));
		FillRect(pCSCD->hdc, &pCSCD->rect, hbr);
		DeleteObject(hbr);
			
		*pResult = CDRF_SKIPDEFAULT;
		return TRUE;
	}
	else if (pCSCD->nBar == SB_HORZ)
	{
		rc = &pCSCD->rect;

		if (pCSCD->uState == CDIS_SELECTED) 
			cdt = &cdt_horz_active[pCSCD->uItem];
		else				   
			cdt = &cdt_horz_normal[pCSCD->uItem];
		
		if ((pCSCD->uItem == HTSCROLL_PAGELEFT) ||
			(pCSCD->uItem == HTSCROLL_PAGERIGHT))

		{
			COLORREF fg = WADlg_getColor(cdt->x);
			COLORREF bg = WADlg_getColor(cdt->y);

			COLORREF color = RGB((GetRValue(fg)+GetRValue(bg))/2,
								(GetGValue(fg)+GetGValue(bg))/2,
								(GetBValue(fg)+GetBValue(bg))/2);

			HBRUSH hbr = CreateSolidBrush(color);
			FillRect(pCSCD->hdc, rc, hbr);
			DeleteObject(hbr);

			*pResult = CDRF_SKIPDEFAULT;
			return TRUE;
		}
		if (pCSCD->uItem == HTSCROLL_THUMB)
		{
			// get skin bitmap
			HDC hdcSkin = CreateCompatibleDC(pCSCD->hdc);
			HBITMAP oBitmap = (HBITMAP)SelectObject(hdcSkin, WADlg_getBitmap());
			SetStretchBltMode(pCSCD->hdc, COLORONCOLOR);

			// top left
			StretchBlt(pCSCD->hdc, rc->left, rc->top, 4, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// middle left
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left, rc->top+4, 4, rc->bottom-rc->top-8, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// bottom left
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left, rc->bottom-4, 4, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			
			// top middle
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left+4, rc->top, rc->right-rc->left-8, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// middle middle left
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left+4, rc->top+4, (rc->right-rc->left-8)/2+2, rc->bottom-rc->top-7, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// middle middle right
			cdt++;
			StretchBlt(pCSCD->hdc, rc->right-(rc->right-rc->left-8)/2-3, rc->top+4, (rc->right-rc->left-8)/2, rc->bottom-rc->top-7, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// bottom middle
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left+4, rc->bottom-cdt->height, rc->right-rc->left-8, cdt->height, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);

			// top right
			cdt++;
			StretchBlt(pCSCD->hdc, rc->right-4, rc->top, 4, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// middle right
			cdt++;
			StretchBlt(pCSCD->hdc, rc->right-4, rc->top+4, 4, rc->bottom-rc->top-8, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// bottom right
			cdt++;
			StretchBlt(pCSCD->hdc, rc->right-4, rc->bottom-4, 4, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);

			// middle middle middle
			cdt++;
			if ((rc->right-rc->left-8) > cdt->width)
			{
				StretchBlt(pCSCD->hdc, rc->left+(rc->right-rc->left)/2-4, rc->top+4, cdt->width, cdt->height, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			}

			// free dc
			SelectObject(hdcSkin, oBitmap);
			DeleteDC(hdcSkin);

			*pResult = CDRF_SKIPDEFAULT;
			return TRUE;
		}
	}
	else if (pCSCD->nBar == SB_VERT)
	{
		rc = &pCSCD->rect;

		if (pCSCD->uState == CDIS_SELECTED)  
			cdt = &cdt_vert_active[pCSCD->uItem];
		else				    
			cdt = &cdt_vert_normal[pCSCD->uItem];

		if ((pCSCD->uItem == HTSCROLL_PAGEGUP) ||
			(pCSCD->uItem == HTSCROLL_PAGEGDOWN))

		{
			COLORREF fg = WADlg_getColor(cdt->x);
			COLORREF bg = WADlg_getColor(cdt->y);

			COLORREF color = RGB((GetRValue(fg)+GetRValue(bg))/2,
								(GetGValue(fg)+GetGValue(bg))/2,
								(GetBValue(fg)+GetBValue(bg))/2);

			HBRUSH hbr = CreateSolidBrush(color);
			FillRect(pCSCD->hdc, rc, hbr);
			DeleteObject(hbr);

			*pResult = CDRF_SKIPDEFAULT;
			return TRUE;
		}
		if (pCSCD->uItem == HTSCROLL_THUMB)
		{
			// get skin bitmap
			HDC hdcSkin = CreateCompatibleDC(pCSCD->hdc);
			HBITMAP oBitmap = (HBITMAP)SelectObject(hdcSkin, WADlg_getBitmap());
			SetStretchBltMode(pCSCD->hdc, COLORONCOLOR);

			// top left
			StretchBlt(pCSCD->hdc, rc->left, rc->top, 4, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// top middle
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left+4, rc->top, rc->right-rc->left-8, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// top right
			cdt++;
			StretchBlt(pCSCD->hdc, rc->right-4, rc->top, 4, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);

			// middle left
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left, rc->top+4, 4, rc->bottom-rc->top-8, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// middle middle top
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left+4, rc->top+4, rc->right-rc->left-7, (rc->bottom-rc->top-8)/2+2, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// middle middle bottom
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left+4, rc->bottom-(rc->bottom-rc->top-8)/2-4, rc->right-rc->left-7, (rc->bottom-rc->top-8)/2, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// middle right
			cdt++;
			StretchBlt(pCSCD->hdc, rc->right-cdt->width, rc->top+4, cdt->width, rc->bottom-rc->top-8, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);

			// bottom left
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left, rc->bottom-4, 4, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// bottom middle
			cdt++;
			StretchBlt(pCSCD->hdc, rc->left+4, rc->bottom-4, rc->right-rc->left-8, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// bottom right
			cdt++;
			StretchBlt(pCSCD->hdc, rc->right-4, rc->bottom-4, 4, 4, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			// middle middle middle
			cdt++;
			if ((rc->bottom-rc->top-8) > cdt->height)
			{
				StretchBlt(pCSCD->hdc, rc->left+4, rc->top+(rc->bottom-rc->top)/2-4, cdt->width, cdt->height, hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);
			}

			// free dc
			SelectObject(hdcSkin, oBitmap);
			DeleteDC(hdcSkin);

			*pResult = CDRF_SKIPDEFAULT;
			return TRUE;
		}
	}
	else
	{
		*pResult = CDRF_DODEFAULT;
		return TRUE;
	}

	// get skin bitmap
	HDC hdcSkin = CreateCompatibleDC(pCSCD->hdc);
	HBITMAP oBitmap = (HBITMAP)SelectObject(hdcSkin, WADlg_getBitmap());
	SetStretchBltMode(pCSCD->hdc, COLORONCOLOR);

	//normal bitmaps, use same code for HORZ and VERT
	StretchBlt(pCSCD->hdc, rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top,
		hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);

	// free dc
	SelectObject(hdcSkin, oBitmap);
	DeleteDC(hdcSkin);

	*pResult = CDRF_SKIPDEFAULT;
	return TRUE;
}

BOOL CUserInterface::OnGetDispInfoUnicodeAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	NMLVDISPINFOW* pLVDI = (NMLVDISPINFOW *)pnmh;

	int encoding = wndWinampAL.GetSetting(settingEncodingLangAL);
	int cp = CP_ACP;
	switch (encoding)
	{
	case langTraditionalChinese:	cp = 950;	break;
	case langSimplifiedChinese:		cp = 936;	break;
	case langJapanese:				cp = 932;	break;
	case langKorean:				cp = 949;	break;
	}

	BOOL bReportView = (BOOL)((GetWindowLong(m_wndList, GWL_STYLE) & LVS_TYPEMASK) == LVS_REPORT);

	if ((bReportView || m_bShowLabel) && (pLVDI->item.mask & LVIF_TEXT))
	{
		CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(pLVDI->item.iItem);
		if (pAL == NULL) return FALSE;

		HDITEM hdi;
		if (bReportView)
		{
			memset(&hdi, 0, sizeof(HDITEM));
			hdi.mask = HDI_LPARAM;
			m_wndList.GetDlgItem(0).SendMessage(HDM_GETITEM, pLVDI->item.iSubItem, (LPARAM)&hdi);
		}
		else	// always display the "title" for cover view
		{
			hdi.lParam = columnName;
		}

		char str[MAX_PATH] = "";
		switch (hdi.lParam)
		{
		case columnName:		// name
			if (bReportView)
			{
				if (m_bShowIndexNum)
				{
					wchar_t *spacer = (m_AlbumList.GetSize() > 9) && (pLVDI->item.iItem < 9) ? L" " : L"";
					swprintf(pLVDI->item.pszText, L"%s%ld. %s", spacer, pLVDI->item.iItem+1, pAL->GetTitle());
				}
				else
				{
					wcsncpy(pLVDI->item.pszText, pAL->GetTitle(), pLVDI->item.cchTextMax);
				}
			}
			else
			{
				wcsncpy(pLVDI->item.pszText, pAL->GetTitle(), pLVDI->item.cchTextMax);
			}
			break;

		case columnTime:		// time
			if (m_nTimeDisplay == 0)
			{
				int nTime = pAL->GetTotalTime();

				if (nTime > 3600)
				{
					int hour = nTime / 3600;
					int min = nTime % 3600;
					wsprintf(str, "%ld:%02ld:%02ld", hour, min / 60, min % 60);
				}
				else
					wsprintf(str, "%ld:%02ld", nTime / 60, nTime % 60);
			}
			else if (m_nTimeDisplay == 1)
				wsprintf(str, "%ld:%02ld", pAL->GetTotalTime() / 60, pAL->GetTotalTime() % 60);
			MultiByteToWideChar(cp, MB_PRECOMPOSED, str, -1, pLVDI->item.pszText, pLVDI->item.cchTextMax);
			break;

		case columnYear:		// year
			wsprintf(str, "%ld", pAL->GetYear());
			MultiByteToWideChar(cp, MB_PRECOMPOSED, str, -1, pLVDI->item.pszText, pLVDI->item.cchTextMax);
			break;

		case columnTrack:		// tracks
			wsprintf(str, "%ld", pAL->GetTrackCount());
			MultiByteToWideChar(cp, MB_PRECOMPOSED, str, -1, pLVDI->item.pszText, pLVDI->item.cchTextMax);
			break;

		case columnMultiDisc:
			wsprintf(str, "%ld", ((pAL->dwFlags & AI_MULTIDISC) != 0));
			MultiByteToWideChar(cp, MB_PRECOMPOSED, str, -1, pLVDI->item.pszText, pLVDI->item.cchTextMax);
			break;

		case columnPath:
			if (m_nShowFullPath)
			{
				if (pAL->IsPlaylistBased())
					wcsncpy(pLVDI->item.pszText, pAL->GetM3U(), pLVDI->item.cchTextMax);
				else
					wcsncpy(pLVDI->item.pszText, pAL->GetPath(), pLVDI->item.cchTextMax);
			}
			else
			{
				const wchar_t *ptr = NULL;

				if (pAL->IsPlaylistBased())
					ptr = pAL->GetM3U();
				else
					ptr = pAL->GetPath();

				if ((int)wcslen(ptr) > pAL->nPathLen)
				{
					wcsncpy(pLVDI->item.pszText, ptr + pAL->nPathLen + 1, pLVDI->item.cchTextMax);
				}
			}
			break;

		case columnArtist:
			wcsncpy(pLVDI->item.pszText, pAL->GetArtist(), pLVDI->item.cchTextMax);
			break;

		case columnAlbum:
			wcsncpy(pLVDI->item.pszText, pAL->GetAlbum(), pLVDI->item.cchTextMax);
			break;

		case columnGenre:
			wcsncpy(pLVDI->item.pszText, pAL->GetGenre(), pLVDI->item.cchTextMax);
			break;
		}
		pAL->Release();
	}

	if (!bReportView && (pLVDI->item.mask & LVIF_IMAGE))
	{
		CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(pLVDI->item.iItem);
		if (pAL == NULL) return FALSE;

		if (pAL->GetIconIndex() >= 0)
		{
			pLVDI->item.iImage = pAL->GetIconIndex();
			DbgPrint("Profile %ld: Got image %ld for item\n", m_nProfileID, pLVDI->item.iImage, pLVDI->item.iItem);
		}
		else
		{
			DbgPrint("Profile %ld: Adding item %ld to cover queue\n", m_nProfileID, pLVDI->item.iItem);
			pLVDI->item.iImage = 1;
			EnterCriticalSection(&m_csCover);
			m_CoverArray.Add(pLVDI->item.iItem);
			LeaveCriticalSection(&m_csCover);
			SetEvent(m_hCoverEvent);
		}
		pAL->Release();
	}

	if (!bReportView && (pLVDI->item.mask & LVIF_STATE))
	{
		if (pLVDI->item.iItem == m_AlbumList.GetCurAlbumIndex())
		{
			pLVDI->item.state |= INDEXTOOVERLAYMASK(1);
		}
	}

	*pResult = 0;

	return TRUE;
}

BOOL CUserInterface::OnGetDispInfoAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	NMLVDISPINFO* pLVDI = (NMLVDISPINFO *)pnmh;

	BOOL bReportView = (BOOL)((GetWindowLong(m_wndList, GWL_STYLE) & LVS_TYPEMASK) == LVS_REPORT);

	if ((bReportView || m_bShowLabel) && (pLVDI->item.mask & LVIF_TEXT))
	{
		CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(pLVDI->item.iItem);
		if (pAL == NULL) return FALSE;

		HDITEM hdi;
		memset(&hdi, 0, sizeof(HDITEM));
		hdi.mask = HDI_LPARAM;
		m_wndList.GetDlgItem(0).SendMessage(HDM_GETITEM, pLVDI->item.iSubItem, (LPARAM)&hdi);
		char *spacer;

		char str[MAX_PATH] = "";
		switch (hdi.lParam)
		{
		case columnName:		// name
			if (bReportView)
			{
				if (m_bShowIndexNum)
				{
					spacer = (m_AlbumList.GetSize() > 9) && (pLVDI->item.iItem < 9) ? " " : "";
					wsprintf(str, "%s%ld. ", spacer, pLVDI->item.iItem+1, (LPTSTR)pAL->GetTitle());
				}
				lstrcat(str, AutoChar(pAL->GetTitle()));
			}
			else
			{
				wsprintf(str, "%s\n%s", pAL->GetAlbum(), pAL->GetArtist());
			}
			lstrcpyn(pLVDI->item.pszText, str, pLVDI->item.cchTextMax);
			break;

		case columnTime:		// time
			if (m_nTimeDisplay == 0)
			{
				int nTime = pAL->GetTotalTime();

				if (nTime > 3600)
				{
					int hour = nTime / 3600;
					int min = nTime % 3600;
					wsprintf(str, "%ld:%02ld:%02ld", hour, min / 60, min % 60);
				}
				else
					wsprintf(str, "%ld:%02ld", nTime / 60, nTime % 60);
			}
			else if (m_nTimeDisplay == 1)
				wsprintf(str, "%ld:%02ld", pAL->GetTotalTime() / 60, pAL->GetTotalTime() % 60);
			lstrcpyn(pLVDI->item.pszText, str, pLVDI->item.cchTextMax);
			break;

		case columnYear:		// year
			wsprintf(str, "%ld", pAL->GetYear());
			lstrcpyn(pLVDI->item.pszText, str, pLVDI->item.cchTextMax);
			break;

		case columnTrack:		// tracks
			wsprintf(str, "%ld", pAL->GetTrackCount());
			lstrcpyn(pLVDI->item.pszText, str, pLVDI->item.cchTextMax);
			break;

		case columnMultiDisc:
			wsprintf(str, "%ld", ((pAL->dwFlags & AI_MULTIDISC) != 0));
			lstrcpyn(pLVDI->item.pszText, str, pLVDI->item.cchTextMax);
			break;

		case columnPath:
			if (m_nShowFullPath)
			{
				if (pAL->IsPlaylistBased())
					lstrcpyn(str, AutoChar(pAL->GetM3U()), MAX_PATH);
				else
					lstrcpyn(str, AutoChar(pAL->GetPath()), MAX_PATH);
			}
			else
			{
				const wchar_t *ptr = NULL;

				if (pAL->IsPlaylistBased())
					ptr = pAL->GetM3U();
				else
					ptr = pAL->GetPath();

				AutoChar ptr2(ptr);

				if (lstrlen(ptr2) > pAL->nPathLen)
				{
					lstrcpyn(str, (char*)ptr2 + pAL->nPathLen + 1, MAX_PATH);
				}
			}
			lstrcpyn(pLVDI->item.pszText, str, pLVDI->item.cchTextMax);
			break;

		case columnArtist:
			lstrcpyn(pLVDI->item.pszText, AutoChar(pAL->GetArtist()), pLVDI->item.cchTextMax);
			break;

		case columnAlbum:
			lstrcpyn(pLVDI->item.pszText, AutoChar(pAL->GetAlbum()), pLVDI->item.cchTextMax);
			break;

		case columnGenre:
			lstrcpyn(pLVDI->item.pszText, AutoChar(pAL->GetGenre()), pLVDI->item.cchTextMax);
			break;
		}
		pAL->Release();
	}

	if (!bReportView && (pLVDI->item.mask & LVIF_IMAGE))
	{
		CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(pLVDI->item.iItem);
		if (pAL == NULL) return FALSE;

		if (pAL->GetIconIndex() >= 0)
		{
			pLVDI->item.iImage = pAL->GetIconIndex();
			DbgPrint("Profile %ld: Got image %ld for item\n", m_nProfileID, pLVDI->item.iImage, pLVDI->item.iItem);
		}
		else
		{
			DbgPrint("Profile %ld: Adding item %ld to cover queue\n", m_nProfileID, pLVDI->item.iItem);
			pLVDI->item.iImage = 1;
			EnterCriticalSection(&m_csCover);
			m_CoverArray.Add(pLVDI->item.iItem);
			LeaveCriticalSection(&m_csCover);
			SetEvent(m_hCoverEvent);
		}
		pAL->Release();
	}

	if (!bReportView && (pLVDI->item.mask & LVIF_STATE))
	{
		if (pLVDI->item.iItem == m_AlbumList.GetCurAlbumIndex())
		{
			pLVDI->item.state |= INDEXTOOVERLAYMASK(1);
		}
	}

	*pResult = 0;

	return TRUE;
}

BOOL CUserInterface::OnGetInfoTipUnicodeAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	LPNMLVGETINFOTIPW pLVGIT = (LPNMLVGETINFOTIPW)pnmh;

	if ((GetWindowLong(m_wndList, GWL_STYLE) & LVS_TYPEMASK) == LVS_REPORT)
		return FALSE;

	CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(pLVGIT->iItem);
	if (pAL == NULL) return FALSE;

	int cp = wndWinampAL.GetEncodingCodepage();

	WCHAR szTime[32] = L"";
	WCHAR str[1024] = L"";
	if (m_nTimeDisplay == 0)
	{
		int nTime = pAL->GetTotalTime();

		if (nTime > 3600)
		{
			int hour = nTime / 3600;
			int min = nTime % 3600;
			wsprintfW(szTime, L"%ld:%02ld:%02ld", hour, min / 60, min % 60);
		}
		else
			wsprintfW(szTime, L"%ld:%02ld", nTime / 60, nTime % 60);
	}
	else if (m_nTimeDisplay == 1)
	{
		wsprintfW(szTime, L"%ld:%02ld", pAL->GetTotalTime() / 60, pAL->GetTotalTime() % 60);
	}
	
	AutoWide a(ALS("Title"), wndWinampAL.GetCodepage());
	AutoWide b(ALS("Year"), wndWinampAL.GetCodepage());
	AutoWide c(ALS("Genre"), wndWinampAL.GetCodepage());
	AutoWide d(ALS("Length"), wndWinampAL.GetCodepage());
	AutoWide e(ALS("# of Tracks"), wndWinampAL.GetCodepage());
	_snwprintf(str, sizeof(str)/sizeof(WCHAR), L"%ls: %ls\n%ls: %ld\n%ls: %ls\n%ls: %ls\n%ls: %ld",
					(LPCWSTR)a, pAL->GetTitle(),
					(LPCWSTR)b, pAL->GetYear(), 
					(LPCWSTR)c, pAL->GetGenre(), 
					(LPCWSTR)d, szTime, 
					(LPCWSTR)e, pAL->GetTrackCount());

	wcsncpy(pLVGIT->pszText, str, pLVGIT->cchTextMax);

	pAL->Release();

	*pResult = TRUE;

	return TRUE;
}

BOOL CUserInterface::OnGetInfoTipAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	LPNMLVGETINFOTIP pLVGIT = (LPNMLVGETINFOTIP)pnmh;

	if ((GetWindowLong(m_wndList, GWL_STYLE) & LVS_TYPEMASK) == LVS_REPORT)
		return FALSE;

	CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(pLVGIT->iItem);
	if (pAL == NULL) return FALSE;

	char szTime[32] = "";
	char str[1024] = "";
	if (m_nTimeDisplay == 0)
	{
		int nTime = pAL->GetTotalTime();

		if (nTime > 3600)
		{
			int hour = nTime / 3600;
			int min = nTime % 3600;
			wsprintf(szTime, "%ld:%02ld:%02ld", hour, min / 60, min % 60);
		}
		else
			wsprintf(szTime, "%ld:%02ld", nTime / 60, nTime % 60);
	}
	else if (m_nTimeDisplay == 1)
		wsprintf(szTime, "%ld:%02ld", pAL->GetTotalTime() / 60, pAL->GetTotalTime() % 60);
	
	char a[64], b[64], c[64], d[64], e[64];
	lstrcpyn(a, ALS("Title"), 64);
	lstrcpyn(b, ALS("Year"), 64);
	lstrcpyn(c, ALS("Genre"), 64);
	lstrcpyn(d, ALS("Length"), 64);
	lstrcpyn(e, ALS("# of Tracks"), 64);
	_snprintf(str, sizeof(str), "%s: %s\n%s: %ld\n%s: %s\n%s: %s\n%s: %ld",
													a, pAL->GetTitle(), 
													b, pAL->GetYear(), 
													c, pAL->GetGenre(), 
													d, szTime, 
													e, pAL->GetTrackCount());

	lstrcpyn(pLVGIT->pszText, str, pLVGIT->cchTextMax);

	pAL->Release();

	*pResult = TRUE;

	return TRUE;
}

BOOL CUserInterface::OnColumnClickAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmh;

	HDITEM hdi;
	memset(&hdi, 0, sizeof(HDITEM));
	hdi.mask = HDI_LPARAM;
	m_wndList.GetDlgItem(0).SendMessage(HDM_GETITEM, pnmlv->iSubItem, (LPARAM)&hdi);

	switch (hdi.lParam)
	{
	case columnName:
		m_AlbumList.Sort(orderName);
		break;

	case columnTime:
		m_AlbumList.Sort(orderTotalTime);
		break;

	case columnYear:
		m_AlbumList.Sort(orderYear);
		break;

	case columnTrack:
		m_AlbumList.Sort(orderNumOfSongs);
		break;

	case columnPath:
		m_AlbumList.Sort(orderPath);
		break;

	case columnArtist:
		m_AlbumList.Sort(orderArtist);
		break;

	case columnAlbum:
		m_AlbumList.Sort(orderAlbum);
		break;

	case columnGenre:
		m_AlbumList.Sort(orderGenre);
		break;
	}

	return FALSE;
}

BOOL CUserInterface::OnBeginDragAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	CStringArrayW files;
	int nPos = -1;
	while ((nPos = m_wndList.SendMessage(LVM_GETNEXTITEM, nPos, MAKELPARAM(LVIS_SELECTED, 0))) != -1)
	{
		CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(nPos);
		if (pAlbum)
		{
			pAlbum->GetFileList(files);
			pAlbum->Release();
		}
	}

	UINT uBuffSize = 0;
	int size = files.GetSize();
	for (int i=0; i<size; ++i)
	{
		uBuffSize += wcslen(files[i]) + 1;
	}

	if (uBuffSize == 0) return FALSE;

	uBuffSize = sizeof(DROPFILES) + sizeof(WCHAR) * (uBuffSize + 1);

	HGLOBAL hGlobal = GlobalAlloc(GHND|GMEM_SHARE, uBuffSize);
	DROPFILES *pDrop = (DROPFILES *)GlobalLock(hGlobal);
	if (pDrop)
	{
		pDrop->pFiles = sizeof(DROPFILES);
		pDrop->fWide = TRUE;
		WCHAR *pszBuff = (WCHAR*) (LPBYTE(pDrop) + sizeof(DROPFILES));

		int size = files.GetSize();
		for (int i=0; i<size; ++i)
		{
			wcscpy (pszBuff, files[i]);
			pszBuff = wcschr (pszBuff, '\0') + 1;
		}
		*pszBuff = 0;

		GlobalUnlock(hGlobal);
	}

	IDataObject *pDataObject;
	IDropSource *pDropSource;
	DWORD		 dwEffect;
	DWORD		 dwResult;

	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };

	// transfer the current selection into the IDataObject
	stgmed.hGlobal = hGlobal;

	// Create IDataObject and IDropSource COM objects
	CreateDropSource(&pDropSource);
	CreateDataObject(&fmtetc, &stgmed, 1, &pDataObject);

	//
	//	The drag-drop operation!
	//
	dwResult = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK, &dwEffect);

	// success!
	if(dwResult == DRAGDROP_S_DROP)
	{
		if(dwEffect & DROPEFFECT_MOVE)
		{
			// remove selection from edit control
		}
	}
	// cancelled
	else if(dwResult == DRAGDROP_S_CANCEL)
	{
	}

	pDataObject->Release();
	pDropSource->Release();
	ReleaseStgMedium(&stgmed);

	return FALSE;
}

BOOL CUserInterface::OnHeaderRClickAlbumList(WPARAM wParam, LPARAM lParam)
{
	// create album specific menu
	CMenu menu;
	menu.CreatePopupMenu();

	//////////////////////////
	// turn on/off columns
	menu.AppendMenu(MF_STRING, IDC_HEADER_TITLE,		ALS("Name"));
	menu.AppendMenu(MF_STRING, IDC_HEADER_TIME,			ALS("Time"));
	menu.AppendMenu(MF_STRING, IDC_HEADER_YEAR,			ALS("Year"));
	menu.AppendMenu(MF_STRING, IDC_HEADER_NUMOFSONGS,	ALS("Tracks"));
	menu.AppendMenu(MF_STRING, IDC_HEADER_PATH,			ALS("Path"));
	menu.AppendMenu(MF_STRING, IDC_HEADER_ARTIST,		ALS("Artist"));
	menu.AppendMenu(MF_STRING, IDC_HEADER_ALBUM,		ALS("Album"));
	menu.AppendMenu(MF_STRING, IDC_HEADER_GENRE,		ALS("Genre"));
#ifdef _DEBUG
	menu.AppendMenu(MF_STRING, IDC_HEADER_MULTIDISC,	ALS("MultiDisc"));
#endif

	CWnd wndHeader = m_wndList.GetDlgItem(0);
	int nCount = Header_GetItemCount(wndHeader);

	if (IsColumnInserted(m_wndList, columnName))
	{
		menu.CheckMenuItem(IDC_HEADER_TITLE, MF_BYCOMMAND|MF_CHECKED);
		if (nCount == 1) menu.EnableMenuItem(IDC_HEADER_TITLE, MF_BYCOMMAND|MF_GRAYED);
	}

	if (IsColumnInserted(m_wndList, columnYear))
	{
		menu.CheckMenuItem(IDC_HEADER_YEAR, MF_BYCOMMAND|MF_CHECKED);
		if (nCount == 1) menu.EnableMenuItem(IDC_HEADER_YEAR, MF_BYCOMMAND|MF_GRAYED);
	}

	if (IsColumnInserted(m_wndList, columnTime))
	{
		menu.CheckMenuItem(IDC_HEADER_TIME, MF_BYCOMMAND|MF_CHECKED);
		if (nCount == 1) menu.EnableMenuItem(IDC_HEADER_TIME, MF_BYCOMMAND|MF_GRAYED);
	}

	if (IsColumnInserted(m_wndList, columnTrack))
	{
		menu.CheckMenuItem(IDC_HEADER_NUMOFSONGS, MF_BYCOMMAND|MF_CHECKED);
		if (nCount == 1) menu.EnableMenuItem(IDC_HEADER_NUMOFSONGS, MF_BYCOMMAND|MF_GRAYED);
	}

#ifdef _DEBUG
	if (IsColumnInserted(m_wndList, columnMultiDisc))
	{
		menu.CheckMenuItem(IDC_HEADER_MULTIDISC, MF_BYCOMMAND|MF_CHECKED);
		if (nCount == 1) menu.EnableMenuItem(IDC_HEADER_MULTIDISC, MF_BYCOMMAND|MF_GRAYED);
	}
#endif

	if (IsColumnInserted(m_wndList, columnPath))
	{
		menu.CheckMenuItem(IDC_HEADER_PATH, MF_BYCOMMAND|MF_CHECKED);
		if (nCount == 1) menu.EnableMenuItem(IDC_HEADER_PATH, MF_BYCOMMAND|MF_GRAYED);
	}

	if (IsColumnInserted(m_wndList, columnArtist))
	{
		menu.CheckMenuItem(IDC_HEADER_ARTIST, MF_BYCOMMAND|MF_CHECKED);
		if (nCount == 1) menu.EnableMenuItem(IDC_HEADER_ARTIST, MF_BYCOMMAND|MF_GRAYED);
	}

	if (IsColumnInserted(m_wndList, columnAlbum))
	{
		menu.CheckMenuItem(IDC_HEADER_ALBUM, MF_BYCOMMAND|MF_CHECKED);
		if (nCount == 1) menu.EnableMenuItem(IDC_HEADER_ALBUM, MF_BYCOMMAND|MF_GRAYED);
	}

	if (IsColumnInserted(m_wndList, columnGenre))
	{
		menu.CheckMenuItem(IDC_HEADER_GENRE, MF_BYCOMMAND|MF_CHECKED);
		if (nCount == 1) menu.EnableMenuItem(IDC_HEADER_GENRE, MF_BYCOMMAND|MF_GRAYED);
	}

	/////////////////////////////
	// column specific menu items
	if (wParam == columnName)
	{
		menu.AppendMenu(MF_SEPARATOR, 0, "");
		menu.AppendMenu(MF_STRING, IDC_SHOWINDEXNUM,	ALS("Show index number"));

		if (m_bShowIndexNum)
			menu.CheckMenuItem(IDC_SHOWINDEXNUM, MF_BYCOMMAND|MF_CHECKED);
	}
	else if (wParam == columnTime)
	{
		menu.AppendMenu(MF_SEPARATOR, 0, "");
		menu.AppendMenu(MF_STRING, IDC_TIMEFORMAT1,		ALS("Show time in &h:mm:ss"));
		menu.AppendMenu(MF_STRING, IDC_TIMEFORMAT2,		ALS("Show time in &mmm:ss"));

		if (m_nTimeDisplay == 0)
			menu.CheckMenuItem(IDC_TIMEFORMAT1, MF_BYCOMMAND|MF_CHECKED);
		else if (m_nTimeDisplay == 1)
			menu.CheckMenuItem(IDC_TIMEFORMAT2, MF_BYCOMMAND|MF_CHECKED);
	}
	else if (wParam == columnPath)
	{
		menu.AppendMenu(MF_SEPARATOR, 0, "");
		menu.AppendMenu(MF_STRING, IDC_SHOWFULLPATH,	ALS("Show full path"));
		if (m_nShowFullPath == 1)
			menu.CheckMenuItem(IDC_SHOWFULLPATH, MF_BYCOMMAND|MF_CHECKED);
	}

	//////////////////////////
	// sort related menu items
	char str[32];
	int path = 0, name = 0, artist = 0, album = 0, genre = 0, year = 0, tracks = 0, totaltime = 0;

	menu.AppendMenu(MF_SEPARATOR, 0, "");

	if (path) wsprintf(str, "[%ld]", path);	else str[0] = 0;
	menu.AppendMenu(MF_STRING, IDC_SORT_PATH,			ALS("Sort list by &path", str));

	if (name) wsprintf(str, "[%ld]", name);	else str[0] = 0;
	menu.AppendMenu(MF_STRING, IDC_SORT_NAME,			ALS("Sort list by &display name", str));

	if (artist) wsprintf(str, "[%ld]", artist);	else str[0] = 0;
	menu.AppendMenu(MF_STRING, IDC_SORT_ARTIST,		ALS("Sort list by &artist name", str));

	if (album) wsprintf(str, "[%ld]", album);	else str[0] = 0;
	menu.AppendMenu(MF_STRING, IDC_SORT_ALBUM,		ALS("Sort list by &album name", str));

	if (genre) wsprintf(str, "[%ld]", genre);	else str[0] = 0;
	menu.AppendMenu(MF_STRING, IDC_SORT_GENRE,		ALS("Sort list by &genre", str));

	if (year) wsprintf(str, "[%ld]", year);	else str[0] = 0;
	menu.AppendMenu(MF_STRING, IDC_SORT_YEAR,			ALS("Sort list by &year", str));

	if (tracks) wsprintf(str, "[%ld]", tracks);	else str[0] = 0;
	menu.AppendMenu(MF_STRING, IDC_SORT_NUMOFSONGS,	ALS("Sort list by &number of tracks", str));

	if (totaltime) wsprintf(str, "[%ld]", totaltime);	else str[0] = 0;
	menu.AppendMenu(MF_STRING, IDC_SORT_TOTALTIME,	ALS("Sort list by album total &time", str));

	menu.AppendMenu(MF_STRING, IDC_SORT_ARTIST_YEAR,	ALS("Sort list by artist name then year"));
	menu.AppendMenu(MF_STRING, IDC_SORT_SHUFFLE,		ALS("&Shuffle albums"));

	switch (m_AlbumList.GetSort())
	{
	case orderPath:
	case orderPathReverse:
		menu.CheckMenuItem(IDC_SORT_PATH, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderName:
	case orderNameReverse:
		menu.CheckMenuItem(IDC_SORT_NAME, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderArtist:
	case orderArtistReverse:
		menu.CheckMenuItem(IDC_SORT_ARTIST, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderAlbum:
	case orderAlbumReverse:
		menu.CheckMenuItem(IDC_SORT_ALBUM, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderGenre:
	case orderGenreReverse:
		menu.CheckMenuItem(IDC_SORT_GENRE, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderYear:
	case orderYearReverse:
		menu.CheckMenuItem(IDC_SORT_YEAR, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderNumOfSongs:
	case orderNumOfSongsReverse:
		menu.CheckMenuItem(IDC_SORT_NUMOFSONGS, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderTotalTime:
	case orderTotalTimeReverse:
		menu.CheckMenuItem(IDC_SORT_TOTALTIME, MF_BYCOMMAND|MF_CHECKED);
		break;
	}

	//////////////////////////////////
	// other header related menu items
	menu.AppendMenu(MF_SEPARATOR, 0, "");
	menu.AppendMenu(MF_STRING, IDC_AUTOHIDEHEADER,		ALS("A&uto hide header"));
	menu.AppendMenu(MF_STRING, IDC_AUTOSIZECOLUMN,		ALS("Auto size &columns"));

	if (m_bAutoHideHeader)
		menu.CheckMenuItem(IDC_AUTOHIDEHEADER, MF_BYCOMMAND|MF_CHECKED);

	if (m_bAutoSizeColumns)
		menu.CheckMenuItem(IDC_AUTOSIZECOLUMN, MF_BYCOMMAND|MF_CHECKED);

	//////////////////////////
	// show the menu
	POINT pt;
	GetCursorPos(&pt);
	DWORD dwFlags = /*TPM_RETURNCMD|TPM_NONOTIFY|*/TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_LEFTBUTTON;
	UINT nRet = menu.TrackPopupMenu(dwFlags, pt.x, pt.y, this, NULL);
	menu.DestroyMenu();

	return FALSE;
}

BOOL CUserInterface::OnDblClkAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmh;

	if (pnmlv->iItem != -1)
	{
		CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(pnmlv->iItem);
		if (pAL == NULL) return FALSE;

		int ed = wndWinampAL.GetSetting(settingEnqueueDefault);

		BOOL bRet = ed ? pAL->Enqueue() : pAL->Play();
		pAL->Release();
		return bRet;
	}
	return FALSE;
}

BOOL CUserInterface::OnLClickAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmh;

	UpdateStatus();

	// alt+lclick to enqueue
	if (GetKeyState(VK_MENU) & 0x8000)
	{
		if (pnmlv->iItem != -1)
		{
			CAlbum *pAL = (CAlbum *)m_AlbumList.GetAlbum(pnmlv->iItem);
			if (pAL == NULL) return FALSE;

			BOOL bRet = pAL->Enqueue();
			pAL->Release();
			return bRet;
		}
	}

	return FALSE;
}

BOOL CUserInterface::OnRClickAlbumList(NMHDR* pnmh, LRESULT* pResult)
{
	NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmh;

	BOOL bReportView = (BOOL)((GetWindowLong(m_wndList, GWL_STYLE) & LVS_TYPEMASK) == LVS_REPORT);

	// view submenu
	CMenu menuView;
	menuView.CreatePopupMenu();
	menuView.AppendMenu(MF_STRING, IDC_COVER_VIEW,	ALS("&Cover"));
	menuView.AppendMenu(MF_STRING, IDC_LIST_VIEW,	ALS("&Details"));
	menuView.AppendMenu(MF_SEPARATOR, 0, "");
	menuView.AppendMenu(MF_STRING, IDC_SHOW_STATUS,	ALS("&Show Status"));
	if (bReportView) menuView.AppendMenu(MF_STRING, IDC_SHOW_HEADER,	ALS("Show &Header"));
	else			 menuView.AppendMenu(MF_STRING, IDC_SHOW_LABEL,		ALS("Show &Label"));

	menuView.CheckMenuRadioItem(IDC_COVER_VIEW, IDC_LIST_VIEW, bReportView ? IDC_LIST_VIEW : IDC_COVER_VIEW, MF_BYCOMMAND);

	if (m_bShowHeader) menuView.CheckMenuItem(IDC_SHOW_HEADER, MF_BYCOMMAND|MF_CHECKED);
	if (m_bShowStatus) menuView.CheckMenuItem(IDC_SHOW_STATUS, MF_BYCOMMAND|MF_CHECKED);
	if (m_bShowLabel)  menuView.CheckMenuItem(IDC_SHOW_LABEL,  MF_BYCOMMAND|MF_CHECKED);

	// sort submenu
	CMenu menuSort;
	menuSort.CreatePopupMenu();
	menuSort.AppendMenu(MF_STRING, IDC_SORT_PATH,			ALS("&Path"));
	menuSort.AppendMenu(MF_STRING, IDC_SORT_NAME,			ALS("&Display name"));
	menuSort.AppendMenu(MF_STRING, IDC_SORT_ARTIST,			ALS("&Artist name"));
	menuSort.AppendMenu(MF_STRING, IDC_SORT_ALBUM,			ALS("&Album name"));
	menuSort.AppendMenu(MF_STRING, IDC_SORT_GENRE,			ALS("&Genre"));
	menuSort.AppendMenu(MF_STRING, IDC_SORT_YEAR,			ALS("&Year"));
	menuSort.AppendMenu(MF_STRING, IDC_SORT_NUMOFSONGS,		ALS("&Number of tracks"));
	menuSort.AppendMenu(MF_STRING, IDC_SORT_TOTALTIME,		ALS("&Total time"));
	menuSort.AppendMenu(MF_STRING, IDC_SORT_ARTIST_YEAR,	ALS("Artist name then year"));
	menuSort.AppendMenu(MF_SEPARATOR, 0, "");
	menuSort.AppendMenu(MF_STRING, IDC_SORT_SHUFFLE,		ALS("&Shuffle albums"));

	switch (m_AlbumList.GetSort())
	{
	case orderPath:
	case orderPathReverse:
		menuSort.CheckMenuItem(IDC_SORT_PATH, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderName:
	case orderNameReverse:
		menuSort.CheckMenuItem(IDC_SORT_NAME, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderArtist:
	case orderArtistReverse:
		menuSort.CheckMenuItem(IDC_SORT_ARTIST, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderAlbum:
	case orderAlbumReverse:
		menuSort.CheckMenuItem(IDC_SORT_ALBUM, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderGenre:
	case orderGenreReverse:
		menuSort.CheckMenuItem(IDC_SORT_GENRE, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderYear:
	case orderYearReverse:
		menuSort.CheckMenuItem(IDC_SORT_YEAR, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderNumOfSongs:
	case orderNumOfSongsReverse:
		menuSort.CheckMenuItem(IDC_SORT_NUMOFSONGS, MF_BYCOMMAND|MF_CHECKED);
		break;

	case orderTotalTime:
	case orderTotalTimeReverse:
		menuSort.CheckMenuItem(IDC_SORT_TOTALTIME, MF_BYCOMMAND|MF_CHECKED);
		break;
	}

	// create album specific menu
	CMenu menu, menuPlay, menuEnqueue, menuSendTo;
	menu.CreatePopupMenu();
	menuPlay.CreatePopupMenu();
	menuEnqueue.CreatePopupMenu();
	menuSendTo.CreatePopupMenu();

	if (pnmlv->iItem == -1)
	{
		menu.AppendMenu(MF_STRING, IDC_PLAY_RANDOM_ALBUM,	ALS("Play random album"));
		menu.AppendMenu(MF_STRING, IDC_PLAY_ALL_ALBUMS,		ALS("&Play all albums"));
		menu.AppendMenu(MF_SEPARATOR, 0, "");
		menu.AppendMenu(MF_POPUP|MF_STRING, (UINT)(HMENU)menuView, ALS("View", "Ctrl+V"));
		menu.AppendMenu(MF_POPUP|MF_STRING, (UINT)(HMENU)menuSort, ALS("&Sort By"));
		menu.AppendMenu(MF_STRING, IDC_JUMP_TO_ALBUM,		ALS("&Jump to album", "F3"));
		menu.AppendMenu(MF_STRING, IDC_QUICK_SCAN_ALBUM,	ALS("&Quick scan", "F5"));
		menu.AppendMenu(MF_STRING, IDC_RELOAD_ALBUM,		ALS("Re&load list", "Ctrl+R"));
		menu.AppendMenu(MF_STRING, IDC_EXPORT_ALBUM_LIST,	ALS("&Generate HTML list", "Ctrl+E"));

		menu.AppendMenu(MF_SEPARATOR, 0, "");
		menu.AppendMenu(MF_STRING, IDC_SHOW_PROPERTIES,		ALS("Pre&ferences...", "Ctrl+P"));

		if (m_AlbumList.GetSize() == 0)
		{
			menu.EnableMenuItem(IDC_SORT_SHUFFLE,		MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDC_JUMP_TO_ALBUM,		MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDC_PLAY_RANDOM_ALBUM,	MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDC_PLAY_ALL_ALBUMS,	MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDC_QUICK_SCAN_ALBUM,	MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDC_EXPORT_ALBUM_LIST,	MF_BYCOMMAND|MF_GRAYED);
		}
	}
	else
	{
		int nSelCount = ListView_GetSelectedCount(m_wndList);
		
		CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(pnmlv->iItem);

		menu.AppendMenu(MF_STRING, IDC_PLAY_ALBUM,			ALS("&Play", "Enter"));
		menu.AppendMenu(MF_STRING, IDC_ENQUEUE_ALBUM,		ALS("&Enqueue", "Shift+Enter"));
		menu.AppendMenu(MF_STRING, IDC_ENQUEUE_N_PLAY,		ALS("&Enqueue and play", "Ctrl+Enter"));
//		menu.AppendMenu(MF_STRING, IDC_ENQUEUE_AFTER_CUR,	ALS("&Enqueue after current", "Alt+Enter"));
		
		menu.AppendMenu(MF_POPUP|MF_STRING, (UINT)(HMENU)menuPlay, ALS("Play..."));
		menu.AppendMenu(MF_POPUP|MF_STRING, (UINT)(HMENU)menuEnqueue, ALS("Enqueue..."));
		
		if ((HMENU)menuPlay)
		{
			menuPlay.AppendMenu(MF_STRING, IDC_PLAY_ALL_ALBUMS,		ALS("Play &all albums"));
			menuPlay.AppendMenu(MF_STRING, IDC_PLAY_RANDOM_ALBUM,	ALS("Play random album"));
			if ((nSelCount == 1) && pAlbum && wcslen(pAlbum->GetArtist()))
			{
				if (menuPlay.IsUnicode())
				{
					WCHAR album_by[MAX_PATH];
					AutoWide play_all_albums_by(ALS("Play all albums &by"), wndWinampAL.GetCodepage());
					AutoWide play_random_album_by(ALS("Play random album &by"), wndWinampAL.GetCodepage());
					
					swprintf(album_by, L"%ls %ls", (LPCWSTR)play_all_albums_by, pAlbum->GetArtist());
					menuPlay.AppendMenu(MF_STRING, IDC_PLAY_ALBUM_BY_ARTIST, album_by);
					swprintf(album_by, L"%ls %ls", (LPCWSTR)play_random_album_by, pAlbum->GetArtist());
					menuPlay.AppendMenu(MF_STRING, IDC_PLAY_RANDOM_ALBUM_BY_ARTIST, album_by);
				}
				else
				{
					char album_by[MAX_PATH];
					wsprintf(album_by, "%s %s", ALS("Play all albums &by"), AutoChar(pAlbum->GetArtist()));
					menuPlay.AppendMenu(MF_STRING, IDC_PLAY_ALBUM_BY_ARTIST, album_by);
					wsprintf(album_by, "%s %s", ALS("Play random album &by"), AutoChar(pAlbum->GetArtist()));
					menuPlay.AppendMenu(MF_STRING, IDC_PLAY_RANDOM_ALBUM_BY_ARTIST, album_by);
				}
			}
		}
		
		if ((HMENU)menuEnqueue)
		{
			menuEnqueue.AppendMenu(MF_STRING, IDC_ENQUEUE_ALL_ALBUMS,	ALS("Enqueue &all albums"));
			menuEnqueue.AppendMenu(MF_STRING, IDC_ENQUEUE_RANDOM_ALBUM,	ALS("Enqueue random album"));
			if ((nSelCount == 1) && pAlbum && wcslen(pAlbum->GetArtist()))
			{
				if (menuPlay.IsUnicode())
				{
					WCHAR album_by[MAX_PATH];
					AutoWide enqueue_all_albums_by(ALS("Enqueue all albums &by"), wndWinampAL.GetCodepage());
					AutoWide enqueue_random_album_by(ALS("Enqueue random album &by"), wndWinampAL.GetCodepage());
					
					wsprintfW(album_by, L"%ls %ls", (LPCWSTR)enqueue_all_albums_by, pAlbum->GetArtist());
					menuEnqueue.AppendMenu(MF_STRING, IDC_ENQUEUE_ALBUM_BY_ARTIST, album_by);
					wsprintfW(album_by, L"%ls %ls", (LPCWSTR)enqueue_random_album_by, pAlbum->GetArtist());
					menuEnqueue.AppendMenu(MF_STRING, IDC_ENQUEUE_RANDOM_ALBUM_BY_ARTIST, album_by);
				}
				else
				{
					char album_by[MAX_PATH];
					wsprintf(album_by, "%s %s", ALS("Enqueue all albums &by"), AutoChar(pAlbum->GetArtist()));
					menuEnqueue.AppendMenu(MF_STRING, IDC_ENQUEUE_ALBUM_BY_ARTIST, album_by);
					wsprintf(album_by, "%s %s", ALS("Enqueue random album &by"), AutoChar(pAlbum->GetArtist()));
					menuEnqueue.AppendMenu(MF_STRING, IDC_ENQUEUE_RANDOM_ALBUM_BY_ARTIST, album_by);
				}
			}
		}

		// Library SendTo is supported
		memset(&sendto, 0, sizeof(librarySendToMenuStruct));
		if (IPC_LIBRARY_SENDTOMENU > 65536 && wndWinamp.SendIPCMessage((WPARAM)0, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
		{
			sendto.mode				= 1;
			sendto.hwnd				= GetSafeHwnd();
			sendto.build_start_id	= IDC_SENDTO_MENU_START;
			sendto.build_end_id		= IDC_SENDTO_MENU_END;
			sendto.data_type		= wndWinamp.IsWinamp551() ? 7/*ML_TYPE_FILENAMESW*/ : ML_TYPE_FILENAMES;
			sendto.build_hMenu		= menuSendTo;
			
			// build the send to submenu
			if (wndWinamp.SendIPCMessage((WPARAM)&sendto, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
			{
				sendto.mode = 2;
				menu.AppendMenu(MF_POPUP|MF_STRING, (UINT)(HMENU)menuSendTo, ALS("Send To..."));
			}
		}
		
		menu.AppendMenu(MF_SEPARATOR, 0, "");
		menu.AppendMenu(MF_POPUP|MF_STRING, (UINT)(HMENU)menuView, ALS("View", "Ctrl+V"));
		menu.AppendMenu(MF_POPUP|MF_STRING, (UINT)(HMENU)menuSort, ALS("&Sort By"));
		menu.AppendMenu(MF_STRING, IDC_JUMP_TO_ALBUM,		ALS("&Jump to album", "F3"));
		menu.AppendMenu(MF_STRING, IDC_QUICK_SCAN_ALBUM,	ALS("&Quick scan", "F5"));
		menu.AppendMenu(MF_STRING, IDC_RELOAD_ALBUM,		ALS("Re&load list", "Ctrl+R"));
		menu.AppendMenu(MF_STRING, IDC_EXPORT_ALBUM_LIST,	ALS("&Generate HTML list", "Ctrl+E"));
		menu.AppendMenu(MF_SEPARATOR, 0, "");
		menu.AppendMenu(MF_STRING, IDC_REFRESH_ALBUM,		ALS("&Refresh item(s)"));
		if (nSelCount == 1)
		{
			menu.AppendMenu(MF_STRING, IDC_SHOW_ALBUM_INFO,	ALS("Show &info...", "F2"));

			LPARAM IPC_IS_MBAPI_ABSENT = wndWinamp.SendIPCMessage((WPARAM)&"gen_mbapi", IPC_REGISTER_WINAMP_IPCMESSAGE);
			if (!wndWinamp.IsWinamp522() || !wndWinamp.SendIPCMessage(0,IPC_IS_MBAPI_ABSENT))
			{
				menu.AppendMenu(MF_STRING, IDC_SHOW_ALBUM_COVER,	ALS("Show &cover", "Ctrl+S"));
			}
		}
		menu.AppendMenu(MF_STRING, IDC_WRITE_PLAYLIST,	ALS("&Write playlist(s)", "Ctrl+W"));
		menu.AppendMenu(MF_STRING, IDC_EXPLORE,			ALS("E&xplore...", "Ctrl+O"));
		menu.AppendMenu(MF_SEPARATOR, 0, "");

		// add custom IPC menu entries
		CPtrArray *p = NULL;
		if (wndWinampAL.GetIPCMenu(&p))
		{
			BOOL bAdd = FALSE;
			int size = p->GetSize();
			for (int i=0; i<size; i++)
			{
				CIPCMenu *m = (CIPCMenu *)p->GetAt(i);
				if (m)
				{
					menu.AppendMenu(MF_STRING, IPC_ADD_MENU+i, m->szName);
					bAdd = TRUE;
				}
			}
			if (bAdd) menu.AppendMenu(MF_SEPARATOR, 0, "");
		}
		
		menu.AppendMenu(MF_STRING, IDC_SHOW_PROPERTIES,	ALS("Pre&ferences...", "Ctrl+P"));
		
		if (nSelCount > 1)
		{
			menu.EnableMenuItem(IDC_SHOW_ALBUM_INFO,	MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDC_SHOW_ALBUM_COVER,	MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDC_EXPLORE,			MF_BYCOMMAND|MF_GRAYED);
		}
		
		if (pAlbum) pAlbum->Release();
	}

	// disable stuff for Winamp5+ (if any)
	if (wndWinamp.IsWinamp5())
	{
	}
	// disable stuff for Winamp 2.9 (if any)
	else
	{
	}

	// set default (play)
	menu.SetDefaultItem(IDC_PLAY_ALBUM);

	POINT pt = pnmlv->ptAction;
	ClientToScreen(pnmlv->hdr.hwndFrom, &pt);

	DWORD dwFlags = TPM_RETURNCMD|TPM_NONOTIFY|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_LEFTBUTTON;
	UINT nRet = menu.TrackPopupMenu(dwFlags, pt.x, pt.y, this, NULL);

	OnCommand(0, nRet, 0);

	menu.DestroyMenu();
	menuView.DestroyMenu();
	menuSort.DestroyMenu();
	menuPlay.DestroyMenu();
	menuEnqueue.DestroyMenu();
	menuSendTo.DestroyMenu();

	return FALSE;
}

void CUserInterface::OnKey(int nKey)
{
	KillTimer(m_hWnd, TIMERID_KEYINPUT);

	int size = m_AlbumList.GetSize();

	// act as an enter
	if (nKey == -1)
	{
		m_wndList.SetCurSel(m_nKey-1);
		m_nKey = 0;
	}
	else if ((m_nKey * 10 + nKey) <= size)
	{
		SetTimer(m_hWnd, TIMERID_KEYINPUT, 1000, NULL);

		m_nKey = m_nKey * 10 + nKey;

		if (m_nKey * 10 > size)
		{
			OnKey(-1);
		}
		else
		{
			m_wndList.SetCurSel(m_nKey-1);
		}
	}
	else
	{
		SetTimer(m_hWnd, TIMERID_KEYINPUT, 1000, NULL);
	}
}

void CUserInterface::SetTimeFormat(int nTimeDisplay)
{
	m_nTimeDisplay = nTimeDisplay;

	Redraw();
}

void CUserInterface::Redraw()
{
	if (m_wndList && m_wndList.IsWindow())
	{
		int nFirst = ListView_GetTopIndex(m_wndList);
		int nLast = ListView_GetCountPerPage(m_wndList) + nFirst;
		ListView_RedrawItems(m_wndList, nFirst, nLast);
	}
}

void CUserInterface::ShowCover(int index)
{
	CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
	if (pAlbum)
	{
		pAlbum->ShowCover();
		pAlbum->Release();
	}
}

void CUserInterface::ShowPreference(CWnd hwnd, BOOL bShowOptional /*=FALSE*/)
{
	// initialize path tab
	CConfigPath path;
	path.m_SearchPaths.RemoveAll();
	int size = m_SearchPaths.GetSize();
	for (int i=0; i<size; ++i)
	{
		path.m_SearchPaths.Add(m_SearchPaths[i]);
	}

	wcsncpy(path.m_szExtList, m_szExtList, MAX_PATH);
	path.m_bStartupScan	= m_bStartupScan;
	path.m_bScanSubDir	= m_bScanSubDir;
	path.m_bScanCDROM	= m_bScanCDROM;
	path.m_bIgnorePL	= m_bIgnorePL;
	path.m_bIgnoreRootPL= m_bIgnoreRootPL;
	path.m_bPLOnly		= m_bPLOnly;

	// initialize naming tab
	CConfigNaming naming;
	naming.m_nDirStyle	= m_nDirStyle;
	naming.m_bFixTitles	= m_bFixTitles;
	naming.m_bMultiDiscFix	= m_bMultiDiscFix;
	naming.m_bUseID3	= m_bUseID3;
	naming.m_bIgnoreTHE	= m_bIgnoreTHE;
	wcsncpy(naming.m_szMultiDiscNames, m_szMultiDiscNames, MAX_PATH);

	// initialize cover tab
	CConfigCover cover;
	cover.m_nCoverWidth		= m_nCoverWidthSave;
	cover.m_nCoverHeight	= m_nCoverHeightSave;
	cover.m_nBorderWidth	= m_nBorderWidthSave;
	cover.m_bCustomBorderColor = m_bCustomBorderColor;
	cover.m_clrCoverBorder	= m_clrCoverBorder;
	cover.m_clrCoverText	= m_clrCoverText;
	cover.m_bCoverShadow	= m_bCoverShadow;
	cover.m_bDrawTitle		= m_bDrawTitle;
	cover.m_bCacheCovers	= m_bCacheCovers;
	cover.m_bSearchFolder	= m_bSearchFolder;
	cover.m_bSearchAltFolder= m_bSearchAltFolder;
	cover.m_bOverrideDefCover	= m_bOverrideDefCover;
	cover.m_bSearchMP3		= m_bSearchMP3;
	wcsncpy(cover.m_szDefaultCover, m_szDefaultCover, MAX_PATH);
	wcsncpy(cover.m_szCoverSearchExt, m_szCoverSearchExt, MAX_PATH);
	wcsncpy(cover.m_szAltFolder, m_szAltFolder, MAX_PATH);

	// global pages (included here for easy access)
	CConfigOption	option;
	CConfigLanguage language;
	CConfigHistory	history;
	CConfigAbout	about;

	// insert pages
	CPreference prf;
	prf.m_nLastConfig = m_nLastConfig;
	prf.AddPage(ALS("Media"),	&path);
	prf.AddPage(ALS("Title"),	&naming);
	prf.AddPage(ALS("Cover"),	&cover);

	if (bShowOptional)
	{
		prf.AddPage(ALS("Option"),	&option);
		prf.AddPage(ALS("Language"),&language);
		prf.AddPage(ALS("History"),	&history);
		prf.AddPage(ALS("About"),	&about);
	}

	if (prf.DoModal(&hwnd) == IDOK)
	{
		m_nLastConfig = prf.m_nLastConfig;

		// update new preference
		wcsncpy(m_szExtList, path.m_szExtList, MAX_PATH);
		m_bStartupScan		= path.m_bStartupScan;
		m_bScanSubDir		= path.m_bScanSubDir;
		m_bScanCDROM		= path.m_bScanCDROM;
		m_bIgnorePL			= path.m_bIgnorePL;
		m_bIgnoreRootPL		= path.m_bIgnoreRootPL;
		m_bPLOnly			= path.m_bPLOnly;
		m_nDirStyle			= naming.m_nDirStyle;
		m_bFixTitles		= naming.m_bFixTitles;
		m_bMultiDiscFix		= naming.m_bMultiDiscFix;
		m_bUseID3			= naming.m_bUseID3;
		m_bIgnoreTHE		= naming.m_bIgnoreTHE;
		wcsncpy(m_szMultiDiscNames, naming.m_szMultiDiscNames, MAX_PATH);
		m_nCoverWidthSave	= cover.m_nCoverWidth;
		m_nCoverHeightSave	= cover.m_nCoverHeight;
		m_nBorderWidthSave	= cover.m_nBorderWidth;
		m_clrCoverText		= cover.m_clrCoverText;
		m_bCoverShadow		= cover.m_bCoverShadow;
		m_bDrawTitle		= cover.m_bDrawTitle;
		m_bCacheCovers		= cover.m_bCacheCovers;
		m_bSearchFolder		= cover.m_bSearchFolder;
		m_bSearchAltFolder	= cover.m_bSearchAltFolder;
		m_bOverrideDefCover	= cover.m_bOverrideDefCover;
		m_bSearchMP3		= cover.m_bSearchMP3;
		wcsncpy(m_szDefaultCover, cover.m_szDefaultCover, MAX_PATH);
		wcsncpy(m_szCoverSearchExt, cover.m_szCoverSearchExt, MAX_PATH);
		wcsncpy(m_szAltFolder, cover.m_szAltFolder, MAX_PATH);

		if ((m_clrCoverBorder != cover.m_clrCoverBorder) ||
			(m_bCustomBorderColor != cover.m_bCustomBorderColor))
		{
			m_bCustomBorderColor = cover.m_bCustomBorderColor;
			m_clrCoverBorder = cover.m_clrCoverBorder;

			// insert cover mask
			SIZE size = { m_nCoverWidth, m_nCoverHeight };
			HBITMAP hBitmapMask = CreateOverlayImage(size);
			if (hBitmapMask)
			{
				int i = ImageList_AddMasked(m_hImageListIcon, hBitmapMask, RGB(0,0,0));
				ImageList_SetOverlayImage(m_hImageListIcon, i, 1);
				DeleteObject(hBitmapMask);
			}
			Redraw();
		}

		m_SearchPaths.RemoveAll();
		m_AlbumList.m_SearchPaths.RemoveAll();
		int size = path.m_SearchPaths.GetSize();
		for (int i=0; i<size; ++i)
		{
			m_SearchPaths.Add(path.m_SearchPaths[i]);
			m_AlbumList.AddPath(path.m_SearchPaths[i]);
		}

		// add CD-ROM
		if (m_bScanCDROM)
		{
			m_AlbumList.AddCDROMs();
		}

		// pass the parameters to the list management object
		wcsncpy(m_AlbumList.m_szExtList, m_szExtList, MAX_PATH);
		m_AlbumList.m_bScanSubDir		= m_bScanSubDir;
		m_AlbumList.m_bIgnorePL			= m_bIgnorePL;
		m_AlbumList.m_bIgnoreRootPL		= m_bIgnoreRootPL;
		m_AlbumList.m_bPLOnly			= m_bPLOnly;
		m_AlbumList.m_nDirStyle			= m_nDirStyle;
		m_AlbumList.m_bFixTitles		= m_bFixTitles;
		m_AlbumList.m_bMultiDiscFix		= m_bMultiDiscFix;
		m_AlbumList.m_bUseID3			= m_bUseID3;
		m_AlbumList.m_bIgnoreTHE		= m_bIgnoreTHE;
		m_AlbumList.m_clrCoverText		= m_clrCoverText;
		m_AlbumList.m_bCoverShadow		= m_bCoverShadow;
		m_AlbumList.m_bDrawTitle		= m_bDrawTitle;
		m_AlbumList.m_bSearchFolder		= m_bSearchFolder;
		m_AlbumList.m_bSearchAltFolder	= m_bSearchAltFolder;
		m_AlbumList.m_bOverrideDefCover	= m_bOverrideDefCover;
		m_AlbumList.m_bSearchMP3		= m_bSearchMP3;
		wcsncpy(m_AlbumList.m_szMultiDiscNames, m_szMultiDiscNames, MAX_PATH);
		wcsncpy(m_AlbumList.m_szDefaultCover, m_szDefaultCover, MAX_PATH);
		wcsncpy(m_AlbumList.m_szCoverSearchExt, m_szCoverSearchExt, MAX_PATH);
		wcsncpy(m_AlbumList.m_szAltFolder, m_szAltFolder, MAX_PATH);

		// ask for rescan
		if (path.m_bNeedRefresh || naming.m_bNeedRefresh)
		{
			char str[MAX_PATH];
			lstrcpy(str, ALS("Your scan parameters are changed."));
			lstrcat(str, "\n");
			lstrcat(str, ALS("Do you want to reload the list now?"));

			if (MessageBox(hwnd, str, "Album List", MB_YESNO|MB_ICONQUESTION) == IDYES)
			{
				ReloadAlbumList();
			}
		}
	}
}

void CUserInterface::JumpToAlbum()
{
	CJumpToAlbum jump;
	jump.m_pAlbumList = &m_AlbumList;
	jump.DoModal(this);
}

void CUserInterface::ToggleView()
{
	if (m_bCoverView)	ListView();
	else				CoverView();
}

void CUserInterface::CoverView()
{
	if ((GetWindowLong(m_wndList, GWL_STYLE) & LVS_TYPEMASK) == LVS_ICON)
		return;

//	ListView_SetUnicodeFormat(m_wndList, TRUE);

	m_wndList.ModifyStyle(LVS_REPORT, LVS_ICON);
	m_bCoverView = TRUE;

	int index = m_AlbumList.GetCurAlbumIndex();
	EnsureVisible(index);

	LayoutControls();

	// queue up all the albums to get covers
	if (m_bInitialized)
	{
		EnterCriticalSection(&m_csCover);
		int size = m_AlbumList.GetSize();
		for (int i=size-1; i>=0; i--)
		{
			m_CoverArray.Add(i);
		}
		LeaveCriticalSection(&m_csCover);
		SetEvent(m_hCoverEvent);
	}
}

void CUserInterface::ListView()
{
	if ((GetWindowLong(m_wndList, GWL_STYLE) & LVS_TYPEMASK) == LVS_REPORT)
		return;

//	ListView_SetUnicodeFormat(m_wndList, FALSE);

	m_wndList.ModifyStyle(LVS_ICON, LVS_REPORT);
	m_bCoverView = FALSE;

	int index = m_AlbumList.GetCurAlbumIndex();
	EnsureVisible(index);

	LayoutControls();
	AutoSizeColumns();
}

void CUserInterface::ToggleShowStatus()
{
	m_bShowStatus = !m_bShowStatus;

	if (m_bShowStatus)
	{
		LayoutControls();

		GetDlgItem(IDC_PLAYBTN).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ENQUEUEBTN).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TOTALALBUMTIME).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PREVALBUMBTN).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NEXTALBUMBTN).ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_PLAYBTN).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ENQUEUEBTN).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TOTALALBUMTIME).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PREVALBUMBTN).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NEXTALBUMBTN).ShowWindow(SW_HIDE);

		LayoutControls();
	}
}

void CUserInterface::ToggleShowHeader()
{
	m_bShowHeader = !m_bShowHeader;

	if (!m_bAutoHideHeader && m_bShowHeader)
	{
		m_wndList.ModifyStyle(0, LVS_NOCOLUMNHEADER);
		m_wndList.ModifyStyle(LVS_NOCOLUMNHEADER, 0);
	}
	else
		m_wndList.ModifyStyle(0, LVS_NOCOLUMNHEADER);
	AutoSizeColumns();
}

void CUserInterface::ToggleShowLabel()
{
	m_bShowLabel = !m_bShowLabel;

	if (m_bShowLabel)
	{
		ListView_SetIconSpacing(m_wndList, m_nCoverWidth + 3, m_nCoverHeight + 40);
	}
	else
	{
		ListView_SetIconSpacing(m_wndList, m_nCoverWidth + 3, m_nCoverHeight + 3);
	}

	InvalidateRect(m_wndList, NULL, TRUE);
}

void CUserInterface::ListSizeChanged()
{
	if (m_wndList)
	{
		ListView_SetItemCountEx(m_wndList, m_AlbumList.GetSize(), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

		Redraw();
	}

	UpdateStatus();
}

void CUserInterface::CurAlbumChanged(int index)
{
	Redraw();
	EnsureVisible(index);
	if (wndWinampAL.GetSetting(settingShowCover))
	{
		ShowCover(index);
	}
	wndWinampAL.SetAutoAdvance(this);
}

void CUserInterface::FinishedQuickScan()
{
	m_AlbumList.Save(m_szCacheFile);
}

void CUserInterface::EnsureVisible(int index)
{
	m_wndList.EnsureVisible(index, FALSE);
}

void CUserInterface::UpdateStatus()
{
	// update total album time status
	CWnd stsTotal  = GetDlgItem(IDC_TOTALALBUMTIME);
	if (stsTotal)
	{
		int size = m_AlbumList.GetSize();

		int nSelectedTime = 0;
		int nPos = -1;
		while ((nPos = m_wndList.SendMessage(LVM_GETNEXTITEM, nPos, MAKELPARAM(LVIS_SELECTED, 0))) != -1)
		{
			CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(nPos);
			if (pAlbum)
			{
				nSelectedTime += pAlbum->GetTotalTime();
				pAlbum->Release();
			}
		}

		char str[260], temp1[64], temp2[64];
		if (size == 0)
		{
			lstrcpy(str, ALS("0 items"));
		}
		else if (size == 1)
		{
			wsprintf(str, ALS("%ld item [%s/%s]"), m_AlbumList.GetSize(), FormatTime(temp1, nSelectedTime), FormatTime(temp2, m_AlbumList.GetTotalTime()));
		}
		else
		{
			wsprintf(str, ALS("%ld items [%s/%s]"), m_AlbumList.GetSize(), FormatTime(temp1, nSelectedTime), FormatTime(temp2, m_AlbumList.GetTotalTime()));
		}
		stsTotal.SetWindowText(str);
	}
}

void CUserInterface::InsertColumn(CWnd wndList, LPCTSTR szName, int nWidth, int nFormat, LPARAM lParam, BOOL bInsertLast /*=FALSE*/)
{
	CWnd wndHeader = wndList.GetDlgItem(0);

	// find the correct place to insert the column
	int size = Header_GetItemCount(wndHeader);
	int nCol = size;
	if (!bInsertLast)
	{
		for (int i=0; i<size; ++i)
		{
			HDITEM hdi;
			memset(&hdi, 0, sizeof(HDITEM));
			hdi.mask = HDI_LPARAM;
			wndHeader.SendMessage(HDM_GETITEM, i, (LPARAM)&hdi);
			if (hdi.lParam > lParam)
				break;
		}
		nCol = i;
	}

	// insert the column
	LVCOLUMN lvc;
	memset(&lvc, 0, sizeof(LVCOLUMN));

	lvc.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
	lvc.fmt = nFormat;
	lvc.pszText = (LPTSTR)szName;
	lvc.cx = nWidth;
	ListView_InsertColumn(wndList, nCol, &lvc);

	// set lParam so we know what that column is
	HDITEM hdi;
	memset(&hdi, 0, sizeof(HDITEM));
	hdi.mask = HDI_LPARAM;
	hdi.lParam = lParam;
	wndList.GetDlgItem(0).SendMessage(HDM_SETITEM, nCol, (LPARAM)&hdi);
}

void CUserInterface::RemoveColumn(CWnd wndList, int nID)
{
	CWnd wndHeader = wndList.GetDlgItem(0);

	int nCol = -1;
	int size = Header_GetItemCount(wndHeader);
	for (int i=0; i<size; ++i)
	{
		HDITEM hdi;
		memset(&hdi, 0, sizeof(HDITEM));
		hdi.mask = HDI_LPARAM;
		wndHeader.SendMessage(HDM_GETITEM, i, (LPARAM)&hdi);
		if (hdi.lParam == nID)
		{
			nCol = i;
			break;
		}
	}

	if (nCol != -1) ListView_DeleteColumn(wndList, nCol);
}

BOOL CUserInterface::IsColumnInserted(CWnd wndList, int nID)
{
	CWnd wndHeader = wndList.GetDlgItem(0);

	int size = Header_GetItemCount(wndHeader);
	for (int i=0; i<size; ++i)
	{
		HDITEM hdi;
		memset(&hdi, 0, sizeof(HDITEM));
		hdi.mask = HDI_LPARAM;
		wndHeader.SendMessage(HDM_GETITEM, i, (LPARAM)&hdi);
		if (hdi.lParam == nID)
			return TRUE;
	}

	return FALSE;
}

void CUserInterface::ToggleColumn(CWnd wndList, int nID)
{
	if (IsColumnInserted(wndList, nID))
		RemoveColumn(wndList, nID);
	else if (nID == columnName)
		InsertColumn(m_wndList, ALS("Name"), 190,  LVCFMT_LEFT, columnName);
	else if (nID == columnTime)
		InsertColumn(m_wndList, ALS("Time"), 50,  LVCFMT_RIGHT, columnTime);
	else if (nID == columnYear)
		InsertColumn(m_wndList, ALS("Year"), 50,  LVCFMT_RIGHT,  columnYear);
	else if (nID == columnTrack)
		InsertColumn(m_wndList, ALS("Tracks"), 50,  LVCFMT_RIGHT,  columnTrack);
	else if (nID == columnMultiDisc)
		InsertColumn(m_wndList, ALS("MultiDisc"), 50,  LVCFMT_RIGHT,  columnMultiDisc);
	else if (nID == columnPath)
		InsertColumn(m_wndList, ALS("Path"), 50,  LVCFMT_LEFT,  columnPath);
	else if (nID == columnArtist)
		InsertColumn(m_wndList, ALS("Artist"), 50,  LVCFMT_LEFT,  columnArtist);
	else if (nID == columnAlbum)
		InsertColumn(m_wndList, ALS("Album"), 100,  LVCFMT_LEFT,  columnAlbum);
	else if (nID == columnGenre)
		InsertColumn(m_wndList, ALS("Genre"), 50,  LVCFMT_LEFT,  columnGenre);

	// resize columns (if autosize is on)
	AutoSizeColumns();
}

void CUserInterface::ToggleAutoSizeColumns()
{
	if (m_bAutoSizeColumns = !m_bAutoSizeColumns)
	{
		AutoSizeColumns();
	}
}

void CUserInterface::AutoSizeColumns(int nNewWidth /*=0*/)
{
	if (!m_bAutoSizeColumns) return;

	CWnd wndList = GetDlgItem(IDC_ALBUMLIST);
	CWnd wndHeader = wndList.GetDlgItem(0);

	// pick a column to resize
	int resize_column = -1;
	int size = Header_GetItemCount(wndHeader);
	for (int i=0; i<size; ++i)
	{
		int index = Header_OrderToIndex(wndHeader, i);

		HDITEM hdi;
		memset(&hdi, 0, sizeof(HDITEM));
		hdi.mask = HDI_WIDTH|HDI_LPARAM;
		wndHeader.SendMessage(HDM_GETITEM, index, (LPARAM)&hdi);
		if (hdi.lParam == columnName)
		{
			resize_column = index;
			break;
		}
		else if (hdi.lParam == columnAlbum)
		{
			resize_column = index;
		}
		else if (hdi.lParam == columnArtist)
		{
			if (resize_column == -1)
				resize_column = index;
		}
	}

	// resize the column if we picked one
	if (resize_column != -1)
	{
		int offset = (ListView_GetItemCount(wndList)>ListView_GetCountPerPage(wndList)) ? 3 : 0;
		int width = 0;

		// add all the columns width	
		int size = Header_GetItemCount(wndHeader);
		for (int i=0; i<size; ++i)
		{
			if (i != resize_column)
			{
				width += ListView_GetColumnWidth(wndList, i);
			}
		}

		RECT rcList;
		wndList.GetClientRect(&rcList);
		if (nNewWidth == 0) nNewWidth = rcList.right;
		int column0width = max(60, nNewWidth-1 - width - offset);
		ListView_SetColumnWidth(wndList, resize_column, column0width);
	}

	//TODO: remove glitch when the number of item matches the height of the listctrl
}

BOOL CUserInterface::ProcessAccelerator(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ((wParam == VK_TAB) && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		wndWinamp.PostMessage(uMsg, wParam, lParam);
		return TRUE;
	}

	MSG msg;

	msg.hwnd = hwnd;
	msg.message = uMsg;
	msg.wParam = wParam;
	msg.lParam = lParam;
	msg.time = GetMessageTime();
	GetCursorPos(&msg.pt);

	return TranslateAccelerator(m_hWnd, m_hAccTable, &msg) != 0;
}

BOOL CUserInterface::DoLongOperation(int size)
{
	int ret = IDYES;
	int limit = wndWinampAL.GetSetting(settingPlayAllLimit);
	BOOL warn = (BOOL)wndWinampAL.GetSetting(settingPlayAllWarning);

	if (warn && (size > limit))
	{
		char str [255] = "";
		sprintf (str, ALS("This operation can potentially make Winamp less responsive.\nProceed to operate on these %ld albums?"), size);
		return (BOOL)(MessageBox(m_hWnd, str, ALS("Warning"), MB_YESNO|MB_ICONQUESTION) == IDYES);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// IPC functions

BOOL CUserInterface::IPC_PlayAlbum(DWORD index)
{
	CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
	if (pAlbum)
	{
		BOOL bRet = pAlbum->Play();
		pAlbum->Release();
		return bRet;
	}
	return FALSE;
}

BOOL CUserInterface::IPC_EnqueueAlbum(DWORD index)
{
	CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
	if (pAlbum)
	{
		BOOL bRet = pAlbum->Enqueue();
		pAlbum->Release();
		return bRet;
	}
	return FALSE;
}

DWORD CUserInterface::IPC_GetAlbumSize()
{
	return m_AlbumList.GetSize();
}

BOOL CUserInterface::IPC_PlayRandomAlbum()
{
	if (m_AlbumList.GetSize() == 0) return FALSE;

	return m_AlbumList.PlayRandomAlbum();
}

BOOL CUserInterface::IPC_EnqueueRandomAlbum()
{
	if (m_AlbumList.GetSize() == 0) return FALSE;

	return m_AlbumList.EnqueueRandomAlbum();
}

BOOL CUserInterface::IPC_PlayPreviousAlbum()
{
	if (m_AlbumList.GetSize() == 0) return FALSE;

	int loop = wndWinampAL.GetSetting(settingLoop);

	int index = m_AlbumList.GetCurAlbumIndex() - 1;

	if (index < 0) return loop ? IPC_PlayAlbum(IPC_GetAlbumSize()-1) : FALSE;

	return IPC_PlayAlbum(index);
}

BOOL CUserInterface::IPC_PlayNextAlbum()
{
	if (m_AlbumList.GetSize() == 0) return FALSE;

	int loop = wndWinampAL.GetSetting(settingLoop);

	int index = m_AlbumList.GetCurAlbumIndex() + 1;

	if ((DWORD)index >= IPC_GetAlbumSize()) return loop ? IPC_PlayAlbum(0) : FALSE;

	return IPC_PlayAlbum(index);
}

// play previous (different artist)
BOOL CUserInterface::IPC_PlayPreviousAlbumArtist()
{
	if (m_AlbumList.GetSize() == 0) return FALSE;

	int index = m_AlbumList.GetCurAlbumIndex();

	CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
	if (pAlbum == NULL) return FALSE;

	wchar_t szArtist[MAX_PATH] = L"";
	wcscpy(szArtist, pAlbum->artist);
	pAlbum->Release();

	int loop = wndWinampAL.GetSetting(settingLoop);

	while (TRUE)
	{
		if (0 < index)
		{
			index--;
		}
		else if (loop)
		{
			index = m_AlbumList.GetSize() - 1;
		}
		else
		{
			return FALSE;
		}

		CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
		if (pAlbum)
		{
			// different artist, play it
			if (wcsicmp(szArtist, pAlbum->artist) != 0)
			{
				pAlbum->Release();
				return IPC_PlayAlbum(index);
			}
			pAlbum->Release();
		}
	}
}

// play previous (different artist)
BOOL CUserInterface::IPC_PlayNextAlbumArtist()
{
	if (m_AlbumList.GetSize() == 0) return FALSE;

	int index = m_AlbumList.GetCurAlbumIndex();

	CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
	if (pAlbum == NULL) return FALSE;

	wchar_t szArtist[MAX_PATH] = L"";
	wcscpy(szArtist, pAlbum->artist);
	pAlbum->Release();

	int loop = wndWinampAL.GetSetting(settingLoop);

	while (TRUE)
	{
		if ((index + 1) < m_AlbumList.GetSize())
		{
			index++;
		}
		else if (loop)
		{
			index = 0;
		}
		else
		{
			return FALSE;
		}

		CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
		if (pAlbum)
		{
			// different artist, play it
			if (wcsicmp(szArtist, pAlbum->artist) != 0)
			{
				pAlbum->Release();
				return IPC_PlayAlbum(index);
			}
			pAlbum->Release();
		}
	}
}

BOOL CUserInterface::IPC_PlayAllAlbums()
{
	int size = m_AlbumList.GetSize();

	if (!DoLongOperation(size)) return FALSE;

	if (size)
	{
		// play first album
		CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(0);
		if (pAlbum)
		{
			pAlbum->Play();
			pAlbum->Release();
		}

		// enqueue the rest
		for (int i=1; i<size; ++i)
		{
			CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(i);
			if (pAlbum)
			{
				pAlbum->Enqueue();
				pAlbum->Release();
			}
		}
			
		return TRUE;
	}
	return FALSE;
}

BOOL CUserInterface::IPC_EnqueueAllAlbums()
{
	int size = m_AlbumList.GetSize();

	if (!DoLongOperation(size)) return FALSE;

	if (size)
	{
		// enqueue all albums
		for (int i=0; i<size; ++i)
		{
			CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(i);
			if (pAlbum)
			{
				pAlbum->Enqueue();
				pAlbum->Release();
			}
		}
			
		return TRUE;
	}
	return FALSE;
}

int CUserInterface::IPC_GetAlbumIndex()
{
	return m_AlbumList.GetCurAlbumIndex();
}

int CUserInterface::IPC_GetAlbumYear(DWORD index)
{
	CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
	if (pAlbum)
	{
		int nRet = pAlbum->GetYear();
		pAlbum->Release();
		return nRet;
	}
	return 0;
}

LPCTSTR CUserInterface::IPC_GetAlbumName(DWORD index)
{
	static char title[MAX_PATH] = "";

	CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
	if (pAlbum)
	{
		lstrcpyn(title, AutoChar(pAlbum->GetTitle(), wndWinampAL.GetEncodingCodepage()), MAX_PATH);
		pAlbum->Release();
		return title;
	}
	return NULL;
}

LPCTSTR CUserInterface::IPC_GetAlbumTitle(DWORD index)
{
	static char album[MAX_PATH] = "";

	CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
	if (pAlbum)
	{
		lstrcpyn(album, AutoChar(pAlbum->GetAlbum(), wndWinampAL.GetEncodingCodepage()), MAX_PATH);
		pAlbum->Release();
		return album;
	}
	return NULL;
}

LPCTSTR CUserInterface::IPC_GetAlbumArtist(DWORD index)
{
	static char artist[MAX_PATH] = "";

	CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(index);
	if (pAlbum)
	{
		lstrcpyn(artist, AutoChar(pAlbum->GetArtist(), wndWinampAL.GetEncodingCodepage()), MAX_PATH);
		pAlbum->Release();
		return artist;
	}
	return NULL;
}

void CUserInterface::IPC_ShowHide()
{
	if (m_bEmbedML) return;

	if (m_bShow)
	{
		GetParent().ShowWindow(SW_HIDE);
		m_bShow = FALSE;
	}
	else
	{
		::ShowWindow(wa_wnd.me, SW_SHOW);
		m_bShow = TRUE;
	}
}

void CUserInterface::IPC_JumpToAlbum()
{
	JumpToAlbum();
}

void CUserInterface::IPC_ShowPreference()
{
	ShowPreference(GetSafeHwnd(), TRUE/*show optional*/);
}

BOOL CUserInterface::IPC_PlayAlbumName(LPCTSTR szAlbumA)
{
	if (szAlbumA == NULL) return FALSE;
	if (lstrlen(szAlbumA) == 0) return FALSE;

	AutoWide szAlbum(szAlbumA);

	wchar_t str[MAX_PATH] = L"";
	wcsncpy(str, szAlbum, MAX_PATH);
	StrTrimW(str, L" \t\'\"");

	int size = m_AlbumList.GetSize();
	if (size)
	{
		for (int i=0; i<size; ++i)
		{
			CAlbum *pAlbum = (CAlbum *)m_AlbumList.GetAlbum(i);
			if (wcsicmp(str, pAlbum->album) == 0)
			{
				pAlbum->Play();
				pAlbum->Release();
				return TRUE;
			}
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////
// IUnknown interface

STDMETHODIMP_(ULONG) CUserInterface::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG) CUserInterface::Release()
{
	return 1;
}

STDMETHODIMP CUserInterface::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	*ppvObj = NULL;

	if (iid == IID_IUnknown)
		*ppvObj = static_cast<IUnknown*>(this);

	return (*ppvObj != NULL) ? S_OK : E_NOINTERFACE;
}

//////////////////////////////////////////////////////////////////
// IDropTarget interface

STDMETHODIMP CUserInterface::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	CStringArrayW files;
	if (!ReadHDropData(pDataObject, files)) return E_FAIL;

	files.RemoveAll();

	*pdwEffect = DROPEFFECT_LINK;

	if (m_pIDropHelper) m_pIDropHelper->DragEnter(GetSafeHwnd(), pDataObject, (LPPOINT)&pt, *pdwEffect);
	return S_OK;
}

STDMETHODIMP CUserInterface::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	*pdwEffect = DROPEFFECT_LINK;

	if (m_pIDropHelper) m_pIDropHelper->DragOver((LPPOINT)&pt, *pdwEffect);
	return S_OK;
}

STDMETHODIMP CUserInterface::DragLeave()
{
	if (m_pIDropHelper) m_pIDropHelper->DragLeave();
	return S_OK;
}

STDMETHODIMP CUserInterface::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	CStringArrayW files;
	if (!ReadHDropData(pDataObject, files)) return E_FAIL;

	// add in the new paths
	int size = files.GetSize();
	for (int i=0; i<size; ++i)
	{
		m_SearchPaths.Add(files[i]);
		m_AlbumList.AddPath(files[i]);
	}
	files.RemoveAll();

	// do a quick scan
	m_AlbumList.QuickScan();

	if (m_pIDropHelper) m_pIDropHelper->Drop(pDataObject, (LPPOINT)&pt, *pdwEffect);
	return S_OK;
}

BOOL CUserInterface::ReadHDropData(IDataObject *pDataObject, CStringArrayW &files)
{
	if (pDataObject == NULL)
		return FALSE;

	FORMATETC formatEtc;
	formatEtc.cfFormat = CF_HDROP;
	formatEtc.ptd = NULL;
	formatEtc.dwAspect = DVASPECT_CONTENT;
	formatEtc.lindex = -1;
	formatEtc.tymed = (DWORD) -1;

	STGMEDIUM stgMedium;
	if (FAILED(pDataObject->GetData(&formatEtc, &stgMedium)))
		return FALSE;

	if (stgMedium.tymed == TYMED_HGLOBAL)
	{
	    HDROP hdrop = (HDROP)GlobalLock(stgMedium.hGlobal);
		if (hdrop)
		{
			wchar_t szNextFile[MAX_PATH];
			UINT uNumFiles = DragQueryFileW(hdrop, -1, NULL, 0);
			for (UINT uFile=0; uFile<uNumFiles; ++uFile)
			{
				if (DragQueryFileW(hdrop, uFile, szNextFile, MAX_PATH) > 0)
				{
					DWORD attr = GetFileAttributesW(szNextFile);
					if ((attr != 0xFFFFFFFF) && ((attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
					{
						files.Add(szNextFile);
					}
				}
			}

	        GlobalUnlock(stgMedium.hGlobal);
		}
	}

	ReleaseStgMedium(&stgMedium);

	return (files.GetSize() > 0);
}

//////////////////////////////////////////////////////////////////
// Encoding functions

BOOL CUserInterface::CreateListFont()
{
	// delete old fonts (if created)
	if (m_hFont)		DeleteObject(m_hFont);
	m_hFont		= NULL;

	if (m_hImageList)	ImageList_Destroy(m_hImageList);
	m_hImageList = NULL;

	// recreate the font
	char *fontname = (char *) wndWinamp.SendIPCMessage(1, IPC_GET_GENSKINBITMAP);
	DWORD charset = (DWORD) wndWinamp.SendIPCMessage(2, IPC_GET_GENSKINBITMAP);
	int fontsize = (int) wndWinamp.SendIPCMessage(3, IPC_GET_GENSKINBITMAP);

	m_hFont = CreateFont(-fontsize, 0, 0, 0, FW_REGULAR, 0, 0, 0, charset, 0/*OUT_TT_PRECIS*/, 0, DRAFT_QUALITY, 0, fontname);

	// change the row height by change the image list size
	int height = fontsize;
	HDC hDC = GetDC(m_hWnd);
	if (hDC)
	{
		HFONT hFontOld = (HFONT)SelectObject(hDC, m_hFont);

		TEXTMETRIC tm;
		if (GetTextMetrics(hDC, &tm))
		{
			height = tm.tmHeight + tm.tmExternalLeading + 1;
		}
		SelectObject(hDC, hFontOld);
		ReleaseDC(m_hWnd, hDC);
	}

	m_hImageList = ImageList_Create(height, height, ILC_COLOR24, 0, 10);
	if (m_hImageList)
	{
		ListView_SetImageList(m_wndList, m_hImageList, LVSIL_SMALL);
	}

	return TRUE;
}

int CUserInterface::DrawTextUnicode(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
	int cp = 0;

	int encoding = wndWinampAL.GetSetting(settingEncodingLangAL);
	switch (encoding)
	{
	case langSystemDefault:
		return DrawText(hDC, lpString, nCount, lpRect, uFormat);
			
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
// Cover View Stuff

int CUserInterface::AddImage(HBITMAP hBitmap)
{
	int index = 0;

	if (hBitmap)
	{
		index = ImageList_Add(m_hImageListIcon, hBitmap, m_hEmptyMask);
		DeleteObject(hBitmap);
	}

	return index;
}

HBITMAP CUserInterface::CreateOverlayImage(SIZE size)
{
	// create offscreen surface
	HDC			hDC		= GetDC(GetDesktopWindow());
	HDC			hMem	= CreateCompatibleDC(hDC);
	HBITMAP		hBitmap	= CreateCompatibleBitmap(hDC, size.cx, size.cy);
	HBITMAP		oBitmap	= (HBITMAP)SelectObject(hMem, hBitmap);
	HBRUSH		hBrush	= CreateSolidBrush(m_bCustomBorderColor ? m_clrCoverBorder : WADlg_getColor(WADLG_ITEMFG));

	// fill the whole thing with the border color
	RECT rc = { 0, 0, size.cx, size.cy };
	FillRect(hMem, &rc, hBrush);

	// adjust for border width
	rc.left += m_nBorderWidth;
	rc.right -= m_nBorderWidth;
	rc.top += m_nBorderWidth;
	rc.bottom -= m_nBorderWidth;

	FillRect(hMem, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

	// cleanup
	SelectObject(hMem, oBitmap);
	DeleteObject(hBrush);
	DeleteDC(hMem);
	ReleaseDC(GetDesktopWindow(), hDC);

	return hBitmap;
}

void CUserInterface::CreateEmptyMask(SIZE size)
{
	LPBYTE pBits = NULL;

	ENSURE
	{
		int bytes = (size.cx + 31) / 8 * size.cy;

		pBits = new BYTE [bytes];

		memset(pBits, 0, bytes);

		m_hEmptyMask = CreateBitmap(size.cx, size.cy, 1, 1, pBits);
	}
	END_ENSURE;

	// cleanup
	if (pBits) delete [] pBits;
}

DWORD WINAPI CUserInterface::CoverThreadProc(LPVOID lpData)
{
	SetThreadName(-1, "CoverThreadProc");

	CoInitialize(NULL);

	CUserInterface *pUI = (CUserInterface *)lpData;
	if (pUI)
	{
		DbgPrint("Profile %ld: Starting CoverThreadProc...\n", pUI->m_nProfileID);
		while (!pUI->m_bShuttingDown)
		{
			WaitForSingleObject(pUI->m_hCoverEvent, INFINITE);
			DbgPrint("Profile %ld: Cover Event signalled\n", pUI->m_nProfileID);

			int nSize = 0;
			do
			{
				if (!pUI->m_wndList.IsWindow())
					break;

				int nItem = -1;

				// remove the last one to get the icon
				EnterCriticalSection(&pUI->m_csCover);
				nSize = pUI->m_CoverArray.GetSize();
				if (nSize)
				{
					nItem = (DWORD)pUI->m_CoverArray.GetAt(nSize-1);
					pUI->m_CoverArray.RemoveAt(nSize-1);
				}
				LeaveCriticalSection(&pUI->m_csCover);

				if (nItem != -1)
				{
					CAlbum *pAL = (CAlbum *)pUI->m_AlbumList.GetAlbum(nItem);
					if (pAL != NULL)
					{
						if (pAL->GetIconIndex() < 0)
						{
							DbgPrint("Profile %ld: Getting cover for item %ld\n", pUI->m_nProfileID, nItem);
							SIZE size = { pUI->m_nCoverWidth, pUI->m_nCoverHeight };
							pAL->SetIconIndex(pUI->AddImage(pAL->GetCover(size)));
							ListView_RedrawItems(pUI->m_wndList, nItem, nItem);
							pUI->m_bSaveCache = TRUE;
						}
						pAL->Release();
					}
				}

				// get number items remaining
				EnterCriticalSection(&pUI->m_csCover);
				nSize = pUI->m_CoverArray.GetSize();
				LeaveCriticalSection(&pUI->m_csCover);
			}
			while (!pUI->m_bShuttingDown && (nSize > 0));
		}
		// close handle and stuff
		CloseHandle(pUI->m_hCoverThread);
		pUI->m_hCoverThread		= NULL;
		pUI->m_dwCoverThreadId	= 0;
	}

	CoUninitialize();

	return 0;
}

HIMAGELIST CUserInterface::CreateImageList()
{
	HIMAGELIST himl = ImageList_Create(m_nCoverWidth, m_nCoverHeight, ILC_COLOR24|ILC_MASK, 0, 1);
	if (himl)
	{
		// insert cover mask
		SIZE size = { m_nCoverWidth, m_nCoverHeight };
		HBITMAP hBitmapMask = CreateOverlayImage(size);
		if (hBitmapMask)
		{
			int i = ImageList_AddMasked(himl, hBitmapMask, RGB(0,0,0));
			ImageList_SetOverlayImage(himl, i, 1);
			DeleteObject(hBitmapMask);
		}
		
		HBITMAP hCover = NULL;

		// load overriden default cover
		if (m_bOverrideDefCover)
		{
			LPWSTR ext = PathFindExtensionW(m_szDefaultCover);

			// load custom cover background
			if (wcsicmp(ext, L".png") == 0)
			{
				hCover = LoadPictureFile2 (m_szDefaultCover, size);
			}
			else
			{
				hCover = LoadPictureFile (m_szDefaultCover, size);
			}
		}

		// load built-in cover background
		if (hCover == NULL)
		{
			hCover = LoadBitmap(hDllInstance, MAKEINTRESOURCE(IDB_COVER));
		}

		// create default icon
		int index = ImageList_Add(himl, hCover, m_hEmptyMask);
		DeleteObject(hCover);
	}

	return himl;
}

HIMAGELIST CUserInterface::LoadImageList()
{
	HGLOBAL		hGlobal		= NULL;
	LPVOID		pvData		= NULL;
	LPSTREAM	pstm		= NULL;
	HANDLE		fp			= NULL;
	HIMAGELIST	himl		= NULL;

	// generate image list cache filename
	char szImageListCache[MAX_PATH];
	lstrcpy(szImageListCache, m_szCacheFile);
	PathRenameExtension(szImageListCache, ".ilc");

	ENSURE
	{
		if (!m_bCacheCovers)
			break;

		if (INVALID_HANDLE_VALUE == (fp = CreateFile(szImageListCache, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL))) FAIL;

		DWORD dwSize = GetFileSize(fp, NULL);

		// alloc memory based on file size
		if ((hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwSize)) == NULL)
			FAIL;

		if ((pvData = GlobalLock(hGlobal)) == NULL)
			FAIL;

		DWORD nBytesRead;
		if (!ReadFile(fp, pvData, dwSize, &nBytesRead, NULL))
			FAIL;

		CloseHandle(fp);

		if (SUCCEEDED(CreateStreamOnHGlobal(hGlobal, FALSE, &pstm)))
		{
			himl = ImageList_Read(pstm);

			pstm->Release();
		}

		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);

		return himl;
	}
	END_ENSURE;

	if (pvData) GlobalUnlock(hGlobal);
	if (hGlobal) GlobalFree(hGlobal);
	if (fp) CloseHandle(fp);

	DeleteFile(szImageListCache);

	return NULL;
}

void CUserInterface::SaveImageList(HIMAGELIST himl)
{
	LPSTREAM pstm = NULL;
	
	// generate image list cache filename
	char szImageListCache[MAX_PATH];
	lstrcpy(szImageListCache, m_szCacheFile);
	PathRenameExtension(szImageListCache, ".ilc");

	ENSURE
	{
		if (!m_bCacheCovers)
			break;

		if (m_nCoverWidth != m_nCoverWidthSave)
			break;

		if (m_nCoverHeight != m_nCoverHeightSave)
			break;

		if (FAILED(CreateStreamOnHGlobal(NULL, TRUE, &pstm)))
			FAIL;

		if (!ImageList_Write(himl, pstm))
			FAIL;

		HGLOBAL hGlobal = NULL;
		GetHGlobalFromStream(pstm, &hGlobal);
		if (hGlobal == NULL)
			FAIL;

		DWORD dwSize = GlobalSize(hGlobal);
		LPVOID ptr = GlobalLock(hGlobal);
		if (ptr == NULL)
			FAIL;

		HANDLE fp = NULL;
		if (INVALID_HANDLE_VALUE == (fp = CreateFile(szImageListCache, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)))
			FAIL;

		DWORD nBytesWritten;

		WriteFile(fp, ptr, dwSize, &nBytesWritten, NULL);

		FlushFileBuffers(fp);
		CloseHandle(fp);

		GlobalUnlock(hGlobal);

		pstm->Release();

		return;
	}
	END_ENSURE;

	if (pstm) pstm->Release();
	DeleteFile(szImageListCache);
}

//////////////////////////////////////////////////////////////////
// Media Library Stuff

INT_PTR CUserInterface::MLPluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	// check for any global messages here
	if (message_type >= ML_MSG_TREE_BEGIN && message_type <= ML_MSG_TREE_END)
	{
		if (param1 != m_nMLTreeItem) return 0;
		// local messages for a tree item

		switch (message_type)
		{
		case ML_MSG_TREE_ONCREATEVIEW:
			return (int)CreateInML((HWND)param2);

		case ML_MSG_TREE_ONCLICK:
			if (param2 == ML_ACTION_RCLICK)
			{
				return OnRclickML((HWND)param3);
			}
			return 0;

		case ML_MSG_ONSENDTOBUILD:
			DbgPrint("ML_MSG_ONSENDTOBUILD(%ld, %ld, %ld)\n", param1, param2, param3);
			return 0;
		}
	}
	else if (message_type == ML_MSG_CONFIG)
	{
		return 0;
	}
#if 0
	else if (message_type == ML_MSG_ONSENDTOBUILD)
	{
		DbgPrint("ML_MSG_ONSENDTOBUILD(%ld, %ld, %ld)\n", param1, param2, param3);

		extern INT_PTR ml_PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

		mlAddToSendToStruct s;
		s.context=param2;
		s.desc = strdup(GetTitle());
		s.user32 = (int)this;

		long libhwndipc = (LONG)wndWinamp.SendIPCMessage(WPARAM("LibraryGetWnd"), IPC_REGISTER_WINAMP_IPCMESSAGE);
		if (libhwndipc)
		{
			// now send the message and get the HWND 
			CWnd wndML = (HWND)wndWinamp.SendIPCMessage(-1, LPARAM(libhwndipc));

			wndML.SendMessage(WM_ML_IPC,(WPARAM)&s,ML_IPC_ADDTOSENDTO);
		}
		return 0;
	}
	else if (message_type == ML_MSG_ONSENDTOSELECT)
	{
		DbgPrint("ML_MSG_ONSENDTOSELECT(%ld, %s, %ld)\n", param1, param2, param3);
	}
#endif
		
	return 0;
}
