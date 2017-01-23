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
#ifndef _USERINTERFACE_H_
#define _USERINTERFACE_H_

#include <oleidl.h>
#include "NxSThingerAPI.h"
#include "Util.h"
#include "AlbumList.h"
#include "WinampGen.h"
#include "UserInterface_ListCtrl.h"

enum ListColumn
{
	columnName,
	columnTime,
	columnYear,
	columnTrack,
	columnPath,
	columnArtist,
	columnAlbum,
	columnGenre,
	columnMultiDisc,
};

#define WM_AL_POSTINIT		(WM_USER+1)

//////////////////////////////////////////////////////////////////
// CUserInterface class

struct IDropTargetHelper;
class CUserInterface : public CDialog, IDropTarget
{
	friend class CWinampAL;
	friend class CTabbedUI;
	friend class CWinampGen;
	friend class CUserInterfaceListCtrl;
	friend class CUserInterfaceHeaderCtrl;

public:
	CUserInterface(int id);
	virtual ~CUserInterface();

	BOOL Create					();
	HWND CreateInML				(CWnd wndParent);
	BOOL OnRclickML				(CWnd wndParent);
	int  GetTreeItemID			();

	BOOL Startup				();
	BOOL Shutdown				();
	void Cleanup				();

	BOOL Archive				(BOOL bWrite);

	BOOL ProcessWinampMenu		(int nID);

	void Redraw					();
	void ShowCover				(int index);
	void ShowPreference			(CWnd hwnd, BOOL bShowOptional = FALSE);
	void JumpToAlbum			();

	int		GetProfileID		();
	LPCTSTR GetProfileName		();
	void	SetProfileName		(LPCTSTR name);
	int		GetMenuID			();

	BOOL OkToQuit				();
	void ResetFont				();

