#ifndef _psfinfo_h_
#define _psfinfo_h_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <mbstring.h>
#include "fileinfo.h"

#define TAGMAX (50000)

class CPSFInfo : public CFileInfo {

    public:

    // function to load a file into this structure
    // the argument passed is the path to a PSF file
    int   loadInfo( const char* srcPSF );
    int   loadInfo( const wchar_t* srcPSF );

    // functions used to get information about the "file"
    int   getFileSize() { return fileSize; };
    void  getFileName(char* input);

    // functions to calculate the length of the song
    // and to present it nicely
    int   getLengthInSeconds();
    void  getFormattedLength(char* input);

    // information that is avaliable in tags
    void  getTitle(char* input);
    void  getArtist(char* input);
    void  getAlbumArtist(char* input) { if (input) *input = 0; };
    void  getAlbum(char* input);
    void  getComment(char* input);
    void  getGenre(char* input);

    int   getYear();
    int   getTrackNumber();
	int   getDiscNumber() { return 0; };

    void  getTitle(wchar_t* input);
    void  getArtist(wchar_t* input);
    void  getAlbum(wchar_t* input);
    void  getComment(wchar_t* input);
    void  getGenre(wchar_t* input);

    private:

    // these are the "sub-classes"

    // the file information can not be found elsewhere
    wchar_t fileName[260];
    int     fileSize;
	int		length;
	char	title[260];
	char	artist[260];
	char	gamename[260];
	int		year;
	char	genre[260];
	char	comment[260];
};

int CPSFInfo::loadInfo ( const char* srcPSF )
{
	return loadInfo (AutoWide(srcPSF));
}

int CPSFInfo::loadInfo ( const wchar_t* srcPSF )
{
    FILE* fp;
    char  header[12];

    fileName[0] = '\0';
    fileSize = 0;
	length = 0;
	title[0] = '\0';
	artist[0] = '\0';
	gamename[0] = '\0';
	year = 0;
	genre[0] = '\0';
	comment[0] = '\0';

    if ( srcPSF == NULL || srcPSF[0] == '\0' )
        return 1;
    if ( (fp = _wfopen ( srcPSF, L"rb" )) == NULL )
        return 1;
    if ( (fread ( &header, 1, 12, fp )) != 12 ) {
        fclose (fp);
        return 1;
    }
    if ( memcmp ( &header, "PSF", 3 ) != 0 ) {
        fclose (fp);
        return 1;
    }
	int rsize =
		((((unsigned)(header[ 4])) & 0xFF) <<  0) |
		((((unsigned)(header[ 5])) & 0xFF) <<  8) |
		((((unsigned)(header[ 6])) & 0xFF) << 16) |
		((((unsigned)(header[ 7])) & 0xFF) << 24);
	int exesize =
		((((unsigned)(header[ 8])) & 0xFF) <<  0) |
		((((unsigned)(header[ 9])) & 0xFF) <<  8) |
		((((unsigned)(header[10])) & 0xFF) << 16) |
		((((unsigned)(header[11])) & 0xFF) << 24);
	int tagstart = 16 + rsize + exesize;
    if ( fseek ( fp, tagstart, SEEK_SET ) != 0 ) {
        fclose (fp);
        return 1;
    }
	if ( (fread ( &header, 1, 5, fp)) != 5 ) {
		fclose (fp);
		return 1;
	}
	if ( memcmp ( &header, "[TAG]", 5) != 0 ) {
		fclose (fp);
		return 1;
	}
	tagstart += 5;
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, tagstart, SEEK_SET);
	int l = fileSize - tagstart;
	if(l < 0) l = 0;
	if(l > TAGMAX) l = TAGMAX;

    wcscpy ( fileName, srcPSF );

	char line[1025];
	char variable[512];
	char value[512];
	while (fgets(line, 1024, fp))
	{
		// 2005-01-23 Modified T-Matsuo
//		char *ptr = strchr(line, '=');
		char *ptr = (char *)_mbschr((const unsigned char *)line, '=');
		if (ptr)
		{
			// clean beginning white spaces
			for (*ptr++ = 0; ((0x01 < *ptr) && (*ptr < 0x20)); ptr++) ;
			lstrcpy(value, ptr);
			// clean ending white spaces
			for (ptr = value + lstrlen(value) - 1; ((0x01 < *ptr) && (*ptr < 0x20)); ptr--) *ptr = 0;

			// clean beginning white spaces
			for (ptr = line; ((0x01 < *ptr) && (*ptr < 0x20)); ptr++) ;
			lstrcpy(variable, ptr);
			// clean ending white spaces
			for (ptr = variable + lstrlen(variable) - 1; ((0x01 < *ptr) && (*ptr < 0x20)); ptr--) *ptr = 0;

			if (strcmpi(variable, "length") == 0)
			{
				int len = lstrlen(value);
				// count colons
				int colon = 0;
				for (ptr = value; ptr < (value + len); ptr++)
				{
					if (*ptr == ':') colon++;
				}

				int hours, min, sec, dec;
				hours = min = sec = dec = 0;
				switch (colon)
				{
				case 0:		sscanf(value, "%ld.%ld", &sec, &dec);						break;
				case 1:		sscanf(value, "%ld:%ld.%ld", &min, &sec, &dec);			break;
				case 2:		sscanf(value, "%ld:%ld:%ld.%ld", &hours, &min, &sec, &dec);	break;
				}
				length += hours * 3600 + min * 60 + sec;
			}
			else if (strcmpi(variable, "fade") == 0)
			{
				int len = lstrlen(value);
				// count colons
				int colon = 0;
				for (ptr = value; ptr < (value + len); ptr++)
				{
					if (*ptr == ':') colon++;
				}

				int hours, min, sec, dec;
				hours = min = sec = dec = 0;
				switch (colon)
				{
				case 0:		sscanf(value, "%ld.%ld", &sec, &dec);						break;
				case 1:		sscanf(value, "%ld:%ld.%ld", &min, &sec, &dec);			break;
				case 2:		sscanf(value, "%ld:%ld:%ld.%ld", &hours, &min, &sec, &dec);	break;
				}
				length += hours * 3600 + min * 60 + sec;
			}
			else if (strcmpi(variable, "title") == 0)
			{
				lstrcpy(title, value);
			}
			else if (strcmpi(variable, "artist") == 0)
			{
				lstrcpy(artist, value);
			}
			else if (strcmpi(variable, "game") == 0)
			{
				lstrcpy(gamename, value);
			}
			else if (strcmpi(variable, "year") == 0)
			{
				year = atoi(value);
			}
			else if (strcmpi(variable, "comment") == 0)
			{
				lstrcat(comment, value);
			}
		}
	}

    fclose (fp);
    return 0;
}

