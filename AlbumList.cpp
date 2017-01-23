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
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <search.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mbstring.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "Util.h"

#include "AlbumList.h"

#include "AutoChar.h"

//////////////////////////////////////////////////////////////////
// CAlbumList class

CAlbumList::CAlbumList() : CPtrArray()
{
	m_pCompare			= CompareAlbums;

	m_bInitialized		= FALSE;
	m_dwCDROMs			= 0;
	m_nTotalTime		= 0;
	memset(&m_idCurAlbum, 0, sizeof(GUID));

	m_hThread			= 0;
	m_dwThreadId		= 0;
	m_bStopScanning		= FALSE;
	m_nPathLen			= 0;

	m_pCallback			= NULL;
	m_pCB_data			= NULL;

	memset(m_szExtList, 0, MAX_PATH);
	m_bScanSubDir		= TRUE;
	m_bIgnorePL			= FALSE;
	m_bIgnoreRootPL		= FALSE;
	m_bPLOnly			= FALSE;
	m_nDirStyle			= 0;
    m_bFixTitles		= TRUE;
	m_bMultiDiscFix		= FALSE;
	m_bUseID3			= TRUE;
	m_bIgnoreTHE		= TRUE;
	wcscpy(m_szMultiDiscNames, L"cd, disc");
	m_nRandomDup		= 50;

	memset(m_szDefaultCover, 0, sizeof(m_szDefaultCover));
	memset(m_szCoverSearchExt, 0, sizeof(m_szCoverSearchExt));
	m_clrCoverText		= RGB(255,255,255);
	m_bCoverShadow		= FALSE;
	m_bDrawTitle		= TRUE;
	m_bSearchFolder		= TRUE;
	m_bSearchAltFolder	= FALSE;
	m_bOverrideDefCover	= FALSE;
	m_bSearchMP3		= TRUE;
	memset(m_szAltFolder, 0, sizeof(m_szAltFolder));
}

CAlbumList::~CAlbumList()
{
}

void CAlbumList::Startup()
{
	m_bInitialized = TRUE;

	char szDrive[8] = "C:\\";
	m_dwCDROMs = GetLogicalDrives();
	for (int i=0; i<26; i++)
	{
		if ((m_dwCDROMs >> i) & 1)
		{
			szDrive[0] = 'A' + i;
			if (GetDriveType(szDrive) != DRIVE_CDROM)
				m_dwCDROMs &= ~(1 << i);
		}
	}

	LoadExternalFileReader();
}

void CAlbumList::Shutdown()
{
	if (!m_bInitialized) return;

	// wait for it to finish
	while (m_hThread) Sleep(250);

	int size = GetSize();
	if (size)
	{
		CCriticalSection cs(&m_csMutex);

		for (int i=0; i<size; i++)
		{
			CAlbum *pAlbum = (CAlbum *)GetAt(i);
			if (pAlbum) pAlbum->Release();
		}
		RemoveAll();
	}

	UnloadExternalFileReader();

	m_bInitialized = FALSE;
}

void CAlbumList::StopScan()
{
	// change priority back to normal so it finishes faster
	if (m_hThread != NULL)
	{
		m_bStopScanning	= TRUE;

		SetThreadPriority(m_hThread, THREAD_PRIORITY_NORMAL);

		// wait 2.5 seconds for it to finish
		int i = 10;
		while (m_hThread && i--) Sleep(250);

		// force it 
		if (m_hThread)
		{
			TerminateThread(m_hThread, 0);
			CloseHandle(m_hThread);
			m_hThread		= NULL;
			m_dwThreadId	= NULL;
		}
		m_bStopScanning	= FALSE;
	}
}

BOOL CAlbumList::IsStoppingScan()
{
	return m_bStopScanning;
}

BOOL CAlbumList::OkToQuit()
{
	// change priority back to normal so it finishes faster
	if (m_hThread != NULL)
	{
		// stop all callbacks
		m_pCallback		= NULL;
		m_pCB_data		= NULL;
		m_bStopScanning = TRUE;

		SetThreadPriority(m_hThread, THREAD_PRIORITY_NORMAL);
	}

	return TRUE;
}

void CAlbumList::Reload()
{
	if (m_hThread)
	{
		StopScan();
	}
	m_hThread = CreateThread(NULL, 0, ReloadThreadProc, this, 0, &m_dwThreadId);
	SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
}

DWORD WINAPI CAlbumList::ReloadThreadProc(LPVOID lpData)
{
	SetThreadName(-1, "ReloadThreadProc");

	CAlbumList *pAL = (CAlbumList *)lpData;
	if (pAL)
	{
		// remove everything
		{
			CCriticalSection cs(&pAL->m_csMutex);
			int size = pAL->GetSize();
			for (int i=0; i<size; ++i)
			{
				CAlbum *pAlbum = (CAlbum *)pAL->GetAt(i);
				if (pAlbum) pAlbum->Release();
			}
			pAL->RemoveAll();
		}

		// scan again
		pAL->ScanForNewAlbum();

		// callback
		if (pAL->m_pCallback)
		{
			pAL->m_pCallback(notifyListSizeChanged, 0, 0, pAL->m_pCB_data);
		}

		// close handle and stuff
		CloseHandle(pAL->m_hThread);
		pAL->m_hThread		= NULL;
		pAL->m_dwThreadId	= NULL;
	}

	return 0;
}