	// IPC functions
	BOOL	IPC_PlayAlbum		(DWORD index);
	BOOL	IPC_EnqueueAlbum	(DWORD index);
	DWORD	IPC_GetAlbumSize	();
	BOOL	IPC_PlayRandomAlbum	();
	BOOL	IPC_PlayPreviousAlbum();
	BOOL	IPC_PlayNextAlbum	();
	BOOL	IPC_PlayPreviousAlbumArtist();
	BOOL	IPC_PlayNextAlbumArtist();
	BOOL	IPC_PlayAllAlbums	();
	BOOL	IPC_EnqueueRandomAlbum();
	BOOL	IPC_EnqueueAllAlbums();
	int		IPC_GetAlbumIndex	();
	int		IPC_GetAlbumYear	(DWORD index);
	LPCTSTR IPC_GetAlbumName	(DWORD index);
	LPCTSTR IPC_GetAlbumTitle	(DWORD index);
	LPCTSTR IPC_GetAlbumArtist	(DWORD index);
	void	IPC_ShowHide		();
	void	IPC_JumpToAlbum		();
	void	IPC_ShowPreference	();
	BOOL	IPC_PlayAlbumName	(LPCTSTR szAlbum);

public:		// Drag & Drop implementation
	// IUnknown
	STDMETHOD_(ULONG, AddRef)	();
	STDMETHOD_(ULONG, Release)	();
	STDMETHOD(QueryInterface)	(REFIID iid, LPVOID* ppvObj);
	// IDropTarget
	STDMETHOD(DragEnter)		(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHOD(DragOver)			(DWORD, POINTL, LPDWORD);
	STDMETHOD(DragLeave)		();
	STDMETHOD(Drop)				(LPDATAOBJECT, DWORD, POINTL pt, LPDWORD);

protected:
	virtual BOOL OnInitDialog	();
	virtual BOOL OnPaint		(HDC hdc);
	virtual BOOL OnSize			(UINT nType, int cx, int cy);
	virtual BOOL OnDisplayChange(int cBitsPerPixel, int cx, int cy);
	virtual BOOL OnClose		();
	virtual BOOL OnDestroy		();
	virtual BOOL OnCommand		(WORD wNotifyCode, WORD wID, CWnd wndCtl);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify		(UINT nID, NMHDR *pnmh, LRESULT *pResult);
	virtual BOOL OnDeviceChange	(UINT nEventType, DWORD dwData);

	BOOL OnCustomDrawAlbumList	(NMHDR* pnmh, LRESULT* pResult);
	BOOL OnCoolSBCustomDrawAlbumList(NMHDR* pnmh, LRESULT* pResult);
	BOOL OnGetDispInfoAlbumList	(NMHDR* pnmh, LRESULT* pResult);
	BOOL OnGetDispInfoUnicodeAlbumList	(NMHDR* pnmh, LRESULT* pResult);
	BOOL OnGetInfoTipAlbumList	(NMHDR* pnmh, LRESULT* pResult);
	BOOL OnGetInfoTipUnicodeAlbumList(NMHDR* pnmh, LRESULT* pResult);
	BOOL OnDblClkAlbumList		(NMHDR* pnmh, LRESULT* pResult);
	BOOL OnLClickAlbumList		(NMHDR* pnmh, LRESULT* pResult);
	BOOL OnRClickAlbumList		(NMHDR* pnmh, LRESULT* pResult);
	BOOL OnColumnClickAlbumList	(NMHDR* pnmh, LRESULT* pResult);
	BOOL OnBeginDragAlbumList	(NMHDR* pnmh, LRESULT* pResult);

	BOOL OnHeaderRClickAlbumList(WPARAM wParam, LPARAM lParam);

	BOOL OnPostInit				();
	BOOL InitML					(int nMLTreeItem);

	void OnKey					(int nKey);

	void SetTimeFormat			(int nTimeDisplay);

	void ToggleAutoSizeColumns	();
	void AutoSizeColumns		(int nNewWidth = 0);
	void InsertColumn			(CWnd wndList, LPCTSTR szName, int nWidth, int nFormat, LPARAM lParam, BOOL bInsertLast = FALSE);
	void RemoveColumn			(CWnd wndList, int nID);
	BOOL IsColumnInserted		(CWnd wndList, int nID);
	void ToggleColumn			(CWnd wndList, int nID);
	void ToggleShowLabel		();

	void ToggleView				();
	void CoverView				();
	void ListView				();

	void ListSizeChanged		();
	void CurAlbumChanged		(int index);
	void FinishedQuickScan		();
	LPCTSTR GetTitle			();
	LPCTSTR GetMLTitle			();
	void UpdateStatus			();
	void ToggleShowStatus		();
	void ToggleShowHeader		();
	void EnsureVisible			(int index);
	void LayoutControls			();
	void ReloadAlbumList		();

	BOOL CreateListFont			();
	int DrawTextUnicode			(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat);

	BOOL ProcessAccelerator		(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR MLPluginMessageProc	(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

	BOOL ReadHDropData			(IDataObject *pDataObject, CStringArrayW &files);

	void Search					(LPCTSTR szInputBuffer);

	BOOL DoLongOperation		(int size);

	// cover view stuff
	int AddImage				(HBITMAP hBitmap);
	HBITMAP CreateOverlayImage	(SIZE size);
	void CreateEmptyMask		(SIZE size);
	HIMAGELIST CreateImageList	();
	HIMAGELIST LoadImageList	();
	void SaveImageList			(HIMAGELIST himl);

	static void CALLBACK Notify	(ListNotification ln, WPARAM wParam, LPARAM lParam, LPVOID pData);
	static DWORD WINAPI CoverThreadProc(LPVOID lpData);

private:
	BOOL						m_bSaveCache;
	BOOL						m_bSaveCoverCache;
	BOOL						m_bInitialized;
	int							m_nProfileID;
	char						m_szProfileName[MAX_PATH];
	char						m_szTitle[MAX_PATH];
	embedWindowState			wa_wnd;
	NxSThingerIconStruct		ntis;
	librarySendToMenuStruct		sendto;

	CAlbumList					m_AlbumList;
	CWinampGen					m_wndWinampGen;
	CUserInterfaceListCtrl		m_wndList;
	CUserInterfaceListCtrl		m_wndList2;
	CUserInterfaceHeaderCtrl	m_wndHeader;

	HFONT						m_hFont;
	HBITMAP						m_hEmptyMask;
	HIMAGELIST					m_hImageList;
	HIMAGELIST					m_hImageListIcon;
	HACCEL						m_hAccTable;

	BOOL						m_bShow;
	char						m_szCacheFile[MAX_PATH];

	int							m_nMenuShow;
	int							m_nTimeDisplay;
	BOOL						m_bAutoSizeColumns;
	char						m_ColumnWidths[64];
	int							m_nLastConfig;
	BOOL						m_bShowIndexNum;
	BOOL						m_nShowFullPath;
	BOOL						m_bAutoHideHeader;
	BOOL						m_bShowHeader;
	BOOL						m_bShowStatus;
	BOOL						m_bShowLabel;
	int							m_nRandomDup;

	int							m_nKey;
	BOOL						m_bShuttingDown;

	int							m_nMLTreeItem;
	BOOL						m_bEmbedML;

	HANDLE						m_hCoverThread;
	DWORD						m_dwCoverThreadId;
	HANDLE						m_hCoverEvent;
	CRITICAL_SECTION			m_csCover;
	CDWordArray					m_CoverArray;

	// cover view
	BOOL						m_bCoverView;
	int							m_nCoverWidth;
	int							m_nCoverHeight;
	int							m_nBorderWidth;
	BOOL						m_bCustomBorderColor;
	COLORREF					m_clrCoverBorder;
	COLORREF					m_clrCoverText;
	BOOL						m_bCoverShadow;
	BOOL						m_bDrawTitle;
	BOOL						m_bCacheCovers;
	wchar_t						m_szDefaultCover[MAX_PATH];
	wchar_t						m_szCoverSearchExt[MAX_PATH];
	BOOL						m_bSearchFolder;
	BOOL						m_bSearchAltFolder;
	wchar_t						m_szAltFolder[MAX_PATH];
	BOOL						m_bOverrideDefCover;
	BOOL						m_bSearchMP3;

	// saved cover view values
	int							m_nCoverWidthSave;
	int							m_nCoverHeightSave;
	int							m_nBorderWidthSave;

	// drag & drop
	IDropTargetHelper*			m_pIDropHelper;
	HGLOBAL						m_hGlobal;

	// config path
	CStringArrayW				m_SearchPaths;
	wchar_t						m_szExtList[MAX_PATH];
	BOOL						m_bStartupScan;
	BOOL						m_bScanSubDir;
	BOOL						m_bScanCDROM;
	BOOL						m_bIgnorePL;
	BOOL						m_bIgnoreRootPL;
	BOOL						m_bPLOnly;

	// config naming
	int							m_nDirStyle;
    BOOL						m_bFixTitles;
	BOOL						m_bMultiDiscFix;
	BOOL						m_bUseID3;
	wchar_t						m_szMultiDiscNames[MAX_PATH];
	BOOL						m_bIgnoreTHE;
};

#endif /* _USERINTERFACE_H_ */

