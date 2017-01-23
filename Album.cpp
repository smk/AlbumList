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
#include <Shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <search.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mbstring.h>
#include <ole2.h>
#include <olectl.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "Util.h"

#include "AlbumList.h"
#include "Album.h"
#include "FileInfo.h"

#include "AlbumInfo.h"

#include "AutoChar.h"

#include "RLE.h"

//////////////////////////////////////////////////////////////////
// CAlbum class

CAlbum::CAlbum(CAlbumList *pObj)
{
	memset(name, 0, MAX_PATH);		// display name (combination of artist and album)
	memset(album, 0, MAX_PATH);		// album name
	memset(artist, 0, MAX_PATH);	// artist name
	memset(path, 0, MAX_PATH);		// directory path
	memset(m3u_file, 0, MAX_PATH);	// pathname of playlist (if exist)
	memset(genre, 0, MAX_PATH);		// genre

	nTotalTime	= 0;				// total time in the directory (or playlist)
	nCount		= 0;				// number of files in this directory (or playlist)
	nYear		= 0;				// year of this album
	dwFlags		= 0;				// flags
	CoCreateGuid(&id);				// default ID for this album
	nIconIndex	= -1;

	dwReserved1	= 0;
	dwReserved2	= 0;

	m_pObj		= pObj;

	m_nRef		= 1;
}

CAlbum::~CAlbum()
{
}

int CAlbum::AddRef()
{
	return ++m_nRef;
}

int CAlbum::Release()
{
	if (--m_nRef == 0)
	{
		delete this;
		return 0;
	}

	return m_nRef;
}

BOOL CAlbum::Load(HANDLE fp, BYTE bComp)
{
	DWORD nBytesRead;

	switch (bComp)
	{
	case AL_UNCOMPRESS:
		return ReadFile(fp, (LPBYTE)name, sizeof(ALBUMINFO), &nBytesRead, NULL);

	case AL_PER_ALBUM_RLE:
		return RLE_Decoding(fp, (LPBYTE)name, sizeof(ALBUMINFO));
	}

	return FALSE;
}

BOOL CAlbum::Save(HANDLE fp, BYTE bComp)
{
	DWORD nBytesWritten;

	switch (bComp)
	{
	case AL_UNCOMPRESS:
		WriteFile(fp, (LPBYTE)name, sizeof(ALBUMINFO), &nBytesWritten, NULL);
		return TRUE;

	case AL_PER_ALBUM_RLE:
		RLE_Encoding(fp, (LPBYTE)name, sizeof(ALBUMINFO));
		return TRUE;
	}

	return FALSE;
}

BOOL CAlbum::AddDir(LPCWSTR dir, int nLen)
{
	if (nLen) nPathLen = nLen;

	BOOL bTrackAdded = FALSE;

	wchar_t mypath[256];

	// reset artist list
	m_VariousArtist.RemoveAll();
	m_MiscAlbum.RemoveAll();
	m_MixedGenre.RemoveAll();

	CStringArrayW dirs;
	if (m_pObj->m_bMultiDiscFix)
	{
		if (GetDiscDirs(dir, dirs))
		{
			dwFlags |= AI_MULTIDISC;
		}
	}
	else
	{
		dirs.Add(L".");
	}

	for (int d=0; d<dirs.GetSize(); d++)
	{
		wchar_t mydir[MAX_PATH];
		wcscpy(mydir, dir);
		PathAppendW(mydir, dirs[d]);

		// for all the extensions we have
		CTokensW extensions(m_pObj->m_szExtList, L" ?,\"'~!.-:;\t\n");
		int size = extensions.GetSize();
		for (int i=0; i<size; i++)
		{
			if (m_pObj->IsStoppingScan())
			{
				bTrackAdded = FALSE;
				break;
			}

			// find all the *.ext in this directory
			wcscpy(mypath, mydir);
			PathAppendW(mypath, L"*.");
			wcscat(mypath, extensions[i]);

			WIN32_FIND_DATAW FindFileData;
			HANDLE hFile;
			if ((hFile = FindFirstFileW(mypath, &FindFileData)) != INVALID_HANDLE_VALUE)
			{
				do
				{
					if (m_pObj->IsStoppingScan())
					{
						bTrackAdded = FALSE;
						break;
					}

					if (_wcsicmp(PathFindExtensionW(FindFileData.cFileName)+1, extensions[i]) == 0)
					{
						// get file length (in seconds)
						if (AddTrack(mydir, FindFileData.cFileName))
							bTrackAdded = TRUE;
					}
				}
				while (FindNextFileW(hFile, &FindFileData) != 0);

				FindClose(hFile);
			}
		}
	}

	if (bTrackAdded)
	{
		GenerateArtistName();
		GenerateAlbumName();
		GenerateGenre();

		// get year from dir if file is not in id3tag
		if (nYear == 0)
		{
			WIN32_FILE_ATTRIBUTE_DATA fileData;
			if (Win98GetFileAttributesEx(dir, GetFileExInfoStandard, &fileData))
			{
				SYSTEMTIME st;
				if (FileTimeToSystemTime(&fileData.ftCreationTime, &st))
				{
					nYear = st.wYear;
				}
			}
			else
			{
				struct _stat buf;
				if (_wstat(m3u_file, &buf) == 0)
				{
					struct tm *t = NULL;
					if (t = localtime(&buf.st_ctime))
					{
						nYear = t->tm_year;
					}
				}
			}
		}

		wcsncpy(path, dir, MAX_PATH);

		GenerateDisplayName(dir);
	}
	m_VariousArtist.RemoveAll();
	m_MiscAlbum.RemoveAll();
	m_MixedGenre.RemoveAll();

	return bTrackAdded;
}

BOOL CAlbum::AddM3U(LPCWSTR path1, int nLen, LPCWSTR filename /*=NULL*/)
{
	if (nLen) nPathLen = nLen;

	wchar_t pathname[MAX_PATH] = L"";

	// create pathname using the path and filename
	wcscat(pathname, path1);
	if (filename != NULL)
		PathAppendW(pathname, filename);

	// reset artist list
	m_VariousArtist.RemoveAll();
	m_MiscAlbum.RemoveAll();
	m_MixedGenre.RemoveAll();

	CStringArrayW dirs;
	if (m_pObj->m_bMultiDiscFix)
	{
		if (GetDiscDirs(path1, dirs))
		{
			dwFlags |= AI_MULTIDISC;
		}
	}

	FILE *f = _wfopen(pathname, L"rt");
	if (f != NULL)
	{
		int codepage = wndWinampAL.GetEncodingCodepage();
		char buf[512];
		wchar_t line[512];

		if ((fread(buf, 1, 3, f) == 3) &&
			(memcmp(buf, "﻿", 3) == 0))
		{
			codepage = CP_UTF8;
		}
		else
		{
			fseek(f, 0, SEEK_SET);
		}

		BOOL bExtendedPlaylist = FALSE;
		BOOL bTrackHasTimeInfo = FALSE;

		while (fgets(buf, 512, f))
		{
			if (m_pObj->IsStoppingScan())
			{
				nCount = 0;
				break;
			}

			wcscpy(line, AutoWide(buf, codepage));

			int len = wcslen(line);
			if (line[len-1] == 0xa) line[len-1] = 0;

			// extended playlist header
			// #EXTM3U
			if (0 == wcsicmp(line, L"#EXTM3U"))
			{
				bExtendedPlaylist = TRUE;
			}
			// artist name (custom AL tag)
			// #EXTART:Vivian Lai
			else if (0 == _wcsnicmp(line, L"#EXTART:", 8))
			{
				wcsncpy(artist, line+8, MAX_PATH);
			}
			// album name (custom AL tag)
			// #EXTALB:On the Sunshine Road
			else if (0 == _wcsnicmp(line, L"#EXTALB:", 8))
			{
				wcsncpy(album, line+8, MAX_PATH);
			}
			// genre (custom AL tag)
			// #EXTGENRE:Pop
			else if (0 == _wcsnicmp(line, L"#EXTGENRE:", 10))
			{
				wcsncpy(genre, line+10, MAX_PATH);
			}
			// file descriptions
			// #EXTINF:225,Vivian Lai - Super Kid
			// for shoutcast streams
			// #EXTINF:-1,Monkey Radio: Grooving sexy beats.
			else if (0 == _wcsnicmp(line, L"#EXTINF:", 8))
			{
				wchar_t *p = wcschr(line+8, L',');
				if (p)
				{
					wchar_t szTime[8] = L"";
					wcsncpy(szTime, line+8, p-(line+8)+1);
					if (_wtoi(szTime) > 0)
					{
						nTotalTime += _wtoi(szTime);
						bTrackHasTimeInfo = TRUE;
					}
				}
			}
			// skip other "comments" in the playlist
			else if (line[0] == '#')
			{
			}
			// filenames
			// c:\mp3\vivianlai\vivianlai01.mp3
			// for shoutcast streams
			// http://216.32.166.79:11280
			else
			{
				// make sure this is not a shoutcast stream
				if (_wcsnicmp(line, L"http://", 7) != 0)
				{
					// we need to parse the MP3 file if
					// there is no time info, or
					// the album/artist is not filled
					if ((bTrackHasTimeInfo == FALSE) ||
						(wcslen(album) == 0) ||
						(wcslen(artist) == 0) ||
						(wcslen(genre) == 0) ||
						(nYear == 0))
					{
						wchar_t filename[512];
						if (PathIsRelativeW(line))
						{
							wcscpy(filename, pathname);
							PathRemoveFileSpecW(filename);
							PathAppendW(filename, line);
						}
						else
						{
							wcscpy(filename, line);
						}

						CFileInfo *pFileInfo = CreateFileInfo(filename);
						if (pFileInfo)
						{
							if (pFileInfo->loadInfo(filename) == 0)
							{
								if (!bTrackHasTimeInfo)
								{
									nTotalTime += pFileInfo->getLengthInSeconds();
								}
								if (wcslen(album) == 0)
								{
									wchar_t temp_album[MAX_PATH] = L"";
									pFileInfo->getAlbum(temp_album);
									// increase count
									wchar_t *count = NULL;
									if (m_MiscAlbum.Lookup(temp_album, count))
									{
										int i = _wtoi(count) + 1;
										wchar_t count2[8];
										m_MiscAlbum.SetAt(temp_album, _itow(i, count2, 10));
									}
									else
									{
										m_MiscAlbum.SetAt(temp_album, L"1");
									}
								}
								if (wcslen(genre) == 0)
								{
									wchar_t temp_genre[MAX_PATH] = L"";
									pFileInfo->getGenre(temp_genre);
									// increase count
									wchar_t *count = NULL;
									if (m_MixedGenre.Lookup(temp_genre, count))
									{
										int i = _wtoi(count) + 1;
										wchar_t count2[8];
										m_MixedGenre.SetAt(temp_genre, _itow(i, count2, 10));
									}
									else
									{
										m_MixedGenre.SetAt(temp_genre, L"1");
									}
								}
								if (wcslen(artist) == 0)
								{
									wchar_t temp_artist[MAX_PATH] = L"";
									pFileInfo->getAlbumArtist(temp_artist);
									if (wcslen(temp_artist) == 0) pFileInfo->getArtist(temp_artist);
									StrTrimW(temp_artist, L" \t\0");
									// increase count
									wchar_t *count = NULL;
									if (m_VariousArtist.Lookup(temp_artist, count))
									{
										int i = _wtoi(count) + 1;
										wchar_t count2[8];
										m_VariousArtist.SetAt(temp_artist, _itow(i, count2, 10));
									}
									else
									{
										m_VariousArtist.SetAt(temp_artist, L"1");
									}
								}
								if (nYear == 0)
								{
									nYear = pFileInfo->getYear();
								}
							}
							delete pFileInfo;
						}
					}
				}
				++nCount;

				// reset it for next track
				bTrackHasTimeInfo = FALSE;
			}
		}

		fclose(f);
	}

	if (nCount > 0)
	{
		GenerateArtistName();
		GenerateAlbumName();
		GenerateGenre();

		// get year from dir if file is not in id3tag
		if (nYear == 0)
		{
			WIN32_FILE_ATTRIBUTE_DATA fileData;
			if (Win98GetFileAttributesEx(pathname, GetFileExInfoStandard, &fileData))
			{
				SYSTEMTIME st;
				if (FileTimeToSystemTime(&fileData.ftCreationTime, &st))
				{
					nYear = st.wYear;
				}
			}
			else
			{
				struct _stat buf;
				if (_wstat(pathname, &buf) == 0)
				{
					struct tm *t = NULL;
					if (t = localtime(&buf.st_ctime))
					{
						nYear = t->tm_year;
					}
				}
			}
		}
	
		wcsncpy(m3u_file, pathname, MAX_PATH);
		PathRemoveFileSpecW(pathname);
		wcsncpy(path, pathname, MAX_PATH);

		GenerateDisplayName(path);
	}
	m_VariousArtist.RemoveAll();
	m_MiscAlbum.RemoveAll();
	m_MixedGenre.RemoveAll();

	return nCount > 0;
}