void CAlbumList::QuickScan()
{
	if (m_hThread)
	{
		StopScan();
	}

	m_hThread = CreateThread(NULL, 0, QuickScanThreadProc, this, 0, &m_dwThreadId);
	SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
}

DWORD WINAPI CAlbumList::QuickScanThreadProc(LPVOID lpData)
{
	SetThreadName(-1, "QuickScanThreadProc");

	CAlbumList *pAL = (CAlbumList *)lpData;
	if (pAL)
	{
		// remove invalid/outdated stuff first
		int size = pAL->GetSize();
		for (int i=size-1; i>=0; --i)
		{
			if (pAL->m_bStopScanning) break;

			CAlbum *pAlbum = (CAlbum *)pAL->GetAlbum(i);

			// album not in search path anymore
			if (!pAL->AlbumPartOfSearchPath(pAlbum->GetPath()) ||
			// ignore playlists
				(pAL->m_bIgnorePL && pAlbum->IsPlaylistBased()) ||
			// non playlist in 'playlist only'
				(pAL->m_bPLOnly && !pAlbum->IsPlaylistBased()) ||
			// multiple disc cd
				(pAL->m_bMultiDiscFix && pAlbum->IsPartOfMultiDisc())
				)
			{
				DbgPrint("CAlbumList::QuickScanThreadProc - Remove... %S\n", pAlbum->GetPath());

				pAL->m_nTotalTime -= pAlbum->GetTotalTime();

				// delete the album
				pAL->RemoveAt(i);
				pAlbum->Release();

				// callback
				if (pAL->m_pCallback && !pAL->m_bStopScanning)
				{
					pAL->m_pCallback(notifyListSizeChanged, 0, 0, pAL->m_pCB_data);
				}
			}
			// album is outdated
			else
			{
				switch (pAlbum->IsOutdated())
				{
				case IOD_UPTODATE:
					DbgPrint("CAlbumList::QuickScanThreadProc - Up to date... %S\n", pAlbum->GetPath());
					break;

				case IOD_REMOVE:
					DbgPrint("CAlbumList::QuickScanThreadProc - Remove... %S\n", pAlbum->GetPath());
					pAL->m_nTotalTime -= pAlbum->GetTotalTime();

					// remove the album
					pAL->RemoveAt(i);
					pAlbum->Release();

					// callback
					if (pAL->m_pCallback && !pAL->m_bStopScanning)
					{
						pAL->m_pCallback(notifyListSizeChanged, 0, 0, pAL->m_pCB_data);
					}
					break;

				case IOD_REFRESH:
					DbgPrint("CAlbumList::QuickScanThreadProc - Refresh... %S\n", pAlbum->GetPath());
					pAlbum->Refresh();
					break;
				}
			}
			pAlbum->Release();
		}

		// scan for new stuff
		pAL->ScanForNewAlbum();

		// close handle and stuff
		CloseHandle(pAL->m_hThread);
		pAL->m_hThread		= NULL;
		pAL->m_dwThreadId	= NULL;

		// callback
		if (pAL->m_pCallback && !pAL->m_bStopScanning)
		{
			pAL->m_pCallback(notifyFinishedQuickScan, 0, 0, pAL->m_pCB_data);
		}
	}

	return 0;
}

void CAlbumList::ScanPath(LPCWSTR szPath)
{
	if (m_hThread == NULL)
	{
		//TODO: setting this isn't really thread-safe 
		wcscpy(m_szScanPath, szPath);

		m_hThread = CreateThread(NULL, 0, ScanPathThreadProc, this, 0, &m_dwThreadId);
		SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
	}
}

DWORD WINAPI CAlbumList::ScanPathThreadProc(LPVOID lpData)
{
	SetThreadName(-1, "ScanPathThreadProc");

	CAlbumList *pAL = (CAlbumList *)lpData;
	if (pAL)
	{
		// scan for new stuff
		pAL->ProcessPath(pAL->m_szScanPath, TRUE);

		// close handle and stuff
		CloseHandle(pAL->m_hThread);
		pAL->m_hThread		= NULL;
		pAL->m_dwThreadId	= NULL;
	}

	return 0;
}

void CAlbumList::CleanupPath(LPCWSTR szPath)
{
	if (m_hThread == NULL)
	{
		//TODO: setting this isn't really thread-safe 
		wcscpy(m_szScanPath, szPath);

		m_hThread = CreateThread(NULL, 0, CleanupPathThreadProc, this, 0, &m_dwThreadId);
		SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
	}
}

DWORD WINAPI CAlbumList::CleanupPathThreadProc(LPVOID lpData)
{
	SetThreadName(-1, "CleanupPathThreadProc");

	CAlbumList *pAL = (CAlbumList *)lpData;
	if (pAL)
	{
		// remove invalid/outdated stuff first
		int size = pAL->GetSize();
		for (int i=size-1; i>=0; --i)
		{
			if (pAL->m_bStopScanning) break;

			CAlbum *pAlbum = (CAlbum *)pAL->GetAlbum(i);

			// album in remove search path
			if (wcsstr(pAlbum->GetPath(), pAL->m_szScanPath) == pAlbum->GetPath())
			{
				DbgPrint("CAlbumList::CleanupPathThreadProc - Removing... %S\n", pAlbum->GetPath());

				pAL->m_nTotalTime -= pAlbum->GetTotalTime();

				// remove the album
				pAL->RemoveAt(i);
				pAlbum->Release();

				// callback
				if (pAL->m_pCallback)
				{
					pAL->m_pCallback(notifyListSizeChanged, 0, 0, pAL->m_pCB_data);
				}
			}

			pAlbum->Release();
		}

		// close handle and stuff
		CloseHandle(pAL->m_hThread);
		pAL->m_hThread		= NULL;
		pAL->m_dwThreadId	= NULL;
	}

	return 0;
}

