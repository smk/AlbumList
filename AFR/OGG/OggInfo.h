#ifndef _ogginfo_h_
#define _ogginfo_h_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <vorbis/vorbisfile.h>
#include "..\\..\\fileinfo.h"
#include "..\\..\\AutoWide.h"
#include "..\\..\\AutoChar.h"

class COGGInfo : public CFileInfo {

    public:

    // function to load a file into this structure
    // the argument passed is the path to a Ogg file
    int   loadInfo( const char* srcOGG );
    int   loadInfo( const wchar_t* srcOGG );

    // functions used to get information about the "file"
    int   getFileSize() { return fileSize; };
    void  getFileName(char* input);

    // functions to calculate the length of the song
    // and to present it nicely
    int   getLengthInSeconds();
    void  getFormattedLength(char* input);

    // information that is avaliable in the Ogg tag
    void  getTitle(char* input);
    void  getArtist(char* input);
    void  getAlbumArtist(char* input) { input[0] = 0; }
    void  getAlbum(char* input);
    void  getComment(char* input);
    void  getGenre(char* input);

    int   getYear();
    int   getTrackNumber();
	int   getDiscNumber() { return 0; };

    private:

    // these are the "sub-classes"
    int Length;
    char Title[512];
    char Artist[512];
    char Album[512];
    char Comment[512];
    char Genre[512];
    int Year;
    int Track;

    // the file information can not be found elsewhere
    wchar_t fileName[260];
    int fileSize;

};

long _ogg_file_size ( const wchar_t* filename )
{
	FILE *fp;
	long pos;

	if ( (fp = _wfopen ( filename, L"rb" ) ) == NULL ) {
		return -1;
	}
    if ( 0 != fseek (fp, 0, SEEK_END ) ) {
        fclose (fp);
        return -1;
    }
    if ( (pos = ftell (fp)) < 0 ) {
        fclose (fp);
        return -1;
    }
	fclose (fp);

	return pos;
}

// some sort of UTF-8 decoder :)
void utf8_decode ( const unsigned char *src, unsigned char *dst, int maxlen )
{
    int i;
    while ( --maxlen > 0 ) {
        if ( *src == '\0' ) { // terminating null
            *dst = *src;
            return;
        }
        else if ( *src <= 0x7F ) { // ASCII
            *dst++ = *src++;
        }
        else if ( *src >= 0xC0 && *src <= 0xFD ) { // UTF-8 multibyte
            if ( *src >= 0xFC ) { // 1111110x, 5 bytes after
                src++;
                for ( i = 5; i > 0; i-- ) {
                    if ( *src >= 0x80 && *src <= 0xBF ) src++;
                    else break;
                }
                *dst++ = '?';
            } else
            if ( *src >= 0xF8 ) { // 111110xx, 4 bytes
                src++;
                for ( i = 4; i > 0; i-- ) {
                    if ( *src >= 0x80 && *src <= 0xBF ) src++;
                    else break;
                }
                *dst++ = '?';
            } else
            if ( *src >= 0xF0 ) { // 11110xxx, 3 bytes
                src++;
                for ( i = 3; i > 0; i-- ) {
                    if ( *src >= 0x80 && *src <= 0xBF ) src++;
                    else break;
                }
                *dst++ = '?';
            } else
            if ( *src >= 0xE0 ) { // 1110xxxx, 2 bytes
                src++;
                for ( i = 2; i > 0; i-- ) {
                    if ( *src >= 0x80 && *src <= 0xBF ) src++;
                    else break;
                }
                *dst++ = '?';
            } else
            if ( *src >= 0xC0 ) { // 110xxxxx, 1 byte
                int b1 = *src++;
                int b2 = *src++;
                if ( b2 >= 0x80 && b2 <= 0xBF ) {
                    int ch = (b2 & 0x3F) | ((b1 & 0x1F) << 6);
                    *dst++ = ch <= 0xFF ? ch : '?';
                } else {
                    *dst++ = '?';
                }
            }
        } else { // Illegal
            *dst = '\0';
            return;
        }
    }
    *dst = '\0';
}

COGGInfo::loadInfo ( const char* srcOGG )
{
	return loadInfo (AutoWide(srcOGG));
}

