#ifndef _wmainfo_h_
#define _wmainfo_h_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <objbase.h>
#include "fileinfo.h"

#define ENSURE			do
#define FAIL			break
#define END_ENSURE		while (FALSE)

GUID ASF_Header_Object						= { 0x75B22630, 0x668E, 0x11CF, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C };
GUID ASF_File_Properties_Object				= { 0x8CABDCA1, 0xA947, 0x11CF, 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 };
GUID ASF_Content_Description_Object			= { 0x75B22633, 0x668E, 0x11CF, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C };
GUID ASF_Extended_Content_Description_Object= { 0xD2D0A440, 0xE307, 0x11D2, 0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50 };

class CWMAInfo : public CFileInfo {

    public:

    // function to load a file into this structure
    // the argument passed is the path to a WMA file
    int   loadInfo( const char* srcWMA );
    int   loadInfo( const wchar_t* srcWMA );

    // functions used to get information about the "file"
    int   getFileSize() { return fileSize; };
    void  getFileName(char* input);

    // functions to calculate the length of the song
    // and to present it nicely
    int   getLengthInSeconds();
    void  getFormattedLength(char* input);

    // information that is avaliable in the ID3 tag
    void  getTitle(char* input);
    void  getArtist(char* input);
    void  getAlbumArtist(char* input);
    void  getAlbum(char* input);
    void  getComment(char* input);
    void  getGenre(char* input);

    int   getYear();
    int   getTrackNumber();
	int   getDiscNumber();

    void  getTitle(wchar_t* input);
    void  getArtist(wchar_t* input);
    void  getAlbumArtist(wchar_t* input);
    void  getAlbum(wchar_t* input);
    void  getComment(wchar_t* input);
    void  getGenre(wchar_t* input);

    private:

	ULONGLONG ullDuration;

	BOOL ReadFilePropertiesObject(HANDLE fp);
	BOOL ReadContentDescriptionObject(HANDLE fp);
	BOOL ReadExtendedContentDescriptionObject(HANDLE fp);

    // the file information can not be found elsewhere
    wchar_t fileName[260];
    int fileSize;

	wchar_t artist[MAX_PATH];
	wchar_t albumartist[MAX_PATH];
	wchar_t title[MAX_PATH];
	wchar_t album[MAX_PATH];
	wchar_t genre[MAX_PATH];

	int track;
	int year;
	int discNumber;
};

BOOL CWMAInfo::ReadFilePropertiesObject(HANDLE fp)
{
	ENSURE
	{
		DWORD nBytesRead;

		// skip some fields
		// *File ID*				128 (16 bytes)
		// *File Size*				64 (8 bytes)
		// *Creation Date*			64 (8 bytes)
		// *Data Packets Count*		64 (8 bytes)
		SetFilePointer(fp, 16+8+8+8, NULL, FILE_CURRENT);

		ullDuration = 0;
		if (!ReadFile(fp, &ullDuration, sizeof(ULONGLONG), &nBytesRead, NULL)) FAIL;

		// skip some more fields
		// *Send Duration*			64 (8 bytes)
		SetFilePointer(fp, 8, NULL, FILE_CURRENT);

		ULONGLONG ullPreroll = 0;
		if (!ReadFile(fp, &ullPreroll, sizeof(ULONGLONG), &nBytesRead, NULL)) FAIL;

		ullDuration -= ullPreroll * 10000;

		DWORD dwFlags = 0;
		if (!ReadFile(fp, &dwFlags, sizeof(DWORD), &nBytesRead, NULL)) FAIL;

		if (dwFlags & 0x1) ullDuration = 0;
		
		return TRUE;
	}
	END_ENSURE;

	return FALSE;
}

