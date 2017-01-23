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
#ifndef __ALBUM_H__
#define __ALBUM_H__

#include "FileInfo.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////
// ALBUMINFO struct
//
// stores album information

typedef struct _TAGALBUMINFO
{
	wchar_t name[MAX_PATH];		// display name (combination of artist and album)
	wchar_t album[MAX_PATH];	// album name
	wchar_t artist[MAX_PATH];	// artist name
	wchar_t path[MAX_PATH];		// directory path
	wchar_t m3u_file[MAX_PATH];	// pathname of playlist (if exist)
	wchar_t genre[MAX_PATH];	// genre
	int  nTotalTime;			// total time in the directory (or playlist)
	int  nCount;				// number of files in this directory (or playlist)
	int  nYear;					// year of this album
	DWORD dwFlags;				// flags
	GUID id;					// ID of the album
	int  nPathLen;
	int	 nIconIndex;

	DWORD dwReserved1;
	DWORD dwReserved2;

#define AI_CDROM		0x00000001	// this is on CD-ROM
#define AI_SHORTCUT		0x00000002	// this is a shortcut (*.lnk)
#define AI_MULTIDISC	0x00000004	// this is a multi-disc album
#define AI_USERMODIFIED	0x00000008	// user modified the attribute (do not overwrite)

}	ALBUMINFO, *LPALBUMINFO;

#define ALBUMINFO_VER		sizeof(ALBUMINFO)

// compress used in the cache file
#define AL_UNCOMPRESS		0
#define AL_RLE				1
#define AL_PER_ALBUM_RLE	2

class CAlbumList;
class CAlbum : public ALBUMINFO
{
	friend CAlbumList;

public:
	CAlbum(CAlbumList *pObj);
	virtual ~CAlbum();

	int  AddRef				();
	int  Release			();

	// controls
	BOOL AddDir				(LPCWSTR dir, int nLen);
	BOOL AddM3U				(LPCWSTR path, int nLen, LPCWSTR filename = NULL);
	BOOL AddTrack			(LPCWSTR path, LPCWSTR filename = NULL);

	WORD IsOutdated			();
#define IOD_UPTODATE		0
#define IOD_REMOVE			1
#define	IOD_REFRESH			2

	BOOL Play				();
	BOOL Enqueue			();
	BOOL EnqueueAndPlay		();
	BOOL EnqueueAfterCurrent();
	BOOL WritePlaylist		();
	BOOL ShowCover			();
	BOOL ShowInfo			(CWnd wnd);
	BOOL Refresh			();
	BOOL Explore			();
	BOOL LibrarySendTo		(librarySendToMenuStruct s);

	// archive
	BOOL Load				(HANDLE fp, BYTE bComp);
	BOOL Save				(HANDLE fp, BYTE bComp);

	// get attributes
	LPCWSTR GetPath			();
	LPCWSTR GetM3U			();
	LPCWSTR GetTitle		();
	LPCWSTR GetArtist		();
	LPCWSTR GetAlbum		();
	LPCWSTR GetGenre		();
	int		GetTotalTime	();
	int		GetTrackCount	();
	int		GetYear			();
	BOOL	IsPlaylistBased	();
	BOOL	IsMultiDisc		();
	int		GetNumberOfDiscs();
	void	GetFileList		(CStringArrayW &files);

	// set attributes
	void	SetTitle		(LPCWSTR szTitle);
	void	SetArtist		(LPCWSTR szArtist);
	void	SetAlbum		(LPCWSTR szAlbum);
	void	SetYear			(int year);
	void	SetGenre		(LPCWSTR szGenre);

	int		GetIconIndex	();
	void	SetIconIndex	(int index);

	LPCWSTR	GetCoverFilename();
	LPCWSTR FindCover		(LPCWSTR szPath, LPCWSTR szExt);
	HBITMAP	GetCover		(SIZE size, BOOL bCache = FALSE);
	HBITMAP LoadPictureMP3	(LPCWSTR szFile, SIZE size);
	HBITMAP CreateTempCover	(SIZE size);
	void	SaveThumbnail	(LPCWSTR szFile, SIZE size, HBITMAP hBitmap);

private:
	BOOL PlayDir			();
	BOOL PlayM3U			();
	BOOL WritePlaylist		(BOOL bFullPath, BOOL bInfo, LPCWSTR szOutput = NULL);

	void GenerateDisplayName(LPCWSTR path);
	void GenerateArtistName	();
	void GenerateAlbumName	();
	void GenerateGenre		();

	BOOL GetDiscDirs		(LPCWSTR str, CStringArrayW &dirs);
	void FixTitle			(LPWSTR str);
	BOOL IsFolderMultiDisc	(LPCWSTR str);
	BOOL IsPartOfMultiDisc	();
	void TrimWhiteSpace		(LPWSTR str);
	void TrimYear			(LPWSTR strOut, LPCWSTR strIn);
	BOOL FixMultiDiscAlbumName(LPWSTR str);

private:
	CAlbumList				*m_pObj;
	CMapStringToStringW		 m_VariousArtist;
	CMapStringToStringW		 m_MiscAlbum;
	CMapStringToStringW		 m_MixedGenre;
	wchar_t					 m_szCover[MAX_PATH];
	int						 m_nRef;
};

//////////////////////////////////////////////////////////////////

typedef struct _TAGTRACKINFO
{
	wchar_t path[MAX_PATH];		// full path of the track
	int  nTrack;				// track number
	int  nDisc;					// disc number

}	TRACKINFO, *LPTRACKINFO;

//////////////////////////////////////////////////////////////////
// CTrackArray class

class CTrackArray : public CPtrArray
{
public:
	CTrackArray();
	virtual ~CTrackArray();

public:
	int Add(LPTRACKINFO pTrackInfo);

	LPTRACKINFO operator[](int nIndex);

	BOOL Sort(int sortname);
	static int Compare(const void *elem1, const void *elem2);

protected:
	static BOOL m_bDuplicate;
	static int	m_nSortName;
};

//////////////////////////////////////////////////////////////////

typedef struct _TAGTRACKINFOEX : public _TAGTRACKINFO
{
	wchar_t desc[MAX_PATH];		// #EXTINF:...

}	TRACKINFOEX, *LPTRACKINFOEX;

//////////////////////////////////////////////////////////////////
// CTrackExArray class

class CTrackExArray : public CTrackArray
{
public:
	int Add(LPTRACKINFOEX pTrackInfo);

	LPTRACKINFOEX operator[](int nIndex);
};

//////////////////////////////////////////////////////////////////
// CPictureArray class

typedef struct _TAGPICTUREINFO
{
	wchar_t file[MAX_PATH*2];		// full path of the track
	int  nType;					// type of file, 0 - back, 

}	PICTUREINFO, *LPPICTUREINFO;

class CPictureArray : public CPtrArray
{
public:
	CPictureArray();
	virtual ~CPictureArray();

public:
	int Add(LPPICTUREINFO pPicInfo);

	LPPICTUREINFO operator[](int nIndex);

	static int Compare(const void *elem1, const void *elem2);
};

#endif /* __ALBUM_H__ */