COGGInfo::loadInfo ( const wchar_t* srcOGG )
{
    FILE *fp;
    OggVorbis_File vf;
    double length;
    vorbis_comment *vc;

    wcsncpy (fileName, srcOGG, 260);
    fileSize = _ogg_file_size (srcOGG);

    if ( (fp = _wfopen ( srcOGG, L"rb" )) == NULL )
        return 1;
    if ( ov_open (fp, &vf, NULL, 0 ) != 0 ) {
        fclose (fp);
        return 1;
    }
    length = ov_time_total ( &vf, -1 );
    if ( length == OV_EINVAL ) length = 0;
    Length = (int)length;

    Title[0]   = '\0';
    Artist[0]  = '\0';
    Album[0]   = '\0';
    Comment[0] = '\0';
    Genre[0]   = '\0';
    Year       =   0;
    Track      =   0;

    vc = ov_comment (&vf, -1);
    if ( vc != NULL ) {
        for ( int i = 0; i < vc->comments; i++ ) {
            if ( vc->user_comments[i] != NULL ) {
                if ( strnicmp ( vc->user_comments[i], "DATE=", 5 ) == 0 ) {
                    Year = atoi ( (char *)(vc->user_comments[i] + 5) );
                }
                else if ( strnicmp ( vc->user_comments[i], "TITLE=", 6 ) == 0 ) {
                    // strcpy ( Title, (char *)(vc->user_comments[i] + 6) );
                    utf8_decode ( (unsigned char *)(vc->user_comments[i] + 6), (unsigned char *)Title, sizeof (Title) );
                }
                else if ( strnicmp ( vc->user_comments[i], "ARTIST=", 7 ) == 0 ) {
                    // strcpy ( Artist, (char *)(vc->user_comments[i] + 7) );
                    utf8_decode ( (unsigned char *)(vc->user_comments[i] + 7), (unsigned char *)Artist, sizeof (Artist) );
                }
                else if ( strnicmp ( vc->user_comments[i], "ALBUM=", 6 ) == 0 ) {
                    // strcpy ( Album, (char *)(vc->user_comments[i] + 6) );
                    utf8_decode ( (unsigned char *)(vc->user_comments[i] + 6), (unsigned char *)Album, sizeof (Album) );
                }
                else if ( strnicmp ( vc->user_comments[i], "COMMENT=", 8 ) == 0 ) {
                    // strcpy ( Comment, (char *)(vc->user_comments[i] + 8) );
                    utf8_decode ( (unsigned char *)(vc->user_comments[i] + 8), (unsigned char *)Comment, sizeof (Comment) );
                }
                else if ( strnicmp ( vc->user_comments[i], "GENRE=", 6 ) == 0 ) {
                    // strcpy ( Genre, (char *)(vc->user_comments[i] + 6) );
                    utf8_decode ( (unsigned char *)(vc->user_comments[i] + 6), (unsigned char *)Genre, sizeof (Genre) );
                }
                else if ( strnicmp ( vc->user_comments[i], "TRACKNUMBER=", 12 ) == 0 ) {
                    Track = atoi ( (char *)(vc->user_comments[i] + 12) );
                }
            }
        }
    }
    ov_clear (&vf);

    return 0;
}

void COGGInfo::getFileName(char* input) {

    strcpy(input, AutoChar(fileName));

}

int COGGInfo::getLengthInSeconds()
{
    return Length;
}

void COGGInfo::getFormattedLength(char* input) {

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

void COGGInfo::getTitle(char* input)
{
    // copy result into inputstring
    strcpy(input, (char *)Title);
}

void COGGInfo::getArtist(char* input)
{
    strcpy(input, (char *)Artist);
}

void COGGInfo::getAlbum(char* input)
{
    strcpy(input, (char *)Album);
}

void COGGInfo::getComment(char* input)
{
    strcpy(input, (char *)Comment);
}

void COGGInfo::getGenre(char* input)
{
    strcpy(input, (char *)Genre);
}

int COGGInfo::getYear()
{
    return Year;
}

int COGGInfo::getTrackNumber()
{
    return Track;
}

#endif