int CAlbumList::GetTotalTime()
{
	return m_nTotalTime;
}

void CAlbumList::RemoveAll()
{
	m_nTotalTime = 0;

	CPtrArray::RemoveAll();
}

BOOL CAlbumList::ScanForNewAlbum()
{
	int size = m_SearchPaths.GetSize();
	for (int i=0; i<size; ++i)
	{
		if (m_bStopScanning) break;

		ProcessPath(m_SearchPaths[i], TRUE);
	}

	return TRUE;
}

BOOL CAlbumList::AlbumPartOfSearchPath(LPCWSTR path)
{
	int size = m_SearchPaths.GetSize();
	for (int i=0; i<size; ++i)
	{
		if (wcsstr(path, m_SearchPaths[i]) == path)
			return TRUE;
	}

	return FALSE;
}

void CAlbumList::Sort(SortOrder order1, SortOrder order2)
{
	m_SortOrders.RemoveAll();
	m_SortOrders.InsertAt(0, order2);
	m_SortOrders.InsertAt(0, order1);

	Resort();
}

void CAlbumList::Sort(SortOrder order)
{
	DWORD newOrder = order % 100;

	// remove it from the history if it exists (and not the last one)
	int size = m_SortOrders.GetSize();
	for (int i=1; i<size; ++i)
	{
		if ((m_SortOrders[i] % 100) == newOrder)
		{
			m_SortOrders.RemoveAt(i);
			size -= 1;
			break;
		}
	}

	// reverse sort order?
	if (size && (newOrder == (m_SortOrders[0] % 100)))
	{
		if (m_SortOrders[0] >= orderPathReverse)
			m_SortOrders.SetAt(0, m_SortOrders[0] - 100);
		else
			m_SortOrders.SetAt(0, m_SortOrders[0] + 100);
	}
	else
	{
		m_SortOrders.InsertAt(0, order);
	}

	Resort();
}

void CAlbumList::Resort()
{
	int size = m_SortOrders.GetSize();
	if (size == 0) return;

#ifdef _DEBUG
	DbgPrint("Sort: ");
	for (int i=0; i<size; ++i)
	{
		switch (m_SortOrders[i])
		{
		case orderPath:					DbgPrint("orderPath ");				break;
		case orderName:					DbgPrint("orderName ");				break;
		case orderArtist:				DbgPrint("orderArtist ");			break;
		case orderAlbum:				DbgPrint("orderAlbum ");			break;
		case orderGenre:				DbgPrint("orderGenre");				break;
		case orderYear:					DbgPrint("orderYear ");				break;
		case orderNumOfSongs:			DbgPrint("orderNumOfSongs ");		break;
		case orderTotalTime:			DbgPrint("orderTotalTime ");		break;
		case orderPathReverse:			DbgPrint("orderPathReverse ");		break;
		case orderNameReverse:			DbgPrint("orderNameReverse ");		break;
		case orderArtistReverse:		DbgPrint("orderArtistReverse ");	break;
		case orderAlbumReverse:			DbgPrint("orderAlbumReverse ");		break;
		case orderGenreReverse:			DbgPrint("orderGenreReverse ");		break;
		case orderYearReverse:			DbgPrint("orderYearReverse ");		break;
		case orderNumOfSongsReverse:	DbgPrint("orderNumOfSongsReverse ");break;
		case orderTotalTimeReverse:		DbgPrint("orderTotalTimeReverse ");	break;
		}
	}
	DbgPrint("\n");
#endif

	CPtrArray::Sort();
	
	// callback
	if (m_pCallback)
	{
		m_pCallback(notifyContentChanged, 0, 0, m_pCB_data);
	}
}

void CAlbumList::Shuffle()
{
	srand(GetTickCount());

	// we will swap 2 * "number of albums" times to ensure a good randomness
	int size = GetSize();
	for (int x=0; x<2*size; ++x)
	{
		// Generate 2 random numbers, each between 0 and size-1
		int from	= rand() % size;
		int to		= rand() % size;

		// Swap them
		Swap(to, from);
	}

	// callback
	if (m_pCallback)
	{
		m_pCallback(notifyContentChanged, 0, 0, m_pCB_data);
	}

	// after shuffling, it is not sorted anymore
	m_SortOrders.RemoveAll();
}

SortOrder CAlbumList::GetSort()
{
	return m_SortOrders.GetSize() ? (SortOrder)m_SortOrders[0] : orderOff;
}

BOOL CAlbumList::PlayAlbumByArtist(LPCWSTR artist, BOOL bEnqueue /*=FALSE*/)
{
	bool played = false || bEnqueue;
	int size = GetSize();
	for (int i=0; i<size; ++i)
	{
		CAlbum *pAlbum = (CAlbum *)GetAlbum(i);
		if (pAlbum)
		{
			if (wcsicmp(pAlbum->GetArtist(), artist) == 0)
			{
				if (!played) pAlbum->Play();
				else		 pAlbum->Enqueue();

				played = true;
			}
			pAlbum->Release();
		}
	}

	return TRUE;
}