BOOL CAlbum::AddTrack(LPCWSTR path, LPCWSTR filename /*=NULL*/)
{
	wchar_t pathname[MAX_PATH] = L"";

	// create pathname using the path and filename
	wcscat(pathname, path);
	if (filename != NULL)
		PathAppendW(pathname, filename);

	CFileInfo *pFileInfo = CreateFileInfo(pathname);
	if (pFileInfo)
	{
		if (pFileInfo->loadInfo(pathname) == 0)
		{ 
			nTotalTime += pFileInfo->getLengthInSeconds();

			if (wcslen(album) == 0)
			{
				wchar_t temp_album[MAX_PATH] = L"";
				pFileInfo->getAlbum(temp_album);

				// fix up the name for multi-disc albums
				FixMultiDiscAlbumName(temp_album);

				// increase count
				wchar_t *count = NULL;
				if (m_MiscAlbum.Lookup(temp_album, count))
				{
					int i = _wtoi(count) + 1;
					wchar_t count2[8];
					m_MiscAlbum.SetAt(temp_album, _itow(i, count2, 10));
				}
				else
				{
					m_MiscAlbum.SetAt(temp_album, L"1");
				}
			}

			if (wcslen(genre) == 0)
			{
				wchar_t temp_genre[MAX_PATH] = L"";
				pFileInfo->getGenre(temp_genre);
				// increase count
				wchar_t *count = NULL;
				if (m_MixedGenre.Lookup(temp_genre, count))
				{
					int i = _wtoi(count) + 1;
					wchar_t count2[8];
					m_MixedGenre.SetAt(temp_genre, _itow(i, count2, 10));
				}
				else
				{
					m_MixedGenre.SetAt(temp_genre, L"1");
				}
			}

			if (wcslen(artist) == 0)
			{
				wchar_t temp_artist[MAX_PATH] = L"";
				pFileInfo->getAlbumArtist(temp_artist);
				if (wcslen(temp_artist) == 0) pFileInfo->getArtist(temp_artist);
				StrTrimW(temp_artist, L" \t\0");
				// increase count
				wchar_t *count = NULL;
				if (m_VariousArtist.Lookup(temp_artist, count))
				{
					int i = _wtoi(count) + 1;
					wchar_t count2[8];
					m_VariousArtist.SetAt(temp_artist, _itow(i, count2, 10));
				}
				else
				{
					m_VariousArtist.SetAt(temp_artist, L"1");
				}
			}

			if (nYear == 0)
			{
				nYear = pFileInfo->getYear();
			}
		}
		delete pFileInfo;
	}

	nCount++;

	return TRUE;
}

WORD CAlbum::IsOutdated()
{
	// save and change directory
//	CChangeDir changed_to_dir(path);

	CStringArrayW dirs;

	// update multidisc status
	if (m_pObj->m_bMultiDiscFix)
	{
		BOOL bMultiDisc = GetDiscDirs(path, dirs);

		if ((bMultiDisc && !IsMultiDisc()) ||
			(!bMultiDisc && IsMultiDisc()))
			return IOD_REMOVE;
	}
	else
	{
		dirs.Add(L".");
		if (IsMultiDisc())
			return IOD_REMOVE;
	}

	if (IsPlaylistBased())
	{
		// check if the file is there still
		if (!PathFileExistsW(m3u_file))
			return IOD_REMOVE;
	}
	// file based
	else
	{
		wchar_t mypath[256];
		int count = 0;

		// check if there is now a playlist
		if (!m_pObj->m_bIgnorePL)
		{
			// find all the *.ext in this directory
			wcscpy(mypath, path);
			PathAppendW(mypath, L"*.m3u");

			WIN32_FIND_DATAW FindFileData;
			HANDLE hFile;
			if ((hFile = FindFirstFileW(mypath, &FindFileData)) != INVALID_HANDLE_VALUE)
			{
				FindClose(hFile);
				return IOD_REMOVE;
			}
		}

		// for all the extensions we have
		for (int d=0; d<dirs.GetSize(); d++)
		{
			wchar_t mydir[MAX_PATH];
			wcscpy(mydir, path);
			PathAppendW(mydir, dirs[d]);

			CTokensW extensions(m_pObj->m_szExtList, L" ?,\"'~!.-:;\t\n");
			int size = extensions.GetSize();
			for (int i=0; i<size; ++i)
			{
				// find all the *.ext in this directory
				wcscpy(mypath, mydir);
				PathAppendW(mypath, L"*.");
				wcscat(mypath, extensions[i]);

				WIN32_FIND_DATAW FindFileData;
				HANDLE hFile;
				if ((hFile = FindFirstFileW(mypath, &FindFileData)) != INVALID_HANDLE_VALUE)
				{
					do
					{
						if (_wcsicmp(PathFindExtensionW(FindFileData.cFileName)+1, extensions[i]) == 0)
						{
							++count;
						}
					}
					while (FindNextFileW(hFile, &FindFileData) != 0);

					FindClose(hFile);
				}
			}
		}

		// no songs here anymore?
		if (count == 0) return IOD_REMOVE;

		// number of songs is different?
		if (count != nCount) return IOD_REFRESH;
	}

	return IOD_UPTODATE;
}

BOOL CAlbum::LibrarySendTo(librarySendToMenuStruct s)
{
	extern int IPC_LIBRARY_SENDTOMENU;

	CStringArrayW files;
	GetFileList(files);

	int size = files.GetSize();

	if (s.data_type == ML_TYPE_ITEMRECORDLIST)
	{
		itemRecordList list;
		list.Alloc = size;
		list.Items = (itemRecord*)malloc(size * sizeof(itemRecord));
		list.Size = size;

		memset(list.Items, 0, size * sizeof(itemRecord));

		for (int i=0; i<size; ++i)
		{
			list.Items[i].filename = strdup(AutoChar(files[i]));
		}

		s.data = &list;
		s.mode = 3;
  		wndWinamp.SendIPCMessage((WPARAM)&s, IPC_LIBRARY_SENDTOMENU);
		//TODO: cleanup
	}
	else if (s.data_type == ML_TYPE_ITEMRECORDLISTW)
	{
		itemRecordListW list;
		list.Alloc = size;
		list.Items = (itemRecordW*)malloc(size * sizeof(itemRecordW));
		list.Size = size;

		memset(list.Items, 0, size * sizeof(itemRecordW));

		for (int i=0; i<size; ++i)
		{
			list.Items[i].filename = _wcsdup(files[i]);
		}

		s.data = &list;
		s.mode = 3;
  		wndWinamp.SendIPCMessage((WPARAM)&s, IPC_LIBRARY_SENDTOMENU);
		//TODO: cleanup
	}
	else if (s.data_type == ML_TYPE_FILENAMES)
	{
		UINT uBuffSize = 0;
		for (int i=0; i<size; ++i)
		{
			uBuffSize += strlen(AutoChar(files[i])) + 1;
		}

		if (uBuffSize == 0) return FALSE;

		char *str = (char*)malloc(sizeof(char) * (uBuffSize + 1));
		if (str)
		{
			char *ptr = str;
			for (int i=0; i<size; ++i)
			{
				strcpy (ptr, AutoChar(files[i]));
				ptr = strchr (ptr, '\0') + 1;
			}
			*ptr = 0;

			s.data = str;
			s.mode = 3;
  			wndWinamp.SendIPCMessage((WPARAM)&s, IPC_LIBRARY_SENDTOMENU);

			free(str);
		}
	}
	else if (s.data_type == 7/*ML_TYPE_FILENAMESW*/)
	{
		UINT uBuffSize = 0;
		for (int i=0; i<size; ++i)
		{
			uBuffSize += wcslen(files[i]) + 1;
		}

		if (uBuffSize == 0) return FALSE;

		wchar_t *str = (wchar_t*)malloc(sizeof(wchar_t) * (uBuffSize + 1));
		if (str)
		{
			wchar_t *ptr = str;
			for (int i=0; i<size; ++i)
			{
				wcscpy (ptr, files[i]);
				ptr = wcschr (ptr, '\0') + 1;
			}
			*ptr = 0;

			s.data = str;
			s.mode = 3;
  			wndWinamp.SendIPCMessage((WPARAM)&s, IPC_LIBRARY_SENDTOMENU);

			free(str);
		}
	}

	return TRUE;
}

