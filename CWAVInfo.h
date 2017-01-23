// CWAVInfo.h (c) Andreas Andersson
#ifndef _CWAVINFO_H_
#define _CWAVINFO_H_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <objbase.h>
#include "fileinfo.h"

class CWAVInfo : public CFileInfo
{
public:
    
    // function to load a file into this structure
    // the argument passed is the path to a WAV file
    int   loadInfo( const char* srcWAV );
    int   loadInfo( const wchar_t* srcWAV );

	// Does the actual data loading, called by loadInfo
	int	  loadInfoFromHandle(FILE *fp);

    int   getBitrate() {return 0;}
    
    // functions to calculate the length of the song
    int   getFileSize() { return m_FileSize; };
    void  getFileName(char* input) { strcpy(input, AutoChar(m_FileName)); };

	int   getLengthInSeconds() {return m_LengthInSeconds;}
    void  getFormattedLength(char* input);
 
    // This information is not available in wav files:
    void  getTitle(char* input)		{input[0] = 0;}
    void  getArtist(char* input)	{input[0] = 0;}
    void  getAlbumArtist(char* input) { if (input) *input = 0; };
    void  getAlbum(char* input)		{input[0] = 0;}
    void  getComment(char* input)	{input[0] = 0;}
    void  getGenre(char* input)		{input[0] = 0;}

    int   getYear()					{return 0;}
    int   getTrackNumber()			{return 0;}
	int   getDiscNumber()			{return 0;};

    void  getTitle(wchar_t* input)	{input[0] = 0;};
    void  getArtist(wchar_t* input)	{input[0] = 0;};
    void  getAlbum(wchar_t* input)	{input[0] = 0;};
    void  getComment(wchar_t* input){input[0] = 0;};
    void  getGenre(wchar_t* input)	{input[0] = 0;};

private:

    // the file information can not be found elsewhere
    wchar_t m_FileName[260];
    int m_FileSize;
	int m_LengthInSeconds;
};

int CWAVInfo::loadInfo(const char *srcWAV)
{
	return loadInfo (AutoWide(srcWAV));
}

int CWAVInfo::loadInfo(const wchar_t *srcWAV)
{
    //return value, in case loadInfoFromHandle generates an error
	int ret = 0;

    memset(m_FileName, 0, sizeof(m_FileName));
    m_FileSize = 0;
	m_LengthInSeconds = 0;
	
	// open input-file stream to the specified file, name
    FILE *fp = _wfopen(srcWAV, L"rb");
    
    if (fp)	 // if the file was opened correctly
	{
        // get file size, by setting the pointer in the end and tell the position
        fseek(fp, 0, SEEK_END);
        m_FileSize = ftell(fp);

        // get scrFile into fileName variable
        wcscpy(m_FileName, srcWAV);

		//Call virtual function specific to format
		ret = loadInfoFromHandle(fp);
	}
	else
	{
		fclose(fp);
        return 1;
    }

	fclose(fp);
    return ret;
}

int	CWAVInfo::loadInfoFromHandle(FILE *fp)
{
	BYTE ChunkRIFF[12+4];
	BYTE ChunkFORMAT[24+4];

	fseek(fp, 0, SEEK_SET);
	fread((char *)ChunkRIFF, sizeof(char), 12, fp);
	fseek(fp, 12, SEEK_SET);
	fread((char *)ChunkFORMAT, sizeof(char), 24, fp);

	if (!memcmp(&ChunkRIFF[0], "RIFF", 4) &&
		!memcmp(&ChunkRIFF[8], "WAVE", 4) &&
		!memcmp(&ChunkFORMAT[0], "fmt ", 4))
	{
		int BytesPerSecond = 0;

		memcpy(&BytesPerSecond, &ChunkFORMAT[16], 4);

		//Length of file minus header size
		m_LengthInSeconds = (m_FileSize - 36) / BytesPerSecond;

		return 0;
	}
	else
	{
		return 1;
	}
}

void CWAVInfo::getFormattedLength(char* input)
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