CAlbum *CAlbumList::GetAlbum(int index)
{
	CCriticalSection cs(&m_csMutex);

	CAlbum *pAlbum = (CAlbum *)GetAt(index);
	if (pAlbum) pAlbum->AddRef();

	return pAlbum;
}

int CAlbumList::GetArtistAlbumCount(LPCWSTR artist)
{
	int count = 0;
	int size = GetSize();
	for (int i=0; i<size; ++i)
	{
		CAlbum *pAlbum = (CAlbum *)GetAlbum(i);
		if (pAlbum)
		{
			if (wcsicmp(pAlbum->GetArtist(), artist) == 0)
			{
				count++;
			}
			pAlbum->Release();
		}
	}

	return count;
}

BOOL CAlbumList::PlayRandomAlbumByArtist(LPCWSTR artist, BOOL bEnqueue /*=FALSE*/)
{
	CAlbumList albums;

	// find all the albums by this artist
	int size = GetSize();
	for (int i=0; i<size; ++i)
	{
		CAlbum *pAlbum = (CAlbum *)GetAlbum(i);
		if (pAlbum)
		{
			if (wcsicmp(pAlbum->GetArtist(), artist) == 0)
			{
				albums.Add(pAlbum);
			}
			pAlbum->Release();
		}
	}

	albums.SetCurAlbum(GetCurAlbum());

	return albums.PlayRandomAlbum(bEnqueue);
}

BOOL CAlbumList::PlayRandomAlbum(BOOL bEnqueue /*=FALSE*/)
{
	if (GetSize() == 0) return FALSE;
	if (GetSize() == 1)
	{
		BOOL bRet = FALSE;
		CAlbum *pAlbum = (CAlbum *)GetAlbum(0);
		if (pAlbum)
		{
			bRet = bEnqueue? pAlbum->Enqueue() : pAlbum->Play();
			pAlbum->Release();
		}
		return bRet;
	}

	// reset history if we had played everything already
	if (m_PlayHistory.GetCount() >= GetSize())
	{
		m_PlayHistory.RemoveAll();
	}

	// only keep so many history (performance issue)
	if (m_nRandomDup && (m_nRandomDup < m_PlayHistory.GetCount()))
	{
		m_PlayHistory.RemoveAll();
	}

	if (m_PlayHistory.GetCount() == 0)
	{
		int index = GetCurAlbumIndex();
		CAlbum *pAlbum = (CAlbum *)GetAlbum(index);
		if (pAlbum)
		{
			char string[64];
			itoa((int)pAlbum, string, 16);
			m_PlayHistory.SetAt(string, "0");
			pAlbum->Release();
		}
	}

	// randomize again
	srand((unsigned)GetTickCount());

	// try as many times as we need
	CAlbum *pAlbum = NULL;
	while (pAlbum = (CAlbum *)GetAlbum(rand() % GetSize()))
	{
		char string[64], *str;
		itoa((int)pAlbum, string, 16);
		if (!m_PlayHistory.Lookup(string, str))
		{
			BOOL bRet = bEnqueue? pAlbum->Enqueue() : pAlbum->Play();
			pAlbum->Release();
			return bRet;
		}
		pAlbum->Release();
	}

	return FALSE;
}

BOOL CAlbumList::Load(LPCTSTR pathname)
{
	HANDLE		fp			= NULL;
	LPALBUMINFO pAlbumList	= NULL;

	ENSURE
	{
		if (INVALID_HANDLE_VALUE == (fp = CreateFile(pathname, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL))) FAIL;

		DWORD nBytesRead;
		int nSize = 0;
		DWORD dwValue;

		char signature[7] = "\0\0\0\0\0\0";
		// read signature
		if (!ReadFile(fp, signature, 6, &nBytesRead, NULL)) FAIL;
		if (6 != nBytesRead) FAIL;

		// check signature
		if (lstrcmp(signature, "GENM3A") != 0) FAIL;

		// read size of album list info structure (acts as version number)
		if (!ReadFile(fp, &dwValue, sizeof(DWORD), &nBytesRead, NULL)) FAIL;
		if (sizeof(DWORD) != nBytesRead) FAIL;

		// structure size check OK?
		if (dwValue != ALBUMINFO_VER) FAIL;

		// read size of album list successful
		if (!ReadFile(fp, &dwValue, sizeof(DWORD), &nBytesRead, NULL)) FAIL;
		if (sizeof(DWORD) != nBytesRead) FAIL;
		if (dwValue == 0) break;
		nSize = dwValue;

		// read compression method
		BYTE bCompression = 0;
		if (!ReadFile(fp, &bCompression, 1, &nBytesRead, NULL)) FAIL;

		// read the albumlist
		int i=0;
		switch (bCompression)
		{
		case AL_UNCOMPRESS:
		case AL_PER_ALBUM_RLE:
			for (i=0; i<nSize; ++i)
			{
				CAlbum *pAlbum = new CAlbum(this);
				if (pAlbum->Load(fp, bCompression))
				{
					Add(pAlbum);

					m_nTotalTime += pAlbum->GetTotalTime();
				}
				else
				{
					delete pAlbum;
				}
			}
			break;

		case AL_RLE:	// not supported anymore
		default:
			/*do nothing*/;
		}

		CloseHandle(fp);

		// callback
		if (m_pCallback)
		{
			m_pCallback(notifyListLoaded, 0, 0, m_pCB_data);
		}

		return GetSize();
	}
	END_ENSURE;

	if (fp) CloseHandle(fp);

	return FALSE;
}

