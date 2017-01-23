#ifndef __FILEINFO_H__
#define __FILEINFO_H__

#include "AutoWide.h"

class CFileInfo
{
public:
	CFileInfo() {};
	virtual ~CFileInfo() {};

    // function to load a file into this structure
    // the argument passed is the path to a MP3 file
    virtual int   loadInfo( const char* src ) = 0;

    // functions used to get information about the "file"
    virtual int   getFileSize() = 0;
    virtual void  getFileName(char* input) = 0;

    // functions to calculate the length of the song
    // and to present it nicely
    virtual int   getLengthInSeconds() = 0;
    virtual void  getFormattedLength(char* input) = 0;

    // information that is avaliable in the ID3 tag
    virtual void  getTitle(char* input) = 0;
    virtual void  getArtist(char* input) = 0;
    virtual void  getAlbumArtist(char* input) = 0;
    virtual void  getAlbum(char* input) = 0;
    virtual void  getComment(char* input) = 0;
    virtual void  getGenre(char* input) = 0;

    virtual int   getYear() = 0;
    virtual int   getTrackNumber() = 0;
	virtual int   getDiscNumber() = 0;

	virtual bool  getCoverImage(LPBYTE* input, DWORD &dwSize) { return false; };

	// unicode versions
    virtual int   loadInfo( const wchar_t* src ) = 0;
    virtual void  getTitle(wchar_t* input)
	{
		char str[256];
		getTitle(str);
		wcscpy(input, AutoWide(str));
	}
    virtual void  getArtist(wchar_t* input)
	{
		char str[256];
		getArtist(str);
		wcscpy(input, AutoWide(str));
	}
    virtual void  getAlbumArtist(wchar_t* input)
	{
		char str[256];
		getAlbumArtist(str);
		wcscpy(input, AutoWide(str));
	}
    virtual void  getAlbum(wchar_t* input)
	{
		char str[256];
		getAlbum(str);
		wcscpy(input, AutoWide(str));
	}
    virtual void  getComment(wchar_t* input)
	{
		char str[256];
		getComment(str);
		wcscpy(input, AutoWide(str));
	}
    virtual void  getGenre(wchar_t* input)
	{
		char str[256];
		getGenre(str);
		wcscpy(input, AutoWide(str));
	}
};

CFileInfo *CreateFileInfo		(LPCTSTR szFile);
CFileInfo *CreateFileInfo		(LPCWSTR szFile);
CFileInfo *CreateWAExtendedInfo	();
void AddFileReader				(LPCTSTR reader);
void LoadExternalFileReader		();
void UnloadExternalFileReader	();

#endif /* __FILEINFO_H__ */