BOOL CWMAInfo::ReadContentDescriptionObject(HANDLE fp)
{
	PWCHAR pwcTitle = NULL;
	PWCHAR pwcAuthor = NULL;

	ENSURE
	{
		DWORD nBytesRead;

		WORD wTitleLen = 0;
		if (!ReadFile(fp, &wTitleLen, sizeof(WORD), &nBytesRead, NULL)) FAIL;

		WORD wAuthorLen = 0;
		if (!ReadFile(fp, &wAuthorLen, sizeof(WORD), &nBytesRead, NULL)) FAIL;

		// skip some fields
		// *Copyright  Length*		16 (2 bytes)
		// *Description Length*		16 (2 bytes)
		// *Rating Length*			16 (2 bytes)
		SetFilePointer(fp, 2+2+2, NULL, FILE_CURRENT);

		pwcTitle = new WCHAR [wTitleLen];
		if (!ReadFile(fp, pwcTitle, wTitleLen, &nBytesRead, NULL)) FAIL;

		pwcAuthor = new WCHAR [wAuthorLen];
		if (!ReadFile(fp, pwcAuthor, wAuthorLen, &nBytesRead, NULL)) FAIL;

		if (wcslen(title) == 0) wcsncpy(title, pwcTitle, MAX_PATH);
		if (wcslen(artist) == 0) wcsncpy(artist, pwcAuthor, MAX_PATH);

		delete [] pwcTitle;
		delete [] pwcAuthor;

		return TRUE;
	}
	END_ENSURE;

	if (pwcTitle) delete [] pwcTitle;
	if (pwcAuthor) delete [] pwcAuthor;

	return FALSE;
}

BOOL CWMAInfo::ReadExtendedContentDescriptionObject(HANDLE fp)
{
	PWCHAR pwcName = NULL;
	PWCHAR pwcValue = NULL;
	LPTSTR szName = NULL;

	ENSURE
	{
		DWORD nBytesRead;

		// get number of header objects
		WORD wNumOfDescriptors = 0;
		if (!ReadFile(fp, &wNumOfDescriptors, sizeof(WORD), &nBytesRead, NULL)) FAIL;

		for (WORD i=0; i<wNumOfDescriptors; i++)
		{
			WORD wNameLen = 0;
			if (!ReadFile(fp, &wNameLen, sizeof(WORD), &nBytesRead, NULL)) FAIL;

			pwcName = new WCHAR [wNameLen];
			if (!ReadFile(fp, pwcName, wNameLen, &nBytesRead, NULL)) FAIL;

			szName = new char [wNameLen];
			WideCharToMultiByte(CP_ACP, 0, pwcName, -1, szName, MAX_PATH, NULL, NULL);

			WORD wValueType = 0;
			if (!ReadFile(fp, &wValueType, sizeof(WORD), &nBytesRead, NULL)) FAIL;

			WORD wValueLen = 0;
			if (!ReadFile(fp, &wValueLen, sizeof(WORD), &nBytesRead, NULL)) FAIL;

			pwcValue = new WCHAR [wValueLen];
			if (!ReadFile(fp, pwcValue, wValueLen, &nBytesRead, NULL)) FAIL;

			// ID3 tag				Windows Media tag 
			// -------				-----------------
			// Album				WM/AlbumTitle 
			// ContentType			WM/Genre 
			// Copyright			WM/Copyright 
			// LeadArtist			WM/Author 
			// TrackNum				WM/Track or WM/TrackNumber
			// Title				WM/Title 
			// Year					WM/Year 
			// Comment				WM/Description 
			// MusicCDIdentifier	WM/MCDI 
			//
			// more detail info on id3v2 to WM tag mapping can be found on msdn
			// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wmform/htm/id3support.asp
			if (wValueType == 0x0000)		// Unicode
			{
				if (lstrcmp(szName, "WM/Author") == 0)
					wcsncpy(artist, pwcValue, MAX_PATH);
				else if (lstrcmp(szName, "WM/AlbumArtist") == 0)
					wcsncpy(albumartist, pwcValue, MAX_PATH);
				else if (lstrcmp(szName, "WM/AlbumTitle") == 0)
					wcsncpy(album, pwcValue, MAX_PATH);
				else if (lstrcmp(szName, "WM/Title") == 0)
					wcsncpy(title, pwcValue, MAX_PATH);
				else if (lstrcmp(szName, "WM/Year") == 0)
					year = _wtoi(pwcValue);
				else if (lstrcmp(szName, "WM/PartOfSet") == 0)
					discNumber = _wtoi(pwcValue);
				else if (lstrcmp(szName, "WM/Genre") == 0)
					wcsncpy(genre, pwcValue, MAX_PATH);
				else if (lstrcmp(szName, "WM/GenreID") == 0)
				{
					wcsncpy(genre, pwcValue, MAX_PATH);

					// convert id3v1.1 genre number to string
					wchar_t *str = genre;
					if (str[0] == L'(') 
					{
						wchar_t *ptr = str;
						ptr = wcschr(ptr, L')');
						if (ptr) *ptr = 0;
						UCHAR nGenre = _wtoi(str+1);

						// this table of constant strings will be used in all cases..
						const char* table[148] = {
												  "Blues","Classic Rock","Country","Dance","Disco","Funk","Grunge","Hip-Hop","Jazz",
												  "Metal","New Age","Oldies","Other","Pop","R&B","Rap","Reggae","Rock","Techno",
												  "Industrial","Alternative","Ska","Death Metal","Pranks","Soundtrack","Euro-Techno",
												  "Ambient","Trip-Hop","Vocal","Jazz+Funk","Fusion","Trance","Classical","Instrumental",
												  "Acid","House","Game","Sound Clip","Gospel","Noise","Alt. Rock","Bass","Soul","Punk",
												  "Space","Meditative","Instrumental Pop","Instrumental Rock","Ethnic","Gothic",
												  "Darkwave","Techno-Industrial","Electronic","Pop-Folk","Eurodance","Dream",
												  "Southern Rock","Comedy","Cult","Gangsta","Top 40","Christian Rap","Pop/Funk","Jungle",
												  "Native American","Cabaret","New Wave","Psychadelic","Rave","Showtunes","Trailer",
												  "Lo-Fi","Tribal","Acid Punk","Acid Jazz","Polka","Retro","Musical","Rock & Roll",
												  "Hard Rock","Folk","Folk-Rock","National Folk","Swing","Fast Fusion","Bebob","Latin",
												  "Revival","Celtic","Bluegrass","Avantgarde","Gothic Rock","Progressive Rock",
												  "Psychedelic Rock","Symphonic Rock","Slow Rock","Big Band","Chorus","Easy Listening",
												  "Acoustic","Humour","Speech","Chanson","Opera","Chamber Music","Sonata","Symphony",
												  "Booty Bass","Primus","Porn Groove","Satire","Slow Jam","Club","Tango","Samba",
												  "Folklore","Ballad","Power Ballad","Rhythmic Soul","Freestyle","Duet","Punk Rock",
												  "Drum Solo","Acapella","Euro-House","Dance Hall","Goa","Drum & Bass","Club-House",
												  "Hardcore","Terror","Indie","BritPop","Negerpunk","Polsk Punk","Beat",
												  "Christian Gangsta Rap","Heavy Metal","Black Metal","Crossover","Contemporary Christian",
												  "Christian Rock","Merengue","Salsa","Thrash Metal","Anime","Jpop","Synthpop" 
												 };

						if (nGenre>=148)	wcscpy(genre, L"");
						else				wcscpy(genre, AutoWide(table[nGenre]));
					}
				}
					
			}
			else if (wValueType == 0x0003)	// DWORD
			{
				DWORD dwValue = *(DWORD *)pwcValue;
				if ((lstrcmp(szName, "WM/Track") == 0) ||
					(lstrcmp(szName, "WM/TrackNumber") == 0))
					track = dwValue;
				if (lstrcmp(szName, "WM/PartOfSet") == 0)
					discNumber = dwValue;
			}

			delete [] szName;
			delete [] pwcName;
			delete [] pwcValue;
			szName = NULL;
			pwcName = pwcValue = NULL;
		}

		return TRUE;
	}
	END_ENSURE;

	if (szName) delete [] szName;
	if (pwcName) delete [] pwcName;
	if (pwcValue) delete [] pwcValue;

	return FALSE;
}