BOOL CAlbum::Play()
{
	// delete the playlist first to winamp
	wndWinamp.SendIPCMessage(0, IPC_DELETE);

	Enqueue();

	int ksp = wndWinampAL.GetSetting(settingKeepSongPlaying);

	// setup the main window for keep song playing
	// 1) keep song play is on
	// 2) song is playing
	if (ksp && (wndWinamp.SendIPCMessage(0, IPC_ISPLAYING) != 0))
		wndWinamp.KeepSongPlaying();

	// start playback
	// 1) if don't keep current song playing
	// 2) if it is not playing
	if (!ksp || (wndWinamp.SendIPCMessage(0, IPC_ISPLAYING) == 0))
	{
		wndWinamp.SendMessage(WM_COMMAND, WINAMP_BUTTON4, 0);
		wndWinamp.SendIPCMessage(0, IPC_STARTPLAY);
	}

	// notify the list
	if (m_pObj) m_pObj->SetCurAlbum(id);

	return TRUE;
}

BOOL CAlbum::EnqueueAndPlay()
{
	int size = wndWinamp.SendIPCMessage(0, IPC_GETLISTLENGTH);

	if (!Enqueue())
		return FALSE;

	if (size < wndWinamp.SendIPCMessage(0, IPC_GETLISTLENGTH))
	{
		wndWinamp.SendIPCMessage(size, IPC_SETPLAYLISTPOS);
		wndWinamp.SendMessage(WM_COMMAND, WINAMP_BUTTON4, 0);
		wndWinamp.SendMessage(WM_COMMAND, WINAMP_BUTTON2, 0);

		return TRUE;
	}

	return FALSE;
}

BOOL CAlbum::EnqueueAfterCurrent()
{
	int index = wndWinampPE.SendMessage(WM_WA_IPC, IPC_PE_GETCURINDEX, 0);

	BOOL bTemp = FALSE;
	wchar_t playlist[MAX_PATH];

	if (IsPlaylistBased())
	{
		wcscpy(playlist, m3u_file);
	}
	else
	{
		// create temperary playlist
		wchar_t dirTemp[MAX_PATH];
		if (GetTempPathW(MAX_PATH, dirTemp) == 0)
			return FALSE;

		GetTempFileNameW(dirTemp, L"wal", 0, playlist);
		DeleteFileW(playlist);
		PathRenameExtensionW(playlist, L".m3u");

		if (!WritePlaylist(TRUE /*full path*/, FALSE /*no info*/, playlist))
			return FALSE;

		bTemp = TRUE;
	}

	// add separator only if there were stuff in the playlist
	int size = wndWinamp.SendIPCMessage(0, IPC_GETLISTLENGTH);
	if (size && GetModuleHandle("in_text.dll"))
	{
		char str[MAX_PATH] = "separator://-";

		// custom separator??
		//_snprintf(str, MAX_PATH, "separator://?s=---- <%s> ----", name);

		COPYDATASTRUCT cds;
		cds.dwData = IPC_PLAYFILE;
		cds.lpData = (void *) str;
		cds.cbData = lstrlen((char *) cds.lpData)+1; // include space for null
//		wndWinamp.SendMessage(WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
	}

	fileinfo f;
	lstrcpyn(f.file, AutoChar(playlist), MAX_PATH);	// path to the file
	f.index = index;						// insert file position

	//TODO:UNICODE
	COPYDATASTRUCT cds;
	cds.dwData = IPC_PE_INSERTFILENAME;
	cds.lpData = (void *) &f;
	cds.cbData = sizeof(fileinfo);
	wndWinampPE.SendMessage(WM_COPYDATA, 0, (LPARAM)&cds);

	if (bTemp) DeleteFileW(playlist);

	return TRUE;
}