BOOL CAlbumList::Save(LPCTSTR pathname)
{
	int nSize = GetSize();

	HANDLE fp;
	if (INVALID_HANDLE_VALUE != (fp = CreateFile(pathname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)))
	{
		DWORD nBytesWritten;
		DWORD dwValue;

		// write signature
		WriteFile(fp, "GENM3A", 6, &nBytesWritten, NULL);

		// write structure size (acts as version number)
		dwValue = ALBUMINFO_VER;
		WriteFile(fp, &dwValue, sizeof(DWORD), &nBytesWritten, NULL);

		// write the number of entries
		dwValue = nSize;
		WriteFile(fp, &dwValue, sizeof(DWORD), &nBytesWritten, NULL);

		// we have albums
		if (nSize)
		{
			int comp = wndWinampAL.GetSetting(settingCompression);
			BYTE bComp = comp ? AL_PER_ALBUM_RLE : AL_UNCOMPRESS;

			// write compression
			WriteFile(fp, &bComp, sizeof(BYTE), &nBytesWritten, NULL);

			// write the albums
			for (int i=0; i<nSize; i++)
			{
				CAlbum *pAlbum = (CAlbum *)GetAlbum(i);
				if (pAlbum)
				{
					pAlbum->Save(fp, bComp);
					pAlbum->Release();
				}
			}
		}

		FlushFileBuffers(fp);
		CloseHandle(fp);

		return TRUE;
	}
	return FALSE;
}

void CAlbumList::AddPath(LPCWSTR path)
{
	wchar_t longpath[260], *filename;
	GetFullPathNameW(path, MAX_PATH, longpath, &filename);

	m_SearchPaths.Add(longpath);
}

void CAlbumList::AddCDROMs()
{
	// go through all drives
	wchar_t szDrive[8] = L"C:\\";
	for (int i=0; i<26; i++)
	{
		if ((m_dwCDROMs >> i) & 1)
		{
			int curdrive = _getdrive();
			if (_chdrive(i+1) == 0)
			{
				szDrive[0] = L'A' + i;
				AddPath(szDrive);
			}
			_chdrive(curdrive);
		}
	}
}

void CAlbumList::RemovePath(LPCWSTR path)
{
	wchar_t longpath[260], *filename;
	GetFullPathNameW(path, MAX_PATH, longpath, &filename);

	int size = m_SearchPaths.GetSize();
	for (int i=0; i<size; ++i)
	{
		if (wcsicmp(longpath, m_SearchPaths[i]) == 0)
		{
			m_SearchPaths.RemoveAt(i);
			break;
		}
	}
}

BOOL CAlbumList::ProcessM3U(LPCWSTR path)
{
	BOOL bAdded = FALSE;

	// find the first playlist file (*.m3u) in this directory if it exists
	wchar_t m3upath[256];
	wcscpy(m3upath, path);
	PathAppendW(m3upath, L"*.m3u");

	WIN32_FIND_DATAW FindFileData;
	HANDLE hFile;
	if ((hFile = FindFirstFileW(m3upath, &FindFileData)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (m_bStopScanning) break;

			if (wcsicmp(PathFindExtensionW(FindFileData.cFileName), L".m3u") == 0)
			{
				if (M3UExists(path, FindFileData.cFileName))
					bAdded = TRUE;
				else
				{
					CAlbum album(this);
					if (album.AddM3U(path, m_nPathLen, FindFileData.cFileName))
					{
						AddAlbum(&album);
						bAdded = TRUE;
					}
				}
			}
		}
		while (FindNextFileW(hFile, &FindFileData) != 0);

		FindClose(hFile);
	}

	return bAdded;
}