int CWMAInfo::loadInfo ( const char* srcWMA )
{
	return loadInfo (AutoWide(srcWMA));
}

int CWMAInfo::loadInfo ( const wchar_t* srcWMA )
{
	int ret = 1;

	// initialize some variables
	ullDuration = 0;
	track = 0;
	year = 0;
	discNumber = 0;
	memset(title, 0, sizeof(title));
	memset(artist, 0, sizeof(artist));
	memset(albumartist, 0, sizeof(albumartist));
	memset(album, 0, sizeof(album));
	memset(genre, 0, sizeof(genre));

	wcscpy(fileName, srcWMA);

	HANDLE fp = NULL;

	ENSURE
	{
		DWORD nBytesRead;

		if ((fp = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE) FAIL;

		fileSize = GetFileSize (fp, NULL);

		GUID guid;
		if (!ReadFile(fp, &guid, sizeof(GUID), &nBytesRead, NULL)) FAIL;

		// make sure it has the asf header object
		if (!IsEqualGUID(ASF_Header_Object, guid)) FAIL;

		// get size of asf header object
		ULONGLONG ullSize = 0;
		if (!ReadFile(fp, &ullSize, sizeof(ULONGLONG), &nBytesRead, NULL)) FAIL;

		// valid size is bigger than 30
		if (ullSize <= 30) FAIL;

		// get number of header objects
		DWORD dwNumOfHeaders = 0;
		if (!ReadFile(fp, &dwNumOfHeaders, sizeof(DWORD), &nBytesRead, NULL)) FAIL;

		// get the reserved
		BYTE bReserved;
		if (!ReadFile(fp, &bReserved, sizeof(BYTE), &nBytesRead, NULL)) FAIL;
		if (!ReadFile(fp, &bReserved, sizeof(BYTE), &nBytesRead, NULL)) FAIL;

		// read all the headers
		int iRet = 1;
		for (DWORD i=0; i<dwNumOfHeaders; i++)
		{
			DWORD dwCurrentFilePosition = SetFilePointer(fp, 0, NULL, FILE_CURRENT);

			// read property object id
			if (!ReadFile(fp, &guid, sizeof(GUID), &nBytesRead, NULL)) FAIL;

			// get size of property object
			ULONGLONG ullSize = 0;
			if (!ReadFile(fp, &ullSize, sizeof(ULONGLONG), &nBytesRead, NULL)) FAIL;


			if (IsEqualGUID(ASF_File_Properties_Object, guid))
			{
				if (ReadFilePropertiesObject(fp)) iRet = 0;
			}
			else if (IsEqualGUID(ASF_Content_Description_Object, guid))
			{
				if (ReadContentDescriptionObject(fp)) iRet = 0;
			}
			else if (IsEqualGUID(ASF_Extended_Content_Description_Object, guid))
			{
				if (ReadExtendedContentDescriptionObject(fp)) iRet = 0;
			}

			SetFilePointer(fp, dwCurrentFilePosition + (DWORD)ullSize, NULL, FILE_BEGIN);
		}

		CloseHandle(fp);

		return iRet;
	}
	END_ENSURE;

	if (fp) CloseHandle(fp);

	return 1;
}

void CWMAInfo::getFileName(char* input) {

    strcpy(input, AutoChar(fileName));

}

int CWMAInfo::getLengthInSeconds()
{
	return (int)(ullDuration / 10000000);
}

void CWMAInfo::getFormattedLength(char* input) {

    //  s  = complete number of seconds
    int s  = getLengthInSeconds();

    //  ss = seconds to display
    int ss = s%60;

    //  m  = complete number of minutes
    int m  = (s-ss)/60;

    //  mm = minutes to display
    int mm = m%60;

    //  h  = complete number of hours
    int h = (m-mm)/60;

    char szTime[16]; // temporary string

    // make a "hh:mm:ss" if there is any hours, otherwise
    // make it   "mm:ss"
    if (h>0) sprintf(szTime,"%02d:%02d:%02d", h,mm,ss);
    else     sprintf(szTime,     "%02d:%02d",   mm,ss);

    // copy to the inputstring
    strcpy(input, szTime);

}

void CWMAInfo::getTitle(char* input)
{
    // copy result into inputstring
    strcpy(input, AutoChar(title, wndWinampAL.GetEncodingCodepage()));
}

void CWMAInfo::getArtist(char* input)
{
    strcpy(input, AutoChar(artist, wndWinampAL.GetEncodingCodepage()));
}

void CWMAInfo::getAlbumArtist(char* input)
{
    strcpy(input, AutoChar(albumartist, wndWinampAL.GetEncodingCodepage()));
}

void CWMAInfo::getAlbum(char* input)
{
    strcpy(input, AutoChar(album, wndWinampAL.GetEncodingCodepage()));
}

void CWMAInfo::getComment(char* input)
{
    //strcpy(input, (char *)tag.Comment);
}

void CWMAInfo::getGenre(char* input)
{
    strcpy(input, AutoChar(genre, wndWinampAL.GetEncodingCodepage()));
}

void CWMAInfo::getTitle(wchar_t* input)
{
    wcscpy(input, title);
}

void CWMAInfo::getArtist(wchar_t* input)
{
    wcscpy(input, artist);
}

void CWMAInfo::getAlbumArtist(wchar_t* input)
{
    wcscpy(input, albumartist);
}

void CWMAInfo::getAlbum(wchar_t* input)
{
    wcscpy(input, album);
}

void CWMAInfo::getComment(wchar_t* input)
{
    //strcpy(input, (char *)tag.Comment);
}

void CWMAInfo::getGenre(wchar_t* input)
{
    wcscpy(input, genre);
}

int CWMAInfo::getYear()
{
    return year;
}

int CWMAInfo::getTrackNumber()
{
    return track;
}

int CWMAInfo::getDiscNumber()
{
    return discNumber;
}

#endif