BOOL CAlbum::Enqueue()
{
	BOOL bTemp = FALSE;
	wchar_t playlist[MAX_PATH];

	if (IsPlaylistBased())
	{
		wcscpy(playlist, m3u_file);
	}
	else
	{
		// create temperary playlist
		wchar_t dirTemp[MAX_PATH];
		if (GetTempPathW(MAX_PATH, dirTemp) == 0)
			return FALSE;

		GetTempFileNameW(dirTemp, L"wal", 0, playlist);
		DeleteFileW(playlist);
		PathRenameExtensionW(playlist, L".m3u");

		if (!WritePlaylist(TRUE /*full path*/, FALSE /*no info*/, playlist))
			return FALSE;

		bTemp = TRUE;
	}

	// add separator only if there were stuff in the playlist
	int size = wndWinamp.SendIPCMessage(0, IPC_GETLISTLENGTH);
	if (size && GetModuleHandle("in_text.dll"))
	{
		char str[MAX_PATH] = "separator://-";

		// custom separator??
		//_snprintf(str, MAX_PATH, "separator://?s=---- <%s> ----", name);

		COPYDATASTRUCT cds;
		cds.dwData = IPC_PLAYFILE;
		cds.lpData = (void *) str;
		cds.cbData = lstrlen((char *) cds.lpData)+1; // include space for null
		wndWinamp.SendMessage(WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
	}

	if (wndWinamp.IsWinamp513())
	{
		COPYDATASTRUCT cds;
		cds.dwData = IPC_PLAYFILEW;
		cds.lpData = (void *) playlist;
		cds.cbData = (wcslen((wchar_t *) cds.lpData)+1)*sizeof(wchar_t); // include space for null
		wndWinamp.SendMessage(WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
	}
	else
	{
		AutoChar list(playlist);

		COPYDATASTRUCT cds;
		cds.dwData = IPC_PLAYFILE;
		cds.lpData = (void *) list;
		cds.cbData = lstrlen((char *) cds.lpData)+1; // include space for null
		wndWinamp.SendMessage(WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
	}

	if (bTemp) DeleteFileW(playlist);

	return TRUE;
}

BOOL CAlbum::WritePlaylist()
{
	//TODO: show dialog to update album name
	return WritePlaylist(FALSE /*relative path*/, TRUE /*full info*/);
}

BOOL CAlbum::WritePlaylist(BOOL bFullPath, BOOL bInfo, LPCWSTR szOutput /*=NULL*/)
{
	wchar_t outfile[MAX_PATH];

	// generate default output file if not specified
	// album.m3u
	if (szOutput == NULL)
	{
		wcscpy(outfile, path);
		PathAppendW(outfile, L"album.m3u");
	}
	else
	{
		wcscpy(outfile, szOutput);
	}


	BOOL bRet = FALSE;

	CTrackExArray trackArray;
	wchar_t mypath[MAX_PATH];

	// get disc dirs
	CStringArrayW dirs;
	GetDiscDirs(path, dirs);

	for (int d=0; d<dirs.GetSize(); d++)
	{
		wchar_t mydir[MAX_PATH];
		wcscpy(mydir, path);
		PathAppendW(mydir, dirs[d]);

		// for all the extensions we have
		CTokensW extensions(m_pObj->m_szExtList, L" ?,\"'~!.-:;\t\n");
		int size = extensions.GetSize();
		for (int i=0; i<size; i++)
		{
			// find all the *.mp3 in this directory
			wcscpy(mypath, mydir);
			PathAppendW(mypath, L"*.");
			wcscat(mypath, extensions[i]);

			WIN32_FIND_DATAW FindFileData;
			HANDLE hFile;
			if ((hFile = FindFirstFileW(mypath, &FindFileData)) != INVALID_HANDLE_VALUE)
			{
				wchar_t songname[MAX_PATH] = L"";
				wchar_t artistname[MAX_PATH] = L"";
				// get info about the .mp3 files
				do
				{
					if (_wcsicmp(PathFindExtensionW(FindFileData.cFileName)+1, extensions[i]) == 0)
					{
						LPTRACKINFOEX pTrackInfo = new TRACKINFOEX;
						wcscpy(pTrackInfo->path, mydir);
						PathAppendW(pTrackInfo->path, FindFileData.cFileName);
						pTrackInfo->nTrack = 0;
						pTrackInfo->nDisc = d;
						pTrackInfo->desc[0] = 0;
						trackArray.Add(pTrackInfo);

						CFileInfo *pFileInfo = CreateFileInfo(pTrackInfo->path);
						if (pFileInfo)
						{
							if (pFileInfo->loadInfo(pTrackInfo->path) == 0)
							{
								pTrackInfo->nTrack = pFileInfo->getTrackNumber();
								if (pFileInfo->getDiscNumber()) pTrackInfo->nDisc = pFileInfo->getDiscNumber();

								if (bInfo)
								{
									// get song title
									pFileInfo->getTitle(songname);
									if (wcslen(songname) == 0)
										wcscpy(songname, FindFileData.cFileName);

									pFileInfo->getArtist(artistname);
									if (wcslen(artistname) == 0)
										wcscpy(artistname, artist);

									// generate description
									swprintf(pTrackInfo->desc, L"#EXTINF:%d,%s - %s\r\n", pFileInfo->getLengthInSeconds(), artistname, songname);
								}
							}
							delete pFileInfo;
						}

						// reset to relative path (if required)
						if (!bFullPath)
						{
							if (wcscmp(dirs[d], L".") == 0)
							{
								swprintf(pTrackInfo->path, L"%s\r\n", FindFileData.cFileName);
							}
							else
							{
								swprintf(pTrackInfo->path, L"%s\\%s\r\n", dirs[d], FindFileData.cFileName);
							}
						}
						// else add the "\r\n" to the end
						else
						{
							wcscat(pTrackInfo->path, L"\r\n");
						}
					}
				}
				while (FindNextFileW(hFile, &FindFileData) != 0);

				FindClose(hFile);
			}
		}
	}

	// found no tracks??
	if (trackArray.GetSize() == 0)
		return FALSE;

	int sbn = wndWinampAL.GetSetting(settingSortByFilename);

	// sort the playlist (by track number)
	if (trackArray.Sort(sbn))
	{
		// if there are duplicate track numbers
		// sort by filename
		if (!sbn)
		{
			DbgPrint("CAlbum::WritePlaylist - Resorting by filename\n");
			trackArray.Sort(1);
		}
	}

	HANDLE fp = NULL;
	if (INVALID_HANDLE_VALUE != (fp = CreateFileW(outfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)))
	{
		DWORD nBytesWritten;
		int i=0;

		BOOL bUTF8 = wndWinamp.IsWinamp530();
		int cp = wndWinampAL.GetEncodingCodepage();

		if (bUTF8)
		{
			WriteFile(fp, "﻿", 3, &nBytesWritten, NULL);
		}

		WriteTextToFile(fp, "#EXTM3U\r\n", bUTF8, cp);

		if (bInfo)
		{
			if (wcslen(artist))
			{
				wchar_t szArtist[MAX_PATH];
				swprintf(szArtist, L"#EXTART:%s\r\n", artist);
				WriteTextToFile(fp, szArtist, bUTF8, cp);
			}
			if (wcslen(album))
			{
				wchar_t szAlbum[MAX_PATH];
				swprintf(szAlbum, L"#EXTALB:%s\r\n", album);
				WriteTextToFile(fp, szAlbum, bUTF8, cp);
			}
			if (wcslen(genre))
			{
				wchar_t szGenre[MAX_PATH];
				swprintf(szGenre, L"#EXTGENRE:%s\r\n", genre);
				WriteTextToFile(fp, szGenre, bUTF8, cp);
			}
		}

		int size = trackArray.GetSize();
		for (i=0; i<size; i++)
		{
			LPTRACKINFOEX pTrackInfo = trackArray[i];

			if (wcslen(pTrackInfo->desc)) WriteTextToFile(fp, pTrackInfo->desc, bUTF8, cp);

			if (wcslen(pTrackInfo->path)) WriteTextToFile(fp, pTrackInfo->path, bUTF8, cp);
		}

		FlushFileBuffers(fp);
		CloseHandle(fp);
	}

	// clean up
	int size = trackArray.GetSize();
	for (int i=0; i<size; i++)
	{
		delete trackArray[i];
	}
	trackArray.RemoveAll();

	// no output file specified
	// change this dir to playlist
	if (szOutput == NULL)
	{
		wcscpy(m3u_file, outfile);
	}

	return TRUE;
}

BOOL CAlbum::ShowCover()
{
	// create temperary playlist
	char tempfile[MAX_PATH] = "";
	if (GetTempPath(MAX_PATH, tempfile) == 0)
		lstrcpy(tempfile, dirPlugin);
	PathAppend(tempfile, "albumlist.htm");

	CTemplateFile templatefile;
	templatefile.AddReplace("backgroundcolor", (COLORREF)WADlg_getColor(WADLG_WNDBG));

	//---------------------------
	// add bitmap links
	CPictureArray CoverPictures;
	CTokens extensions("*.jpg *.jpeg *.gif *.png", " ");
	int size = extensions.GetSize();
	for (int i=0; i<size; i++)
	{
		wchar_t bmppath[MAX_PATH];
		wcscpy(bmppath, path);
		PathAppendW(bmppath, AutoWide(extensions[i]));

		WIN32_FIND_DATAW FindFileData;
		HANDLE hFile;

		if ((hFile = FindFirstFileW(bmppath, &FindFileData)) != INVALID_HANDLE_VALUE)
		{
			do
			{
				LPPICTUREINFO pPicInfo = new PICTUREINFO;
				wcscpy(pPicInfo->file, path);
				PathAppendW(pPicInfo->file, FindFileData.cFileName);
		
				// convert "\" to "/"
				wchar_t *ptr = pPicInfo->file;
				while (*ptr)
				{
					if (*ptr == L'\\')
						*ptr = L'/';
					++ptr;
				}

				if (_wcsnicmp(FindFileData.cFileName, L"front.", 6) == 0)
					pPicInfo->nType = 0;
				if (_wcsnicmp(FindFileData.cFileName, L"cover.", 6) == 0)
					pPicInfo->nType = 0;
				else if (_wcsnicmp(FindFileData.cFileName, L"inside.", 7) == 0)
					pPicInfo->nType = 1;
				else if (_wcsnicmp(FindFileData.cFileName, L"cd.", 3) == 0)
					pPicInfo->nType = 2;
				else if (_wcsnicmp(FindFileData.cFileName, L"inlay.", 6) == 0)
					pPicInfo->nType = 3;
				else if (_wcsnicmp(FindFileData.cFileName, L"back.", 5) == 0)
					pPicInfo->nType = 4;
				else
					pPicInfo->nType = 64;

				CoverPictures.Add(pPicInfo);
			}
			while (FindNextFileW(hFile, &FindFileData) != 0);

			FindClose(hFile);
		}
	}

	CoverPictures.Sort();

	BOOL bFoundBitmap = FALSE;
	size = CoverPictures.GetSize();
	for (i=0; i<size; i++)
	{
		LPPICTUREINFO pPicInfo = CoverPictures[i];

		templatefile.AddReplace("slidepic", i, AutoChar(pPicInfo->file));
		bFoundBitmap = TRUE;

		delete pPicInfo;
	}
	CoverPictures.RemoveAll();

	// try custom cover	
    if (!bFoundBitmap && wcslen(m_pObj->m_szDefaultCover))
    {
        templatefile.AddReplace("slidepic",0, AutoChar(m_pObj->m_szDefaultCover));
		bFoundBitmap = TRUE;
    }

	//---------------------------
    // no bitmap found?
    if (!bFoundBitmap) return FALSE;

	//---------------------------
	// current album info
	char str[256];
	int hour = nTotalTime/3600;
	int min = (nTotalTime%3600)/60;
	int sec = nTotalTime%60;

	if (hour)
		wsprintf(str, "%ld:%02ld:%02ld", hour, min, sec);
	else
		wsprintf(str, "%ld:%02ld", min, sec);

	templatefile.AddReplace("albumname", AutoChar(album, wndWinampAL.GetEncodingCodepage()));
	templatefile.AddReplace("albumcount", nCount);
	templatefile.AddReplace("displayname", AutoChar(name, wndWinampAL.GetEncodingCodepage()));
	templatefile.AddReplace("albumartist", AutoChar(artist, wndWinampAL.GetEncodingCodepage()));
	templatefile.AddReplace("albumyear", nYear);
	templatefile.AddReplace("albumlengthhrs", hour);
	templatefile.AddReplace("albumlengthmin", min);
	templatefile.AddReplace("albumlengthsec", sec);
	templatefile.AddReplace("albumlength", str);

	//---------------------------
	// convert the file
	char szTemplateFile[MAX_PATH];
	lstrcpy(szTemplateFile, dirPlugin);
	PathAppend(szTemplateFile, "Album List\\Templates\\cover.htt");
	if (!templatefile.Convert(tempfile, szTemplateFile)) return FALSE;

	//---------------------------
	// show the thing in the minibrowser
	char url[MAX_PATH*2] = "file:///";
	lstrcat(url, tempfile);
	wndWinamp.SendIPCMessage(0, IPC_MBBLOCK);
	wndWinamp.SendIPCMessage(0, IPC_MBOPEN);
	wndWinamp.SendIPCMessage((WPARAM)url, IPC_MBOPEN);
	wndWinamp.SendIPCMessage(1, IPC_MBBLOCK);

//	wndWinamp.SendIPCMessage((WPARAM)url, IPC_MBURL);

	return TRUE;
}

BOOL CAlbum::ShowInfo(CWnd wnd)
{
	CAlbumInfo info(this);
	return (info.DoModal(&wnd) == IDOK);
}

BOOL CAlbum::Refresh()
{
	// save and change directory
//	CChangeDir changed_to_dir(path);

	// notify cover is dirty
	if (m_pObj && (nIconIndex != -2))
		m_pObj->NotifyCoverChanged();
		
	// reset everything (if not modified by the user)
	if ((dwFlags & AI_USERMODIFIED) == 0)
	{
		memset(name, 0, MAX_PATH);		// display name (combination of artist and album)
		memset(album, 0, MAX_PATH);		// album name
		memset(artist, 0, MAX_PATH);	// artist name
		memset(genre, 0, MAX_PATH);		// genre
		nYear		= 0;				// year of this album

		nTotalTime	= 0;				// total time in the directory (or playlist)
		nCount		= 0;				// number of files in this directory (or playlist)
		dwFlags		= 0;				// flags

		nIconIndex	= -2;				// reset icon index
	}
	else	// user modified some stuff
	{
		nTotalTime	= 0;				// total time in the directory (or playlist)
		nCount		= 0;				// number of files in this directory (or playlist)
		dwFlags		= AI_USERMODIFIED;	// flags

		nIconIndex	= -2;				// reset icon index
	}

	// notify cache is dirty
	if (m_pObj) m_pObj->NotifyAlbumChanged();

	// re-read directory
	if (IsPlaylistBased())
		return AddM3U(m3u_file, nPathLen);
	else
		return AddDir(path, nPathLen);
}

BOOL CAlbum::Explore()
{
	return (ShellExecuteW(NULL, L"explore", path, NULL, NULL, SW_SHOW) > (HINSTANCE)32);
}

LPCWSTR CAlbum::GetPath()
{
	return path;
}

LPCWSTR CAlbum::GetM3U()
{
	return m3u_file;
}

LPCWSTR CAlbum::GetTitle()
{
	return name;
}

LPCWSTR CAlbum::GetArtist()
{
	return artist;
}

LPCWSTR CAlbum::GetAlbum()
{
	return album;
}

LPCWSTR CAlbum::GetGenre()
{
	return genre;
}

int CAlbum::GetTotalTime()
{
	return nTotalTime;
}

int CAlbum::GetTrackCount()
{
	return nCount;
}

int CAlbum::GetYear()
{
	return nYear;
}

void CAlbum::SetTitle(LPCWSTR szTitle)
{
	wcsncpy(name, szTitle, MAX_PATH);

	dwFlags |= AI_USERMODIFIED;
}

void CAlbum::SetArtist(LPCWSTR szArtist)
{
	if (wcscmp(artist, szArtist) != 0)
	{
		wcsncpy(artist, szArtist, MAX_PATH);
		dwFlags |= AI_USERMODIFIED;
	}
}

void CAlbum::SetAlbum(LPCWSTR szAlbum)
{
	if (wcscmp(album, szAlbum) != 0)
	{
		wcsncpy(album, szAlbum, MAX_PATH);
		dwFlags |= AI_USERMODIFIED;
	}
}

void CAlbum::SetYear(int year)
{
	if (nYear != year)
	{
		nYear = year;
		dwFlags |= AI_USERMODIFIED;
	}
}

void CAlbum::SetGenre(LPCWSTR szGenre)
{
	if (wcscmp(genre, szGenre) != 0)
	{
		wcsncpy(genre, szGenre, MAX_PATH);
		dwFlags |= AI_USERMODIFIED;
	}
}

void CAlbum::GenerateArtistName()
{
	// we treat more than 1 artist in an album to be 
	// a "various artists" album
	if (wcslen(artist) == 0)
	{
		int size = m_VariousArtist.GetCount();

		// more than 1 artist name
		// use the name that is over 70%
		if (size > 1)
		{
			// find substrings so that in VA albums
			// artist name "A & B" is also counted in artist "A" and artist "B"
			// if they happen to occur by themselves
			POSITION pos = m_VariousArtist.GetStartPosition();
			while (pos)
			{
				wchar_t *key, *value;
				m_VariousArtist.GetNextAssoc(pos, key, value);
				int count1 = _wtoi(value);
				
				POSITION pos2 = m_VariousArtist.GetStartPosition();
				while (pos2)
				{
					wchar_t *key2, *value2;
					m_VariousArtist.GetNextAssoc(pos2, key2, value2);
					if (pos != pos2)
					{
						if (wcsstr(key2, key) != NULL)
						{
							wchar_t count2[8];
							m_VariousArtist.SetAt(key, _itow(count1 + _wtoi(value2), count2, 10));
						}
					}
				}
			}

			float passRatio = wndWinampAL.GetSetting(settingVariousArtistRatio) / 100.0f;

			// find the one that's over 70%
			float maxRatio = 0.0f;
			POSITION pos3 = m_VariousArtist.GetStartPosition();
			while (pos3)
			{
				wchar_t *key, *value;
				m_VariousArtist.GetNextAssoc(pos3, key, value);

				float ratio = (float)_wtoi(value)/(float)nCount;
				if (ratio >= passRatio && ratio > maxRatio)
				{
					wcscpy(artist, key);
					maxRatio = ratio;
				}
			}

			// no artist name over 70%?
			// treat this as a various artist album
			if (wcslen(artist) == 0)
			{
				wcscpy(artist, AutoWide(ALS("Various Artists"), wndWinampAL.GetCodepage()));
			}
		}
		// only 1 artist name
		else if (size > 0)
		{
			POSITION pos = m_VariousArtist.GetStartPosition();
			if (pos)
			{
				wchar_t *key, *value;
				m_VariousArtist.GetNextAssoc(pos, key, value);
				wcscpy(artist, key);
			}
		}
		// no artist name
		// leave it as is
	}
}

void CAlbum::GenerateAlbumName()
{
	// we treat more than 1 album name in an album to be 
	// a "misc" album
	if (wcslen(album) == 0)
	{
		int size = m_MiscAlbum.GetCount();

		// more than 1 album name
		// use the name that is over 70%
		if (size > 1)
		{
			POSITION pos = m_MiscAlbum.GetStartPosition();
			while (pos)
			{
				wchar_t *key, *value;
				m_MiscAlbum.GetNextAssoc(pos, key, value);

				if ((float)_wtoi(value)/(float)nCount >= 0.7)
				{
					wcscpy(album, key);
					break;
				}
			}

			// no album name over 70%?
			// treat this as a various album
			if (wcslen(album) == 0)
			{
				wcscpy(album, AutoWide(ALS("Misc"), wndWinampAL.GetCodepage()));
			}
		}
		// only 1 album name
		else if (size > 0)
		{
			POSITION pos = m_MiscAlbum.GetStartPosition();
			if (pos)
			{
				wchar_t *key, *value;
				m_MiscAlbum.GetNextAssoc(pos, key, value);
				wcscpy(album, key);
			}
		}
		// no album name
		// leave it as is
	}
}

void CAlbum::GenerateGenre()
{
	// we treat more than 1 genre in an album to be 
	// a "mixed" genre
	if (wcslen(genre) == 0)
	{
		int size = m_MixedGenre.GetCount();

		// more than 1 genre
		// use the name that is over 70%
		if (size > 1)
		{
			POSITION pos = m_MixedGenre.GetStartPosition();
			while (pos)
			{
				wchar_t *key, *value;
				m_MixedGenre.GetNextAssoc(pos, key, value);

				if ((float)_wtoi(value)/(float)nCount >= 0.7)
				{
					wcscpy(genre, key);
					break;
				}
			}

			// no genre over 70%?
			// treat this as a mixed genre
			if (wcslen(genre) == 0)
			{
				wcscpy(genre, AutoWide(ALS("Mixed"), wndWinampAL.GetCodepage()));
			}
		}
		// only 1 genre
		else if (size > 0)
		{
			POSITION pos = m_MixedGenre.GetStartPosition();
			if (pos)
			{
				wchar_t *key, *value;
				m_MixedGenre.GetNextAssoc(pos, key, value);
				wcscpy(genre, key);
			}
		}
		// no genre
		// leave it as is
	}
}

void CAlbum::GenerateDisplayName(LPCWSTR dir)
{
	name[MAX_PATH-1] = 0;

	// use ID3 tags
	if (m_pObj->m_bUseID3)
	{
		if ((wcslen(album) != 0) && (wcslen(artist) != 0))
		{
			swprintf(name, L"%s - %s", artist, album);
			return;
		}
	}

	// root of directory
	if (nPathLen == (int)wcslen(dir))
	{
		if (wcslen(artist) == 0)
			wcscpy(artist, AutoWide(ALS("Unknown Artist"), wndWinampAL.GetCodepage()));
		if (wcslen(album) == 0)
			wcscpy(album, AutoWide(ALS("Unknown Album"), wndWinampAL.GetCodepage()));

		if (wcslen(m3u_file))
		{
			wchar_t temp[MAX_PATH];
			wcsncpy(temp, m3u_file, MAX_PATH);
			PathRemoveExtensionW(temp);
			PathStripPathW(temp);
			swprintf(name, L"{Root} %s", temp);
		}
		else
		{
			swprintf(name, L"{Root} %s", dir);
		}
		return;
	}

	// use directory styles
	wchar_t szDir[MAX_PATH];
	wchar_t szMultiCD[MAX_PATH] = L"";

	if (nPathLen < (int)wcslen(dir))
	{
		wcsncpy(szDir, dir+nPathLen, MAX_PATH);
	}
	else
	{
		wcsncpy(szDir, dir, MAX_PATH);
	}

	// convert %20 & underscore to spaces
	if (m_pObj->m_bFixTitles)
	{
		FixTitle(szDir);
	}
	
	if (m_pObj->m_bMultiDiscFix) 
	{
		// find the last "\"
		wchar_t *ptr = wcsrchr(szDir, L'\\');

		//Make sure we aren�t pointing at the first and only "\"
		if ((ptr - szDir) > 3)
		{
			//This is a hack so "\\Artist - Double Album\\CD 2" gets treated properly by removing
			//the "CD 2" part from the dir and then appending it to name deduced from the
			//parent directory
			if (IsFolderMultiDisc(ptr + 1))
			{
				swprintf(szMultiCD, L" (%s)", ptr + 1);
				*ptr = '\0';
			}
		}
	}

	CTokensW subdirs(szDir, L"\\");
	int size = subdirs.GetSize();

	// smart detect
	if (m_pObj->m_nDirStyle == 0)
	{
		// check last sub dir for artist & album
		if (size > 0)
		{
			CNamingTokensW parts(subdirs[size-1]);
			int sz = parts.GetSize();

			// check for year
			wchar_t str[MAX_PATH];
			for (int i=0; i<sz; ++i)
			{
				TrimYear(str, parts[i]);

				int year = _wtoi(str);
				if (1900 <= year && year <= 2100)
				{
					nYear = year;
					parts.RemoveAt(i);
					break;
				}
			}

			sz = parts.GetSize();
			if (sz == 0)
			{
				if (size > 1)
				{
					size -= 1;
					parts.Parse(subdirs[size-1]);
					sz = parts.GetSize();
				}
			}

			// only 1 part? treat it as album, we'll get artist from the above level
			if (sz == 1)	
			{
				if (size > 1)
				{
					wcscpy(album, parts[0]);
					wcscpy(artist, subdirs[size-2]);
				}
				else
				{
					wcscpy(artist, parts[0]);
				}
			}
			// first part artist, rest album
			else if (sz > 1)
			{
				wcscpy(artist, parts[0]);

				album[0] = 0;
				for (int i=1; i<sz; ++i)
				{
					if (i != 1) wcscat(album, L" - ");
					wcscat(album, parts[i]);
				}
			}
		}
	}
	// "...\Artist\Album"
	else if (m_pObj->m_nDirStyle == 1)
	{
		// last sub dir is album
		if (size > 0)
			wcsncpy(album, subdirs[size-1], MAX_PATH);

		// second last sub dir is artist
		if (size > 1)
			wcsncpy(artist, subdirs[size-2], MAX_PATH);
	}
	// "...\Artist - Album"
	else if (m_pObj->m_nDirStyle == 2)
	{
		// check last sub dir for artist & album
		if (size > 0)
		{
			CNamingTokensW parts(subdirs[size-1]);
			int sz = parts.GetSize();

			// first part is artist
			if (sz > 0)
				wcsncpy(artist, parts[0], MAX_PATH);

			// the rest are album
			album[0] = 0;
			for (int i=1; i<sz; ++i)
			{
				if (i != 1) wcscat(album, L" - ");
				wcscat(album, parts[i]);
			}
		}
	}
	// "...\Artist - Album - Year"
	else if (m_pObj->m_nDirStyle == 3)
	{
		// check last sub dir for artist & album
		if (size > 0)
		{
			CNamingTokensW parts(subdirs[size-1]);
			int sz = parts.GetSize();

			// first part is artist
			if (sz > 0)
				wcsncpy(artist, parts[0], MAX_PATH);

			// the middle parts is album
			if (sz > 2)			// we have 3 parts
			{
				wcsncpy(album, parts[1], MAX_PATH);
				for (int i=2; i<sz; ++i)
				{
					int year = _wtoi(parts[i]);
					if (1900 <= year && year <= 2100)
					{
						nYear = year;
						break;
					}

					wcscat(album, L" - ");
					wcscat(album, parts[i]);
				}
			}
			// not enough parameters
			else if (sz > 1)	// we have 2 parts
			{
				int year = _wtoi(parts[1]);
				if (1900 <= year && year <= 2100)
					nYear = year;
				else
					wcscat(album, parts[1]);
			}
		}
	}
	// "...\Artist\Year - Album"
	else if (m_pObj->m_nDirStyle == 4)
	{
		// last sub dir is year - album
		if (size > 0)
		{
			CNamingTokensW parts(subdirs[size-1]);
			int sz = parts.GetSize();

			if (sz > 1)
			{
				// first part is year
				int year = _wtoi(parts[0]);
				if (1900 <= year && year <= 2100)
					nYear = year;
				// the rest are album
				album[0] = 0;
				for (int i=1; i<sz; ++i)
				{
					if (i != 1) wcscat(album, L" - ");
					wcscat(album, parts[i]);
				}
			}
			// not enough parameters
			else
			{
				int year = _wtoi(parts[0]);
				if (1900 <= year && year <= 2100)
					nYear = year;
				else
					// first part is year
					wcsncpy(album, parts[0], MAX_PATH);
			}
		}

		// second last sub dir is artist
		if (size > 1)
			wcsncpy(artist, subdirs[size-2], MAX_PATH);
	}
	// "...\Artist - Year - Album"
	else if (m_pObj->m_nDirStyle == 5)
	{
		// check last sub dir for artist & album
		if (size > 0)
		{
			CNamingTokensW parts(subdirs[size-1]);
			int sz = parts.GetSize();

			// first part is artist
			if (sz > 0)
				wcsncpy(artist, parts[0], MAX_PATH);

			// the middle parts is album
			if (sz > 2)			// we have 3 parts
			{
				album[0] = 0;
				for (int i=2; i<sz; ++i)
				{
					if (i != 2) wcscat(album, L" - ");
					wcscat(album, parts[i]);
				}

				int year = _wtoi(parts[1]);
				if (1900 <= year && year <= 2100)
					nYear = year;
			}
			// not enough parameters
			else if (sz > 1)	// we have 2 parts
			{
				int year = _wtoi(parts[1]);
				if (1900 <= year && year <= 2100)
					nYear = year;
				else
					wcscat(album, parts[1]);
			}
		}
	}

	TrimWhiteSpace(artist);
	TrimWhiteSpace(album);

	// generate display based on artist and album names
	if (wcslen(artist) == 0)
		wcscpy(artist, AutoWide(ALS("Unknown Artist"), wndWinampAL.GetCodepage()));
	if (wcslen(album) == 0)
		wcscpy(album, AutoWide(ALS("Unknown Album"), wndWinampAL.GetCodepage()));
	swprintf(name, L"%s - %s", artist, album);

	//add (Disc X) etc to the end of the name directory contained multiple discs.
	wcscat(name, szMultiCD);
}

BOOL CAlbum::IsPlaylistBased()
{
	return (wcslen(m3u_file) != 0);
}

void CAlbum::GetFileList(CStringArrayW &files)
{
	if (IsPlaylistBased())
	{
		files.Add(m3u_file);
	}
	else
	{
		CTrackExArray trackArray;

		// get disc dirs
		CStringArrayW dirs;
		GetDiscDirs(path, dirs);

		for (int d=0; d<dirs.GetSize(); d++)
		{
			wchar_t mydir[MAX_PATH];
			wcscpy(mydir, path);
			PathAppendW(mydir, dirs[d]);

			wchar_t mypath[MAX_PATH];

			CTokensW extensions(m_pObj->m_szExtList, L" ?,\"'~!.-:;\t\n");
			int size = extensions.GetSize();
			for (int i=0; i<size; i++)
			{
				// find all the *.mp3 in this directory
				wcscpy(mypath, mydir);
				PathAppendW(mypath, L"*.");
				wcscat(mypath, extensions[i]);

				WIN32_FIND_DATAW FindFileData;
				HANDLE hFile;
				if ((hFile = FindFirstFileW(mypath, &FindFileData)) != INVALID_HANDLE_VALUE)
				{
					// get info about the .mp3 files
					do
					{
						if (wcsicmp(PathFindExtensionW(FindFileData.cFileName)+1, extensions[i]) == 0)
						{
							LPTRACKINFOEX pTrackInfo = new TRACKINFOEX;
							wcscpy(pTrackInfo->path, mydir);
							PathAppendW(pTrackInfo->path, FindFileData.cFileName);
							pTrackInfo->nTrack = 0;
							pTrackInfo->nDisc = d;
							pTrackInfo->desc[0] = 0;
							trackArray.Add(pTrackInfo);

							CFileInfo *pFileInfo = CreateFileInfo(pTrackInfo->path);
							if (pFileInfo)
							{
								if (pFileInfo->loadInfo(pTrackInfo->path) == 0)
								{
									pTrackInfo->nTrack = pFileInfo->getTrackNumber();
									if (pFileInfo->getDiscNumber()) pTrackInfo->nDisc = pFileInfo->getDiscNumber();
								}
								delete pFileInfo;
							}
						}
					}
					while (FindNextFileW(hFile, &FindFileData) != 0);

					FindClose(hFile);
				}
			}
		}

		// found no tracks??
		if (trackArray.GetSize() == 0) return;

		int sbn = wndWinampAL.GetSetting(settingSortByFilename);

		// sort the playlist (by track number)
		if (trackArray.Sort(sbn))
		{
			// if there are duplicate track numbers
			// sort by filename
			if (!sbn)
			{
				DbgPrint("CAlbum::GetFileList - Resorting by filename\n");
				trackArray.Sort(1);
			}
		}

		int size = trackArray.GetSize();
		for (int i=0; i<size; ++i)
		{
			LPTRACKINFOEX pTrackInfo = trackArray[i];
			files.Add(pTrackInfo->path);
		}
	}
}

void CAlbum::FixTitle(LPWSTR str)
{
	int len = wcslen(str);

	for (int i=0; i<len; ++i)
	{
		// convert %20 to space in titles
		if (str[i] == L'%')
		{
			if (wcsncmp(str+i, L"%20", 3) == 0)
			{
				str[i] = L' ';

				// move the remaining characters forward
				for (int j=i+3; j<len; j++)
				{
					str[j-2] = str[j];
				}

				// adjust the length
				len -= 2;
				str[len] = 0;
			}
		}

		// convert underscore to space in titles
		else if (str[i] == L'_')
		{
			str[i] = L' ';
		}
	}
}

BOOL CAlbum::GetDiscDirs(LPCWSTR str, CStringArrayW &dirs)
{
	BOOL bFoundMultiDisc = FALSE;

	wchar_t mypath[MAX_PATH];
	wcscpy(mypath, str);

	if (!PathIsDirectoryW(mypath))
	{
		PathRemoveFileSpecW(mypath);
	}

	PathAppendW(mypath, L"*.*");

	dirs.Add(L".");

	WIN32_FIND_DATAW FindFileData;
	HANDLE hFile;
	if ((hFile = FindFirstFileW(mypath, &FindFileData)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp(FindFileData.cFileName, L".") && 
					wcscmp(FindFileData.cFileName, L".."))
				{
					if (IsFolderMultiDisc(FindFileData.cFileName))
					{
						dirs.Add(FindFileData.cFileName);
						bFoundMultiDisc = TRUE;
					}
				}
			}
		}
		while (FindNextFileW(hFile, &FindFileData) != 0);

		FindClose(hFile);
	}

	dirs.Sort();

	return bFoundMultiDisc;
}

BOOL CAlbum::FixMultiDiscAlbumName(LPWSTR str)
{
	wchar_t albumname[MAX_PATH];
	wchar_t multidiscnames[MAX_PATH];

	if (m_pObj->m_bMultiDiscFix)
	{
		// lower case version of the album name
		wcsncpy(albumname, str, MAX_PATH);
		_wcslwr(albumname);

		// lower case version of the "cd" or "disc"
		wcsncpy(multidiscnames, m_pObj->m_szMultiDiscNames, MAX_PATH);
		_wcslwr(multidiscnames);

		CTokensW disc(multidiscnames, L" ?,\"'~!.-:;\t\n");
		for (int i=0; i<disc.GetSize(); ++i)
		{
			//Check for "cd" or "disc" etc...
			wchar_t *p;
			if ((p = wcsstr(albumname, disc[i])) > albumname)
			{
				int len = wcslen(disc[i]);
				if ((wcschr(L" ([{<-", *(p-1)) != NULL) &&	// spacing before "cd" or "disc"
					(*(p + len) == L' '))					// space after "cd" or "disc"
				{
					while ((--p > albumname) && (wcschr(L" ([{<-_", *p) != NULL));
					str[p - albumname + 1] = NULL;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOL CAlbum::IsFolderMultiDisc(LPCWSTR str)
{
	CTokensW disc(m_pObj->m_szMultiDiscNames, L" ?,\"'~!.-:;\t\n");

	for (int i=0; i<disc.GetSize(); ++i)
	{
		//Check for "cd" or "disc" etc...
		int len = wcslen(disc[i]);
		if (_wcsnicmp(disc[i], str, len) == 0)
		{
			//Make sure it's followed by a number (whitespace ignored)
			if (_wtoi(str + len) > 0)
			{
				return TRUE;
			}
			else
			{
				wchar_t num[MAX_PATH] = L"";
				wcsncpy(num, str+len, MAX_PATH);
				StrTrimW(_wcsupr(num), L" \0");
				if (wcslen(num) == 1)
				{
					if ((L'A' <= num[0]) && (num[0] <= L'Z'))
						return TRUE;
				}
			}
		}
	}
	
	return FALSE;
}

BOOL CAlbum::IsMultiDisc()
{
	return ((dwFlags & AI_MULTIDISC) != 0);
}

int CAlbum::GetNumberOfDiscs()
{
	if (!IsMultiDisc()) return 1;

	CStringArrayW dirs;

	GetDiscDirs(path, dirs);

	return dirs.GetSize() - 1;
}

BOOL CAlbum::IsPartOfMultiDisc()
{
	LPWSTR str = PathFindFileNameW(path);

	return IsFolderMultiDisc(str);
}

void CAlbum::TrimWhiteSpace(LPWSTR str)
{
	if (str == NULL) return;

	int len = wcslen(str);
	if (len == 0) return;

	wchar_t *start = str;
	while (*start == L' ')
		++start;

	// whole string is white space?
	if (start - str >= len)
	{
		str[0] = 0;
		return;
	}

	wchar_t *end = str + len - 1;
	while (*end == L' ')
		--end;

	len = end-start+1;
	if (str != start)
	{
		memmove(str, start, len*sizeof(wchar_t));
	}
	*(str+len) = 0;
}

void CAlbum::TrimYear(LPWSTR strOut, LPCWSTR strIn)
{
	if (strIn == NULL) return;
	if (strOut == NULL) return;

	int len = wcslen(strIn);
	if (len == 0) return;

	// match a bracketed 4-digit year (6 chars)
	// note we're actually trying to match an opening bracket followed by a minimum of 4 digits
	BOOL match = FALSE;
	const wchar_t *start = strIn;
	const wchar_t *end = strIn;

	while (start <= strIn + len - 6 && end <= strIn + len - 1)
	{
		while (*start != L'[' && *start != L'(' && *start != L'{' && start <= strIn + len - 6)
			++start;

		// skip bracket
		++start;

		// test for digit
		if (*start < L'0' || *start > L'9')
		{
			continue;
		}

		// skip current digit
		end = start+1;

		while (*end >= L'0' && *end <= L'9' && end <= strIn + len - 1)
			++end;

		// min. 4 digits
		if (end-start+1 >= 4)
		{
			match = TRUE;
			break;
		}
	}
	
	if (!match)
	{
		wcscpy(strOut, L"");
		return;
	}

	len = end-start+1;
	wcsncpy(strOut, start, len);
}

int CAlbum::GetIconIndex()
{
	return nIconIndex;
}

void CAlbum::SetIconIndex(int index)
{
	nIconIndex = index;
}

LPCWSTR CAlbum::FindCover(LPCWSTR szPath, LPCWSTR szExt)
{
	wcscpy(m_szCover, szPath);
	wchar_t *p = m_szCover + wcslen(m_szCover);

	CTokensW extensions(szExt, L";");
	int size = extensions.GetSize();
	for (int i=0; i<size; i++)
	{
		*p = 0;
		PathAppendW(m_szCover, extensions[i]);

		WIN32_FIND_DATAW FindFileData;
		HANDLE hFile;

		if ((hFile = FindFirstFileW(m_szCover, &FindFileData)) != INVALID_HANDLE_VALUE)
		{
			*p = 0;
			PathAppendW(m_szCover, FindFileData.cFileName);
			FindClose(hFile);
			return m_szCover;
		}
	}

	return NULL;
}

void TrimFilename(LPCWSTR szTrim, LPWSTR str)
{
	LPWSTR s = str;
	LPWSTR c = str;

	while (*s)
	{
		LPCWSTR p = szTrim;
		while (*p && (*s != *p)) p++;

		if (*p == 0)
		{
			*c++ = *s;
		}
		s++;
	}
	*c = 0;
}

LPCWSTR CAlbum::GetCoverFilename()
{
	const wchar_t *ext = m_pObj->m_szCoverSearchExt;
	LPCWSTR szReturn= NULL;

	// 1. try alternate folder for cover
	if (m_pObj->m_bSearchAltFolder && wcslen(m_pObj->m_szAltFolder))
	{
		LPCWSTR szExt = L"jpg jpeg gif bmp png";

		CTokensW extensions(szExt, L" ");
		int size = extensions.GetSize();
		for (int i=0; i<size; i++)
		{
			wchar_t szFile[MAX_PATH] = L"";
			swprintf(szFile, L"%s - %s.%s", artist, album, extensions[i]);
			// strip invalid character off the name
			TrimFilename(L"\\/:*?\"<>|", szFile);
			if ((szReturn = FindCover(m_pObj->m_szAltFolder, szFile)) != NULL)
				return szReturn;
		}
	}

	// 2. check the current directory first for bitmaps
	if (m_pObj->m_bSearchFolder)
	{
		if ((szReturn = FindCover(path, ext)) != NULL)
			return szReturn;
	}

	// 3. try the directory above for multi-disc
	if (m_pObj->m_bSearchFolder && m_pObj->m_bMultiDiscFix)
	{
		if (IsMultiDisc())
		{
			CStringArrayW dirs;
			if (GetDiscDirs(path, dirs))
			{
				for (int d=0; d<dirs.GetSize(); d++)
				{
					wchar_t mydir[MAX_PATH];
					wcscpy(mydir, path);
					PathAppendW(mydir, dirs[d]);

					if ((szReturn = FindCover(mydir, ext)) != NULL)
						return szReturn;
				}
			}
		}
		else
		{
			wchar_t *p = PathFindFileNameW(path);
			if (IsFolderMultiDisc(p))
			{
				wchar_t szPath[MAX_PATH] = L"";
				wcsncpy(szPath, path, p-path);

				if ((szReturn = FindCover(szPath, ext)) != NULL)
					return szReturn;
			}
		}
	}

	// 4. try id3v2/itune file ones
	if (m_pObj->m_bSearchMP3)
	{
		if (IsMultiDisc())
		{
			CStringArrayW dirs;
			if (GetDiscDirs(path, dirs))
			{
				for (int d=0; d<dirs.GetSize(); d++)
				{
					wchar_t mydir[MAX_PATH];
					wcscpy(mydir, path);
					PathAppendW(mydir, dirs[d]);

					if ((szReturn = FindCover(mydir, L"*.mp3;*.m4a")) != NULL)
						return szReturn;
				}
			}
		}
		else
		{
			if ((szReturn = FindCover(path, L"*.mp3;*.m4a")) != NULL)
				return szReturn;
		}
	}

	return L"";
}

void CAlbum::SaveThumbnail(LPCWSTR szFile, SIZE size, HBITMAP hBitmap)
{
	HDC		hDC		= NULL;
	LPVOID	lpBits	= NULL;
	FILE*	pFile	= NULL;

	ENSURE
	{
		if ((hDC = GetDC(GetDesktopWindow())) == NULL)
			FAIL;
		
		// get bitmap info header
		BITMAPINFO bmi;
		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		if (!GetDIBits(hDC, hBitmap, 0, 0, NULL, &bmi, DIB_RGB_COLORS))
			FAIL;

		// allocate bits for the bitmap
		lpBits = new char [bmi.bmiHeader.biSizeImage];
		if (lpBits == NULL)
			FAIL;

		// get the bits
		if (!GetDIBits(hDC, hBitmap, 0, bmi.bmiHeader.biHeight, lpBits, &bmi, DIB_RGB_COLORS))
			FAIL;

		// open file for writing
		pFile = _wfopen(szFile, L"wb");
		if (pFile == NULL)
			FAIL;

		// write bitmap file header
        BITMAPFILEHEADER bmfh;
        bmfh.bfType		= 'B'+('M'<<8);
        bmfh.bfOffBits	= sizeof(BITMAPFILEHEADER) + bmi.bmiHeader.biSize;
        bmfh.bfSize		= sizeof(BITMAPFILEHEADER) + bmi.bmiHeader.biSize + bmi.bmiHeader.biSizeImage;
        bmfh.bfReserved1= 0;
		bmfh.bfReserved2= 0;
        fwrite(&bmfh, 1, sizeof(BITMAPFILEHEADER), pFile);

        // write bitmap info header
        fwrite(&bmi.bmiHeader, 1, sizeof(BITMAPINFOHEADER), pFile);

		// write palette entries (if any)
		int nPalEntries = 0;
		if (bmi.bmiHeader.biCompression == BI_BITFIELDS)
			nPalEntries = 3;
		else
			nPalEntries = (bmi.bmiHeader.biBitCount <= 8) ? (int)(1 << bmi.bmiHeader.biBitCount) : 0;

		// Check biClrUsed
		if (bmi.bmiHeader.biClrUsed)
			nPalEntries = bmi.bmiHeader.biClrUsed;

		if (nPalEntries)
			fwrite(bmi.bmiColors, 1, nPalEntries * sizeof(RGBQUAD), pFile);

        // finally, write the image data itself 
        fwrite(lpBits, 1, bmi.bmiHeader.biSizeImage, pFile);
	}
	END_ENSURE;

	// clean up
	if (pFile) fclose(pFile);

	if (lpBits) delete [] lpBits;

	if (hDC) ReleaseDC(GetDesktopWindow(), hDC);

}

HBITMAP	CAlbum::GetCover(SIZE size, BOOL bCache /*=FALSE*/)
{
	LPCWSTR szFile = GetCoverFilename();

	wchar_t szThumb[MAX_PATH];
	wcscpy(szThumb, path);
	PathAppendW(szThumb, L"al.thm");

	HBITMAP hBitmap = NULL;
	
	// -2 to refresh thumbnail
	if (bCache && (GetIconIndex() != -2))
	{
		hBitmap = (HBITMAP)LoadImageW(NULL, szThumb, IMAGE_BITMAP, size.cx, size.cy, LR_LOADFROMFILE);
		if (hBitmap) return hBitmap;
	}

	if (wcslen(szFile))
	{
		LPWSTR ext = PathFindExtensionW(szFile);

		if (wcsicmp(ext, L".mp3") == 0)
		{
			hBitmap = LoadPictureMP3(szFile, size);
		}
		else if (wcsicmp(ext, L".m4a") == 0)
		{
			hBitmap = LoadPictureMP3(szFile, size);
		}
		else if (wcsicmp(ext, L".png") == 0)
		{
			hBitmap = LoadPictureFile2(szFile, size);
		}
		else
		{
			hBitmap = LoadPictureFile(szFile, size);
		}
	}

	if (hBitmap)
	{
		if (bCache) SaveThumbnail(szThumb, size, hBitmap);
		// notify cache is dirty
		if (m_pObj) m_pObj->NotifyCoverChanged();
		return hBitmap;
	}

	return CreateTempCover(size);
}

HBITMAP CAlbum::LoadPictureMP3(LPCWSTR szFile, SIZE size)
{
	HGLOBAL		hGlobal		= NULL;
	LPVOID		pvData		= NULL;
	LPSTREAM	pstm		= NULL;
	LPPICTURE	gpPicture	= NULL;
	LPBYTE		lpCover		= NULL;
	DWORD		dwSize		= 0;
	CFileInfo  *pFileInfo	= NULL;

	ENSURE
	{
		CFileInfo *pFileInfo = CreateFileInfo(szFile);
		if (pFileInfo == NULL)
			FAIL;

		if (pFileInfo->loadInfo(szFile) != 0)
			FAIL;

		if (!pFileInfo->getCoverImage(&lpCover, dwSize))
			FAIL;

		// alloc memory based on file size
		if ((hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwSize)) == NULL)
			FAIL;

		if ((pvData = GlobalLock(hGlobal)) == NULL)
			FAIL;

		// store in global memory
		memcpy(pvData, lpCover, dwSize);

		GlobalUnlock(hGlobal);

		// create IStream* from global memory
		if (FAILED(CreateStreamOnHGlobal(hGlobal, TRUE, &pstm)))
			FAIL;

		// Create IPicture from image file

		if (FAILED(OleLoadPicture(pstm, dwSize, FALSE, IID_IPicture, (LPVOID *)&gpPicture)))
			FAIL;

		pstm->Release();
		pstm = NULL;

		OLE_HANDLE han;
		gpPicture->get_Handle(&han);

		BITMAP bm;
		GetObject((HBITMAP)han, sizeof(bm), &bm);

		// create offscreen surface
		HDC			hDC		= GetDC(GetDesktopWindow());
		HDC			hMem1	= CreateCompatibleDC(hDC);
		HDC			hMem2	= CreateCompatibleDC(hDC);
		HBITMAP		hBitmap	= CreateCompatibleBitmap(hDC, size.cx, size.cy);
		HBITMAP		oBitmap1= (HBITMAP)SelectObject(hMem1, hBitmap);
		HBITMAP		oBitmap2= (HBITMAP)SelectObject(hMem2, (HBITMAP)han);
		int			oBltMode= SetStretchBltMode(hMem1, bWinNT ? HALFTONE : COLORONCOLOR);

		StretchBlt(hMem1, 0, 0, size.cx, size.cy, hMem2, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

		// cleanup
		SetStretchBltMode(hMem1, oBltMode);
		SelectObject(hMem1, oBitmap1);
		SelectObject(hMem2, oBitmap2);
		DeleteDC(hMem1);
		DeleteDC(hMem2);
		ReleaseDC(GetDesktopWindow(), hDC);

		gpPicture->Release();
		gpPicture = NULL;

		GlobalFree(hGlobal);
		delete [] lpCover;
		delete pFileInfo;

		return hBitmap;
	}
	END_ENSURE;

	if (hGlobal)	GlobalFree(hGlobal);
	if (pstm)		pstm->Release();
	if (gpPicture) 	gpPicture->Release();
	if (lpCover)	delete [] lpCover;
	if (pFileInfo)	delete pFileInfo;

	return NULL;
}

HBITMAP CAlbum::CreateTempCover(SIZE size)
{
	HBITMAP hCover = NULL;

	// load overriden default cover
	if (m_pObj->m_bOverrideDefCover)
	{
		LPWSTR ext = PathFindExtensionW(m_pObj->m_szDefaultCover);

		// load custom cover background
		if (wcsicmp(ext, L".png") == 0)
		{
			hCover = LoadPictureFile2 (m_pObj->m_szDefaultCover, size);
		}
		else
		{
			hCover = LoadPictureFile (m_pObj->m_szDefaultCover, size);
		}
	}

	// load built-in cover background
	if (hCover == NULL)
	{
		hCover = LoadBitmap(hDllInstance, MAKEINTRESOURCE(IDB_COVER));
	}

	BITMAP bm;
	GetObject(hCover, sizeof(bm), &bm);

	// create the font
	char *fontname = (char *) wndWinamp.SendIPCMessage(1, IPC_GET_GENSKINBITMAP);
	DWORD charset = (DWORD) wndWinamp.SendIPCMessage(2, IPC_GET_GENSKINBITMAP);
	HFONT hFont = CreateFont(-11, 0, 0, 0, FW_REGULAR, 0, 0, 0, charset, 0/*OUT_TT_PRECIS*/, 0, DRAFT_QUALITY, 0, fontname);

	// create offscreen surface
	HDC			hDC		= GetDC(GetDesktopWindow());
	HDC			hMem1	= CreateCompatibleDC(hDC);
	HDC			hMem2	= CreateCompatibleDC(hDC);
	HBITMAP		hBitmap	= CreateCompatibleBitmap(hDC, size.cx, size.cy);
	HBITMAP		oBitmap1= (HBITMAP)SelectObject(hMem1, hBitmap);
	HBITMAP		oBitmap2= (HBITMAP)SelectObject(hMem2, (HBITMAP)hCover);
	HFONT		oFont	= (HFONT)SelectObject(hMem1, hFont);

	// set drawing modes
	int oBltMode = SetStretchBltMode(hMem1, bWinNT ? HALFTONE : COLORONCOLOR);
	int oBkMode = SetBkMode(hMem1, TRANSPARENT);

	// copy over the cover template
	StretchBlt(hMem1, 0, 0, size.cx, size.cy, hMem2, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

	wchar_t str[MAX_PATH] = L"";
	swprintf(str, L"%s\n%s", artist, album);

	// draw text over cover
	if (m_pObj->m_bDrawTitle)
	{
		// draw shadow
		RECT rc = { 5, size.cy / 2 - 15, size.cx - 5, size.cy / 2 + 15 };
		if (m_pObj->m_bCoverShadow)
		{
			SetTextColor(hMem1, RGB(0, 0, 0));
			DrawTextW(hMem1, str, wcslen(str), &rc, DT_NOPREFIX|DT_CENTER);
		}

		// draw the artist/album name onto the template
		OffsetRect(&rc, -1, -1);
		SetTextColor(hMem1, m_pObj->m_clrCoverText);
		DrawTextW(hMem1, str, wcslen(str), &rc, DT_NOPREFIX|DT_CENTER);
	}

	// cleanup
	SetBkMode(hMem1, oBkMode);
	SetStretchBltMode(hMem1, oBltMode);
	SelectObject(hMem1, oFont);
	SelectObject(hMem1, oBitmap1);
	SelectObject(hMem2, oBitmap2);
	DeleteDC(hMem1);
	DeleteDC(hMem2);
	ReleaseDC(GetDesktopWindow(), hDC);
	DeleteObject(hFont);
	DeleteObject(hCover);

	return hBitmap;
}

//////////////////////////////////////////////////////////////////
// CTrackArray class

BOOL CTrackArray::m_bDuplicate = FALSE;
int CTrackArray::m_nSortName = 0;

CTrackArray::CTrackArray() : CPtrArray()
{
	m_pCompare = Compare;
}

CTrackArray::~CTrackArray()
{
}

int CTrackArray::Add(LPTRACKINFO pTrackInfo)
{
	return CPtrArray::Add(pTrackInfo);
}

LPTRACKINFO CTrackArray::operator[](int nIndex)
{
	return (LPTRACKINFO)GetAt(nIndex);
}

BOOL CTrackArray::Sort(int sortname)
{
	m_bDuplicate = FALSE;
	m_nSortName = sortname;

	CPtrArray::Sort();

	return m_bDuplicate;
}

int CTrackArray::Compare(const void *elem1, const void *elem2)
{
	LPTRACKINFO pTrackInfo1 = *(LPTRACKINFO*)elem1;
	LPTRACKINFO pTrackInfo2 = *(LPTRACKINFO*)elem2;

	if (!m_nSortName)
	{
		if (pTrackInfo1->nDisc < pTrackInfo2->nDisc) return -1;
		if (pTrackInfo1->nDisc > pTrackInfo2->nDisc) return 1;
		if (pTrackInfo1->nTrack < pTrackInfo2->nTrack) return -1;
		if (pTrackInfo1->nTrack > pTrackInfo2->nTrack) return 1;
		m_bDuplicate = TRUE;
	}
	return wcscmp(pTrackInfo1->path, pTrackInfo2->path);
}

//////////////////////////////////////////////////////////////////
// CTrackExArray class

int CTrackExArray::Add(LPTRACKINFOEX pTrackInfo)
{
	return CPtrArray::Add(pTrackInfo);
}

LPTRACKINFOEX CTrackExArray::operator[](int nIndex)
{
	return (LPTRACKINFOEX)GetAt(nIndex);
}

//////////////////////////////////////////////////////////////////
// CPictureArray class

CPictureArray::CPictureArray() : CPtrArray()
{
	m_pCompare = Compare;
}

CPictureArray::~CPictureArray()
{
}

int CPictureArray::Add(LPPICTUREINFO pPicInfo)
{
	return CPtrArray::Add(pPicInfo);
}

LPPICTUREINFO CPictureArray::operator[](int nIndex)
{
	return (LPPICTUREINFO)GetAt(nIndex);
}

int CPictureArray::Compare(const void *elem1, const void *elem2)
{
	LPPICTUREINFO pPicInfo1 = *(LPPICTUREINFO*)elem1;
	LPPICTUREINFO pPicInfo2 = *(LPPICTUREINFO*)elem2;

	if (pPicInfo1->nType < pPicInfo2->nType) return -1;
	if (pPicInfo1->nType > pPicInfo2->nType) return 1;

	return 0;
}