void CPSFInfo::getFileName(char* input) {

    strcpy(input, AutoChar(fileName));

}

int CPSFInfo::getLengthInSeconds()
{
    return length;
}

void CPSFInfo::getFormattedLength(char* input) {

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

void CPSFInfo::getTitle(char* input)
{
    strcpy(input, title);
}

void CPSFInfo::getArtist(char* input)
{
    strcpy(input, artist);
}

void CPSFInfo::getAlbum(char* input)
{
    strcpy(input, gamename);
}

void CPSFInfo::getComment(char* input)
{
    strcpy(input, comment);
}

void CPSFInfo::getGenre(char* input)
{
    strcpy(input, genre);
}

void CPSFInfo::getTitle(wchar_t* input)
{
    wcscpy(input, AutoWide(title, wndWinampAL.GetEncodingCodepage()));
}

void CPSFInfo::getArtist(wchar_t* input)
{
    wcscpy(input, AutoWide(artist, wndWinampAL.GetEncodingCodepage()));
}

void CPSFInfo::getAlbum(wchar_t* input)
{
    wcscpy(input, AutoWide(gamename, wndWinampAL.GetEncodingCodepage()));
}

void CPSFInfo::getComment(wchar_t* input)
{
    wcscpy(input, AutoWide(comment, wndWinampAL.GetEncodingCodepage()));
}

void CPSFInfo::getGenre(wchar_t* input)
{
    wcscpy(input, AutoWide(genre, wndWinampAL.GetEncodingCodepage()));
}

int CPSFInfo::getYear()
{
    return year;
}

int CPSFInfo::getTrackNumber()
{
    return 0;
}

#endif