BOOL CAlbumList::CheckSubDirectories(LPCWSTR path, WIN32_FIND_DATAW *file)
{
	if (file == NULL) return FALSE;

	if (file->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (wcscmp(file->cFileName, L".") && 
			wcscmp(file->cFileName, L".."))
		{
			wchar_t newpath[256];
			wcscpy(newpath, path);
			PathAppendW(newpath, file->cFileName);

			if ((file->cFileName[0] == L'~') ||
				(file->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
				(file->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
			{
				DbgPrint("CAlbumList::CheckSubDirectories - Skipping Path: \"%S\"\n", newpath);
			}
			else
			{
				return ProcessPath(newpath, FALSE);
			}
		}
	}
	else if (wcsicmp(PathFindExtensionW(file->cAlternateFileName), L".lnk") == 0)
	{
/*		char newpath[256];
		lstrcpy(newpath, path);
		PathAppend(newpath, file->cFileName);

		if (ResolveIt(m_hWnd, newpath, newpath))
		{
			DWORD dwAttrib = GetFileAttributes(newpath);
			if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
			{
				return AddAlbumsToList(newpath, FALSE);
			}
		}*/
	}

	return FALSE;
}

BOOL CAlbumList::ProcessSubDirectory(LPCWSTR path)
{
	wchar_t mypath[256];
	wcscpy(mypath, path);
	PathAppendW(mypath, L"*.*");

	WIN32_FIND_DATAW FindFileData;
	HANDLE hFile;

	BOOL bAdded = FALSE;
	if ((hFile = FindFirstFileW(mypath, &FindFileData)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (m_bStopScanning) break;

			if (CheckSubDirectories(path, &FindFileData))
				bAdded = TRUE;
		}
		while (FindNextFileW(hFile, &FindFileData) != 0);

		FindClose(hFile);
	}

	return bAdded;
}

BOOL CAlbumList::ProcessPath(LPCWSTR path, BOOL bSearchRoot)
{
	BOOL bFound = FALSE;
	BOOL bProcessSub = TRUE;

	// save and change directory
	CChangeDir changed_to_dir(path);

	if (bSearchRoot)
	{
		m_nPathLen = wcslen(path);
	}

	DbgPrint("CAlbumList::ProcessPath - Processing Path: \"%S\"%s\n", path, bSearchRoot ? " [ROOT]" : "");

	// process M3Us?
	if (!m_bIgnorePL && (!bSearchRoot || !m_bIgnoreRootPL))
	{
		if (ProcessM3U(path))
			bFound = TRUE;
	}

	BOOL bMultiDisc = FALSE;

	// path already part of the list?
	CAlbum *pAlbum = NULL;
	if (pAlbum = FindAlbumByPath(path))
	{
		bMultiDisc = pAlbum->IsMultiDisc();
		pAlbum->Release();

		DbgPrint("CAlbumList::ProcessPath - Album exists %s\n", bMultiDisc ? "(MultiDisc)" : "");
		bFound = TRUE;
	}

	// check for files in this directory
	if (!bFound && !m_bPLOnly)
	{
		CAlbum album(this);
		if (album.AddDir(path, m_nPathLen))
		{
			AddAlbum(&album);
			bFound = TRUE;

			bMultiDisc = album.IsMultiDisc();
		}
	}

	// check all sub-directories
	if (m_bScanSubDir && !bMultiDisc)
	{
		if (ProcessSubDirectory(path))
			bFound = TRUE;
	}

	return bFound;
}

BOOL CAlbumList::AddAlbum(CAlbum *pAlbumInfo)
{
	CAlbum *pAI = new CAlbum(this);
	*pAI = *pAlbumInfo;

	if (IsCDROM(pAI->GetPath())) pAI->dwFlags |= AI_CDROM;

	Add(pAI);

	m_nTotalTime += pAI->GetTotalTime();

	Resort();

	// callback
	if (m_pCallback)
	{
		m_pCallback(notifyListSizeChanged, 0, 0, m_pCB_data);
	}

	return TRUE;
}

BOOL CAlbumList::IsCDROM(LPCWSTR path)
{
	int nDrive = towupper(path[0]) - 'A';
	return ((m_dwCDROMs & (1 << nDrive)) != 0);
}

CAlbum *CAlbumList::FindAlbumByPath(LPCWSTR path)
{
	int size = GetSize();
	for (int i=0; i<size; i++)
	{
		CAlbum *pAlbum = (CAlbum *)GetAlbum(i);
		if (pAlbum)
		{
			if (wcsicmp(path, pAlbum->path) == 0)
				return pAlbum;

			pAlbum->Release();
		}
	}

	return NULL;
}

BOOL CAlbumList::M3UExists(LPCWSTR path, LPCWSTR filename)
{
	wchar_t path2[MAX_PATH];
	wcscpy(path2, path);
	PathAppendW(path2, filename);

	int size = GetSize();
	for (int i=0; i<size; ++i)
	{
		CAlbum *pAlbum = (CAlbum *)GetAlbum(i);
		if (pAlbum)
		{
			if (_wcsicmp(path2, pAlbum->m3u_file) == 0)
			{
				pAlbum->Release();
				DbgPrint("CAlbumList::M3UExists - Duplicate M3U: \"%S\"\n", path2);
				return TRUE;
			}
			pAlbum->Release();
		}
	}

	return FALSE;
}

void CAlbumList::SetCurAlbum(GUID& id)
{
	m_idCurAlbum = id;

	// update play history
	int index = GetCurAlbumIndex();
	CAlbum *pAlbum = (CAlbum *)GetAlbum(index);
	if (pAlbum)
	{
		char string[64];
		itoa((int)pAlbum, string, 16);
		m_PlayHistory.SetAt(string, "0");
		pAlbum->Release();
	}

	// callback
	if (m_pCallback)
	{
		m_pCallback(notifyCurAlbumChanged, index, 0, m_pCB_data);
	}
}

GUID CAlbumList::GetCurAlbum()
{
	return m_idCurAlbum;
}

int CAlbumList::GetCurAlbumIndex()
{
	int size = GetSize();
	for (int i=0; i<size; i++)
	{
		CAlbum *pAlbum = (CAlbum *)GetAlbum(i);
		if (pAlbum)
		{
			if (memcmp(&pAlbum->id, &m_idCurAlbum, sizeof(GUID)) == 0)
			{
				pAlbum->Release();
				return i;
			}

			pAlbum->Release();
		}
	}
	return -1;
}

void CAlbumList::GenerateHTMLList()
{
	char str[256];

	if (GetSize())
	{
		// create temperary playlist
		char dirTemp[MAX_PATH];
		if (GetTempPath(MAX_PATH, dirTemp) == 0)
			return;

		char html[MAX_PATH];
		GetTempFileName(dirTemp, "wal", 0, html);
		DeleteFile(html);
		PathRenameExtension(html, ".html");

		CTemplateFile templatefile;

		//---------------------------
		// output filename
		templatefile.AddReplace("outfilename", html);

		//---------------------------
		// character set (UTF8)
		templatefile.AddReplace("charset", "utf8");
		
		templatefile.AddReplace("totalalbums", GetSize());

		//---------------------------
		// album list
		int nCount = 1;
		int nTotalTime = 0;
		for (int i=0; i<GetSize(); i++)
		{
			CAlbum *pAlbum = (CAlbum *)GetAlbum(i);
			if (pAlbum)
			{
				int hour = pAlbum->nTotalTime/3600;
				int min = (pAlbum->nTotalTime%3600)/60;
				int sec = pAlbum->nTotalTime%60;

				if (hour)
					wsprintf(str, "%ld:%02ld:%02ld", hour, min, sec);
				else
					wsprintf(str, "%ld:%02ld", min, sec);

				templatefile.AddReplace("albumindex", i, i+1);
				templatefile.AddReplace("albumname", i, AutoChar(pAlbum->GetAlbum(), CP_UTF8));
				templatefile.AddReplace("albumcount", i, pAlbum->nCount);
				templatefile.AddReplace("displayname", i, AutoChar(pAlbum->GetTitle(), CP_UTF8));
				templatefile.AddReplace("albumartist", i, AutoChar(pAlbum->GetArtist(), CP_UTF8));
				templatefile.AddReplace("albumyear", i, pAlbum->nYear);
				templatefile.AddReplace("albumlengthhrs", i, hour);
				templatefile.AddReplace("albumlengthmin", i, min);
				templatefile.AddReplace("albumlengthsec", i, sec);
				templatefile.AddReplace("albumlength", i, str);

				LPCWSTR szCover = pAlbum->GetCoverFilename();
				LPWSTR ext = PathFindExtensionW(szCover);
				if ((wcsicmp(ext, L".mp3") != 0) &&
					(wcsicmp(ext, L".m4a") != 0))
				{
					templatefile.AddReplace("albumcover", i, AutoChar(szCover, CP_UTF8));
				}

				templatefile.AddReplace("songcounter", i, nCount);
				
				DbgPrint("\t%ld. %s\t(%ld)\n", i+1, pAlbum->name, nCount);

				nCount += pAlbum->nCount;
				nTotalTime += pAlbum->nTotalTime;

				pAlbum->Release();
			}
		}

		//---------------------------
		// total time
		if (nTotalTime > 3600)	// write hours
		{
			templatefile.AddReplace("totallengthhrs", nTotalTime/3600);
		}
		if (nTotalTime > 60)	// write minutes
		{
			templatefile.AddReplace("totallengthmin", (nTotalTime%3600)/60);
		}
		if (nTotalTime)			// write seconds
		{
			templatefile.AddReplace("totallengthsec", nTotalTime%60);
		}

		//---------------------------
		// write average time
		int nAvgLength = nTotalTime/GetSize();
		if (nAvgLength > 3600)
			wsprintf(str, "%ld:%02ld:%02ld", nAvgLength/3600, (nAvgLength%3600)/60, nAvgLength%60);
		else
			wsprintf(str, "%ld:%02ld", nAvgLength/60, nAvgLength%60);

		templatefile.AddReplace("averagelength", str);

		//---------------------------
		// convert the file
		char szTemplateFile[MAX_PATH];
		lstrcpy(szTemplateFile, dirPlugin);
		PathAppend(szTemplateFile, "Album List\\Templates\\export.htt");
		if (!templatefile.Convert(html, szTemplateFile)) return;

		//---------------------------
		// show the thing in the minibrowser
		if (wndWinamp.SendIPCMessage(IPC_GETWND_MB, IPC_ISWNDVISIBLE))
		{
			char url[MAX_PATH*2] = "file:///";
			lstrcat(url, html);
			wndWinamp.SendIPCMessage(0, IPC_MBOPEN);
			wndWinamp.SendIPCMessage((WPARAM)url, IPC_MBOPEN);
		}
		else
		{
			GotoURL(html, SW_SHOWNORMAL);
		}
	}
}

int CAlbumList::CompareAlbums(const void *elem1, const void *elem2)
{
	CAlbum *pAlbum1 = *(CAlbum **)elem1;
	CAlbum *pAlbum2 = *(CAlbum **)elem2;

	pAlbum1->AddRef();
	pAlbum2->AddRef();

	int nRet = pAlbum1->m_pObj->Compare(pAlbum1, pAlbum2, 0);

	pAlbum1->Release();
	pAlbum2->Release();

	return nRet;
}

int CAlbumList::Compare(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel)
{
	if (nLevel >= m_SortOrders.GetSize())
	{
		int nRet = 0;
		if ((nRet = CompareStr(pAlbum1->path, pAlbum2->path)) == 0)
		{
			nRet = CompareStr(pAlbum1->m3u_file, pAlbum2->m3u_file);
		}
		return nRet;
	}

	switch (m_SortOrders[nLevel])
	{
	case orderPath:					return ComparePath(pAlbum1, pAlbum2, nLevel);
	case orderName:					return CompareName(pAlbum1, pAlbum2, nLevel);
	case orderArtist:				return CompareArtist(pAlbum1, pAlbum2, nLevel);
	case orderAlbum:				return CompareAlbum(pAlbum1, pAlbum2, nLevel);
	case orderGenre:				return CompareGenre(pAlbum1, pAlbum2, nLevel);
	case orderYear:					return CompareYear(pAlbum1, pAlbum2, nLevel);
	case orderNumOfSongs:			return CompareNumOfSongs(pAlbum1, pAlbum2, nLevel);
	case orderTotalTime:			return CompareTotalTime(pAlbum1, pAlbum2, nLevel);

	case orderPathReverse:			return ComparePath(pAlbum1, pAlbum2, nLevel, TRUE);
	case orderNameReverse:			return CompareName(pAlbum1, pAlbum2, nLevel, TRUE);
	case orderArtistReverse:		return CompareArtist(pAlbum1, pAlbum2, nLevel, TRUE);
	case orderAlbumReverse:			return CompareAlbum(pAlbum1, pAlbum2, nLevel, TRUE);
	case orderGenreReverse:			return CompareGenre(pAlbum1, pAlbum2, nLevel, TRUE);
	case orderYearReverse:			return CompareYear(pAlbum1, pAlbum2, nLevel, TRUE);
	case orderNumOfSongsReverse:	return CompareNumOfSongs(pAlbum1, pAlbum2, nLevel, TRUE);
	case orderTotalTimeReverse:		return CompareTotalTime(pAlbum1, pAlbum2, nLevel, TRUE);
	}

	return 0;
}

int CAlbumList::ComparePath(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse)
{
	int nRet = 0;
	if ((nRet = CompareStr(pAlbum1->path, pAlbum2->path)) == 0)
	{
		nRet = CompareStr(pAlbum1->m3u_file, pAlbum2->m3u_file);
	}

	if (nRet == 0) return Compare(pAlbum1, pAlbum2, ++nLevel);

	return bReverse ? -nRet : nRet;
}

int CAlbumList::CompareName(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse)
{
	LPCWSTR name1 = pAlbum1->name;
	LPCWSTR name2 = pAlbum2->name;

	if (pAlbum1->m_pObj->m_bIgnoreTHE)
	{
		if (_wcsnicmp(name1, L"the ", 4) == 0)
			name1 += 4;

		if (_wcsnicmp(name2, L"the ", 4) == 0)
			name2 += 4;
	}

	int nRet = CompareStr(name1, name2);

	if (nRet == 0) return Compare(pAlbum1, pAlbum2, ++nLevel);

	return bReverse ? -nRet : nRet;
}

int CAlbumList::CompareArtist(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse)
{
	LPCWSTR artist1 = pAlbum1->artist;
	LPCWSTR artist2 = pAlbum2->artist;

	if (pAlbum1->m_pObj->m_bIgnoreTHE)
	{
		if (_wcsnicmp(artist1, L"the ", 4) == 0)
			artist1 += 4;

		if (_wcsnicmp(artist2, L"the ", 4) == 0)
			artist2 += 4;
	}

	int nRet = CompareStr(artist1, artist2);

	if (nRet == 0) return Compare(pAlbum1, pAlbum2, ++nLevel);

	return bReverse ? -nRet : nRet;
}

int CAlbumList::CompareAlbum(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse)
{
	LPCWSTR album1 = pAlbum1->album;
	LPCWSTR album2 = pAlbum2->album;

	if (pAlbum1->m_pObj->m_bIgnoreTHE)
	{
		if (_wcsnicmp(album1, L"the ", 4) == 0)
			album1 += 4;

		if (_wcsnicmp(album2, L"the ", 4) == 0)
			album2 += 4;
	}

	int nRet = CompareStr(album1, album2);

	if (nRet == 0) return Compare(pAlbum1, pAlbum2, ++nLevel);

	return bReverse ? -nRet : nRet;
}


int CAlbumList::CompareGenre(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse)
{
	LPCWSTR genre1 = pAlbum1->genre;
	LPCWSTR genre2 = pAlbum2->genre;

	int nRet = CompareStr(genre1, genre2);

	if (nRet == 0) return Compare(pAlbum1, pAlbum2, ++nLevel);

	return bReverse ? -nRet : nRet;
}

int CAlbumList::CompareYear(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse)
{
	int nRet = (pAlbum1->nYear - pAlbum2->nYear);

	if (nRet == 0) return Compare(pAlbum1, pAlbum2, ++nLevel);

	return bReverse ? -nRet : nRet;
}

int CAlbumList::CompareNumOfSongs(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse)
{
	int nRet = (pAlbum1->nCount - pAlbum2->nCount);

	if (nRet == 0) return Compare(pAlbum1, pAlbum2, ++nLevel);

	return bReverse ? -nRet : nRet;
}

int CAlbumList::CompareTotalTime(const CAlbum *pAlbum1, const CAlbum *pAlbum2, int nLevel, BOOL bReverse)
{
	int nRet = (pAlbum1->nTotalTime - pAlbum2->nTotalTime);

	if (nRet == 0) return Compare(pAlbum1, pAlbum2, ++nLevel);

	return bReverse ? -nRet : nRet;
}

void CAlbumList::RequestNotification(NotifyProc proc, LPVOID pData)
{
	m_pCallback	= proc;
	m_pCB_data	= pData;
}

void CAlbumList::ResetCovers()
{
	int size = GetSize();
	for (int i=0; i<size; i++)
	{
		CAlbum *pAlbum = (CAlbum *)GetAlbum(i);
		if (pAlbum)
		{
			pAlbum->SetIconIndex(-1);
			pAlbum->Release();
		}
	}
}

void CAlbumList::NotifyAlbumChanged()
{
	if (m_pCallback && !m_bStopScanning)
	{
		m_pCallback(notifyAlbumChanged, 0, 0, m_pCB_data);
	}
}

void CAlbumList::NotifyCoverChanged()
{
	if (m_pCallback && !m_bStopScanning)
	{
		m_pCallback(notifyCoverChanged, 0, 0, m_pCB_data);
	}
}
