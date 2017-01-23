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
#ifndef __ALBUMLIST_H__
#define __ALBUMLIST_H__

#include "FileInfo.h"
#include "Util.h"
#include "Album.h"

enum SortOrder
{
	orderOff				= 0,
	orderPath				= 101,
	orderName,
	orderArtist,
	orderYear,
	orderNumOfSongs,
	orderTotalTime,
	orderAlbum,
	orderGenre,
	orderPathReverse		= 201,
	orderNameReverse,
	orderArtistReverse,
	orderYearReverse,
	orderNumOfSongsReverse,
	orderTotalTimeReverse,
	orderAlbumReverse,
	orderGenreReverse,
};

enum ListNotification
{
	notifyListSizeChanged = 2000,
	notifyContentChanged,
	notifyCurAlbumChanged,
	notifyFinishedQuickScan,
	notifyListLoaded,
	notifyAlbumChanged,
	notifyCoverChanged,
};

typedef void (CALLBACK* NotifyProc)(ListNotification ln, WPARAM wParam, LPARAM lParam, LPVOID pData);

//////////////////////////////////////////////////////////////////
// CAlbumList class

class CAlbumList : public CPtrArray
{
public:
	CAlbumList();				// constructor
	virtual ~CAlbumList();		// destructor

	// settings
	CStringArrayW	m_SearchPaths;				// array of search paths
	wchar_t			m_szExtList[MAX_PATH];
	BOOL			m_bScanSubDir;
	BOOL			m_bIgnorePL;
	BOOL			m_bIgnoreRootPL;
	BOOL			m_bPLOnly;
	int				m_nDirStyle;
    BOOL			m_bFixTitles;
	BOOL			m_bMultiDiscFix;
	BOOL			m_bUseID3;
	wchar_t			m_szMultiDiscNames[MAX_PATH];
	BOOL			m_bIgnoreTHE;
	CDWordArray		m_SortOrders;
	int				m_nRandomDup;
	wchar_t			m_szDefaultCover[MAX_PATH];
	wchar_t			m_szCoverSearchExt[MAX_PATH];
	COLORREF		m_clrCoverText;
	BOOL			m_bCoverShadow;
	BOOL			m_bDrawTitle;
	BOOL			m_bSearchFolder;
	BOOL			m_bSearchAltFolder;
	BOOL			m_bOverrideDefCover;
	BOOL			m_bSearchMP3;
	wchar_t			m_szAltFolder[MAX_PATH];

public:
	// implementation
	void Startup			();				// initializes with AL window handle
	void Shutdown			();				// clean-up

	void Archive			(BOOL bWrite);

	void Reload				();
	void QuickScan			();
	void StopScan			();
	BOOL IsStoppingScan		();

	void ScanPath			(LPCWSTR szPath);
	void CleanupPath		(LPCWSTR szPath);

	BOOL OkToQuit			();

	// list management
	void AddPath			(LPCWSTR path);
	void AddCDROMs			();
	void RemovePath			(LPCWSTR path);

	BOOL Load				(LPCTSTR pathname);
	BOOL Save				(LPCTSTR pathname);

	void Resort				();
	void Sort				(SortOrder order);
	void Sort				(SortOrder order1, SortOrder order2);
	SortOrder GetSort		();

	int  GetTotalTime		();

	void Shuffle			();

	BOOL PlayAlbumByArtist	(LPCWSTR artist, BOOL bEnqueue = FALSE);
	BOOL PlayRandomAlbum	(BOOL bEqueue = FALSE);
	BOOL PlayRandomAlbumByArtist(LPCWSTR artist, BOOL bEqueue = FALSE);

	BOOL EnqueueAlbumByArtist		(LPCWSTR artist)	{ return PlayAlbumByArtist(artist, TRUE);		};
	BOOL EnqueueRandomAlbum			()					{ return PlayRandomAlbum(TRUE);					};
	BOOL EnqueueRandomAlbumByArtist	(LPCWSTR artist)	{ return PlayRandomAlbumByArtist(artist, TRUE);	};

	CAlbum *GetAlbum		(int index);
	int  GetArtistAlbumCount(LPCWSTR artist);

	void SetCurAlbum		(GUID& id);
	GUID GetCurAlbum		();
	int  GetCurAlbumIndex	();

	void GenerateHTMLList	();

	void RequestNotification(NotifyProc proc, LPVOID pData);

	void ResetCovers		();

	// overrides
	void RemoveAll			();

	void NotifyAlbumChanged ();
	void NotifyCoverChanged ();

private:
	// member variables
	BOOL			m_bInitialized;
	DWORD			m_dwCDROMs;
	GUID			m_idCurAlbum;
	int				m_nTotalTime;

	BOOL			m_bStopScanning;

	int				m_nPathLen;

	HANDLE			m_hThread;					// thread handle
	DWORD			m_dwThreadId;				// thread ID

	wchar_t			m_szScanPath[MAX_PATH];

	// notification/callback
	NotifyProc		m_pCallback;				// callback function
	LPVOID			m_pCB_data;					// callback private data

private:
	BOOL ProcessPath			(LPCWSTR path, BOOL bSearchRoot);
	BOOL ProcessM3U				(LPCWSTR path);
	BOOL ProcessSubDirectory	(LPCWSTR path);

	BOOL AddAlbum				(CAlbum *pAlbumInfo);

	BOOL PathExists				(LPCTSTR path, BOOL &bMultiDisc);
	CAlbum *FindAlbumByPath		(LPCWSTR path);
	BOOL M3UExists				(LPCWSTR path, LPCWSTR filename);
	BOOL CheckSubDirectories	(LPCWSTR path, WIN32_FIND_DATAW *file);

	BOOL ScanForNewAlbum		();
	BOOL AlbumPartOfSearchPath	(LPCWSTR path);

	BOOL IsCDROM				(LPCWSTR path);

	static DWORD WINAPI ReloadThreadProc		(LPVOID lpData);
	static DWORD WINAPI QuickScanThreadProc		(LPVOID lpData);
	static DWORD WINAPI ScanPathThreadProc		(LPVOID lpData);
	static DWORD WINAPI CleanupPathThreadProc	(LPVOID lpData);

	static int CompareAlbums	(const void *elem1, const void *elem2);
	int Compare					(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel);
	int ComparePath				(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse = FALSE);
	int CompareName				(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse = FALSE);
	int CompareArtist			(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse = FALSE);
	int CompareAlbum			(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse = FALSE);
	int CompareGenre			(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse = FALSE);
	int CompareYear				(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse = FALSE);
	int CompareNumOfSongs		(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse = FALSE);
	int CompareTotalTime		(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse = FALSE);

	CMapStringToString			m_PlayHistory;
};

#endif /* __ALBUMLIST_H__ */
