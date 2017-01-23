#ifndef _CWAEXTENDEDINFO_H_
#define _CWAEXTENDEDINFO_H_

#include <windows.h>
#include "fileinfo.h"
#include "Gen_m3a.h"

class CWAExtendedInfo : public CFileInfo
{
public:
    
    // function to load a file into this structure
    // the argument passed is the path to a file
    virtual int   loadInfo( const char* srcFile );
    virtual int   loadInfo( const wchar_t* srcFile );

    // functions used to get information about the "file"
    virtual int   getFileSize() { return m_FileSize; };
    virtual void  getFileName(char* input) { lstrcpy(input, AutoChar(m_FileName)); };
    
    // functions to calculate the length of the song
    // and to present it nicely
	virtual int   getLengthInSeconds() {return m_LengthInSeconds;}
    virtual void  getFormattedLength(char* input);
 
    // This information is not available in wav files:
    virtual void  getTitle(char* input);
    virtual void  getArtist(char* input);
    virtual void  getAlbumArtist(char* input) { input[0] = 0;}
    virtual void  getAlbum(char* input);
    virtual void  getComment(char* input)	{input[0] = 0;}
    virtual void  getGenre(char* input);

    virtual int   getYear();
    virtual int   getTrackNumber();
	virtual int   getDiscNumber() { return 0; };

    virtual void  getTitle(wchar_t* input);
    virtual void  getArtist(wchar_t* input);
    virtual void  getAlbum(wchar_t* input);
    virtual void  getComment(wchar_t* input) {input[0] = 0;}
    virtual void  getGenre(wchar_t* input);

public:
    wchar_t m_FileName[260];

private:
    // the file information can not be found elsewhere
    wchar_t m_Artist[260];
    wchar_t m_Album[260];
	wchar_t m_Title[260];
	wchar_t m_Genre[260];
	int m_Year;
	int m_Track;
    int m_FileSize;
	int m_LengthInSeconds;

	BOOL getMetadata(wchar_t *metadata, wchar_t *input);
};

int CWAExtendedInfo::loadInfo(const char *srcFile)
{
	return loadInfo (AutoWide(srcFile));
}

int CWAExtendedInfo::loadInfo(const wchar_t *srcFile)
{
	if (srcFile == NULL) return 1;

	//initialize
	m_LengthInSeconds = 0;
	memset(m_Title, 0, sizeof(m_Title));
	memset(m_Artist, 0, sizeof(m_Artist));
	memset(m_Album, 0, sizeof(m_Album));
	memset(m_Genre, 0, sizeof(m_Genre));
	m_Year = 0;
	m_Track = 0;

	wcsncpy(m_FileName, srcFile, 260);

	int iRet = 1;

	// get track length
	wchar_t len[256];
	if (getMetadata(L"Length", len))
	{
		m_LengthInSeconds = _wtoi(len) / 1000;
		iRet = 0;
	}

    return iRet;
}

BOOL CWAExtendedInfo::getMetadata(wchar_t *metadata, wchar_t *input)
{
	if (wndWinamp.IsWinamp513())
	{
		wchar_t retvalue[256];
		memset(retvalue, 0, sizeof(retvalue));
		extendedFileInfoStructW einfo;
		memset(&einfo, 0, sizeof(extendedFileInfoStructW));
		einfo.filename = m_FileName;
		einfo.metadata = metadata;
		einfo.ret = retvalue;
		einfo.retlen = 256;
		if (wndWinamp.SendIPCMessage((WPARAM)&einfo, IPC_GET_EXTENDED_FILE_INFOW))
		{
			wcscpy(input, einfo.ret);
			return TRUE;
		}
	}
	else
	{
		char retvalue[256];
		memset(retvalue, 0, sizeof(retvalue));
		extendedFileInfoStruct einfo;
		memset(&einfo, 0, sizeof(extendedFileInfoStruct));
		AutoChar name(m_FileName);
		AutoChar data(metadata);
		einfo.filename = name;
		einfo.metadata = data;
		einfo.ret = retvalue;
		einfo.retlen = 256;
		if (wndWinamp.SendIPCMessage((WPARAM)&einfo, IPC_GET_EXTENDED_FILE_INFO))
		{
			wcscpy(input, AutoWide(einfo.ret, wndWinampAL.GetEncodingCodepage()));
			return TRUE;
		}
	}
	return FALSE;
}

void CWAExtendedInfo::getTitle(char* input)
{
	if (wcslen(m_Title) == 0)
	{
		getMetadata(L"Title", m_Title);
	}
	lstrcpy(input, AutoChar(m_Title, wndWinampAL.GetEncodingCodepage()));
}

void CWAExtendedInfo::getArtist(char* input)
{
	if (wcslen(m_Artist) == 0)
	{
		getMetadata(L"Artist", m_Artist);
	}
	lstrcpy(input, AutoChar(m_Artist, wndWinampAL.GetEncodingCodepage()));
}

void CWAExtendedInfo::getAlbum(char* input)
{
	if (wcslen(m_Album) == 0)
	{
		getMetadata(L"Album", m_Album);
	}
	lstrcpy(input, AutoChar(m_Album, wndWinampAL.GetEncodingCodepage()));
}

void CWAExtendedInfo::getGenre(char* input)
{
	if (wcslen(m_Genre) == 0)
	{
		getMetadata(L"Genre", m_Genre);
	}
	lstrcpy(input, AutoChar(m_Genre, wndWinampAL.GetEncodingCodepage()));
}

void CWAExtendedInfo::getTitle(wchar_t* input)
{
	if (wcslen(m_Title) == 0)
	{
		getMetadata(L"Title", m_Title);
	}
	wcscpy(input, m_Title);
}

void CWAExtendedInfo::getArtist(wchar_t* input)
{
	if (wcslen(m_Artist) == 0)
	{
		getMetadata(L"Artist", m_Artist);
	}
	wcscpy(input, m_Artist);
}

void CWAExtendedInfo::getAlbum(wchar_t* input)
{
	if (wcslen(m_Album) == 0)
	{
		getMetadata(L"Album", m_Album);
	}
	wcscpy(input, m_Album);
}

void CWAExtendedInfo::getGenre(wchar_t* input)
{
	if (wcslen(m_Genre) == 0)
	{
		getMetadata(L"Genre", m_Genre);
	}
	wcscpy(input, m_Genre);
}

int CWAExtendedInfo::getYear()
{
	if (m_Year == 0)
	{
		wchar_t year[256];
		memset(year, 0, sizeof(year));
		if (getMetadata(L"Year", year))
		{
			m_Year = _wtoi(year);
		}
	}
	return m_Year;
}

int CWAExtendedInfo::getTrackNumber()
{
	if (m_Track == 0)
	{
		wchar_t track[256];
		if (getMetadata(L"Track", track))
		{
			m_Track = _wtoi(track);
		}
	}
	return m_Track;
}

void CWAExtendedInfo::getFormattedLength(char* input)
{
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
#endif
