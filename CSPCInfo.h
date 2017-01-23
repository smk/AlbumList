#ifndef _spcinfo_h_
#define _spcinfo_h_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "fileinfo.h"
#include "AutoWide.h"
#include "AutoChar.h"

// ID666 Tag
typedef struct {
    char SongTitle[32];
    char GameTitle[32];
    char NameOfDumper[16];
    char Comments[32];
    char DateDumped[11];            // Date SPC was dumped (MM/DD/YYYY)
    char Length[3];                 // Number of seconds to play song before fading out
    char FadeLength[5];             // Length of fade in milliseconds
    char ArtistOfSong[32];
    char DefaultChannelDisables;    // (0 = enable, 1 = disable)
    char Emulator;                  // Emulator used to dump SPC: 0 = unknown, 1 = ZSNES, 2 = Snes9x
    char Reserved[45];
} SPCID666Tag;

// Extended ID666 Tag
typedef struct {
    char SongName[256+1];
    char GameName[256+1];
    char ArtistsName[256+1];
    char DumpersName[256+1];
    int  DumpDate[4+1];         // Date song was dumped (stored as yyyymmdd)
    char EmulatorUsed;
    char Comments[256+1];
    char OfficialTitle[256+1];  // OfficialSoundtrackTitle
    char OSTdisc;
    unsigned char OSTtrack[2];           // (upper byte is the number 0-99, lower byte is an optional ASCII character)
    char PublishersName[256+1];
    unsigned char CopyrightYear[2];
    int  IntroductionLength[4]; // (lengths are stored in 1/64000th seconds)
    int  LoopLength[4];
    int  EndLength[4];
    int  FadeLength[4];
    unsigned char MutedChannels;         // (a bit is set for each channel that's muted)
} SPCExtID666Tag;

class CSPCInfo : public CFileInfo {

    public:

    // function to load a file into this structure
    // the argument passed is the path to a SPC file
    int   loadInfo( const char* srcSPC );
    int   loadInfo( const wchar_t* srcSPC );

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
	SPCID666Tag     ID666Tag;   // basic tag
	SPCExtID666Tag  tag;        // extended tag

    // the file information can not be found elsewhere
    wchar_t fileName[260];
    int     fileSize;
};

int CSPCInfo::loadInfo ( const char* srcSPC )
{
	return loadInfo(AutoWide(srcSPC));
}

int CSPCInfo::loadInfo ( const wchar_t* srcSPC )
{
    FILE* fp;
    char  header[37];

    fileName[0] = '\0';
    fileSize = 0;
    memset ( &ID666Tag, 0, sizeof (ID666Tag) );
    memset ( &tag, 0, sizeof (tag) );

    if ( srcSPC == NULL || srcSPC[0] == '\0' )
        return 1;
    if ( (fp = _wfopen ( srcSPC, L"rb" )) == NULL )
        return 1;
    if ( (fread ( &header, 1, 37, fp )) != 37 ) {
        fclose (fp);
        return 1;
    }
    if ( memcmp ( &header, "SNES-SPC700 Sound File Data v0.30", 33 ) != 0 ) {
        fclose (fp);
        return 1;
    }
    if ( fseek ( fp, 0, SEEK_END ) != 0 ) {
        fclose (fp);
        return 1;
    }
    fileSize = ftell (fp);
    wcscpy ( fileName, srcSPC );

    if ( header[33] != 26 || header[34] != 26 ) {
        fclose (fp);
        return 1;
    }
    if ( header[35] == 26 ) {   // read ID666 tag
        if ( (fseek ( fp, 46, SEEK_SET )) != 0 ) {
            fclose (fp);
            return 1;
        }
        if ( (fread ( &ID666Tag, 1, sizeof (ID666Tag), fp )) != sizeof (ID666Tag) ) {
            fclose (fp);
            return 1;
        }

        strcpy ( tag.ArtistsName, ID666Tag.ArtistOfSong );
        strcpy ( tag.Comments, ID666Tag.Comments );
        strcpy ( tag.DumpersName, ID666Tag.NameOfDumper );
        strcpy ( tag.GameName, ID666Tag.GameTitle );
        strcpy ( tag.SongName, ID666Tag.SongTitle );
    }

    if ( (fseek ( fp, 66048, SEEK_SET )) != 0 ) {
        fclose (fp);
        return 0;
    }
    {   // read Extended ID666 tag
        long size;

        if ( (fread ( &header, 1, 8, fp )) != 8 ) {
            fclose (fp);
            return 0;
        }
        if ( memcmp ( &header, "xid6", 4 ) != 0 ) {
            fclose (fp);
            return 0;
        }

        size = header[4] + (header[5] << 8) + (header[6] << 16) + (header[7] << 24) - 4;
        if ( size <= 0 ) {
            fclose (fp);
            return 0;
        }

        while ( size > 0 ) {
            unsigned char   head[4];
            unsigned long   len;
            char            temp[260];

            size -= 4;
            if ( (fread ( &head, 1, 4, fp )) != 4 ) {
                fclose (fp);
                return 0;
            }

            len = head[1] > 0  ?  head[2] + (head[3] << 8)  :  0;

            if ( len > 0 ) {
                size -= len;
                if ( (fread ( &temp, 1, len, fp )) != len ) {
                    fclose (fp);
                    return 0;
                }
            }

            switch ( head[0] ) {
            case 0x01:  // String  4-256 Song name
                if ( head[1] != 0 ) {
                    memcpy ( tag.SongName, temp, len & 0xFF );
                    tag.SongName[len & 0xFF] = '\0';
                }
                break;
            case 0x02:  // String  4-256 Game name
                if ( head[1] != 0 ) {
                    memcpy ( tag.GameName, temp, len & 0xFF );
                    tag.GameName[len & 0xFF] = '\0';
                }
                break;
            case 0x03:  // String  4-256 Artist's name
                if ( head[1] != 0 ) {
                    memcpy ( tag.ArtistsName, temp, len & 0xFF );
                    tag.ArtistsName[len & 0xFF] = '\0';
                }
                break;
            case 0x04:  // String  4-256 Dumper's name
                if ( head[1] != 0 ) {
                    memcpy ( tag.DumpersName, temp, len & 0xFF );
                    tag.DumpersName[len & 0xFF] = '\0';
                }
                break;
            case 0x05:  // Integer 4     Date song was dumped (stored as yyyymmdd)
                break;
            case 0x06:  // Data    1     Emulator used
                break;
            case 0x07:  // String  4-256 Comments
                if ( head[1] != 0 ) {
                    memcpy ( tag.Comments, temp, len & 0xFF );
                    tag.Comments[len & 0xFF] = '\0';
                }
                break;
            case 0x10:  // String  4-256 Official Soundtrack Title
                if ( head[1] != 0 ) {
                    memcpy ( tag.OfficialTitle, temp, len & 0xFF );
                    tag.OfficialTitle[len & 0xFF] = '\0';
                }
                break;
            case 0x11:  // Data    1     OST disc
                tag.OSTdisc = head[2];
                break;
            case 0x12:  // Data    2     OST track (upper byte is the number 0-99, lower byte is an optional ASCII character)
                if ( head[1] == 0 ) {
                    tag.OSTtrack[0] = head[2];
                    tag.OSTtrack[1] = head[3];
                }
                break;
            case 0x13:  // String  4-256 Publisher's name
                if ( head[1] != 0 ) {
                    memcpy ( tag.PublishersName, temp, len & 0xFF );
                    tag.PublishersName[len & 0xFF] = '\0';
                }
                break;
            case 0x14:  // Data    2     Copyright year
                if ( head[1] == 0 ) {
                    tag.CopyrightYear[0] = head[2];
                    tag.CopyrightYear[1] = head[3];
                }
                break;
            case 0x30:  // Integer 4     Introduction length (lengths are stored in 1/64000th seconds)
                break;
            case 0x31:  // Integer 4     Loop length
                break;
            case 0x32:  // Integer 4     End length
                break;
            case 0x33:  // Integer 4     Fade length
                break;
            case 0x34:  // Data    1     Muted channels (a bit is set for each channel that's muted)
                break;
            }
        }
    }

    fclose (fp);
    return 0;
}

void CSPCInfo::getFileName(char* input) {

    strcpy(input, AutoChar(fileName));

}

int CSPCInfo::getLengthInSeconds()
{
    char temp[6];
    int  length;
    memcpy ( &temp, ID666Tag.Length, 3 );
    temp[3] = '\0';
    length = atoi (temp);
    memcpy ( &temp, ID666Tag.FadeLength, 5 );
    temp[5] = '\0';
    length += atoi (temp) / 1000;
    return length;
}

void CSPCInfo::getFormattedLength(char* input) {

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

void CSPCInfo::getTitle(char* input)
{
    strcpy(input, tag.SongName);
}

void CSPCInfo::getArtist(char* input)
{
    strcpy(input, tag.ArtistsName);
}

void CSPCInfo::getAlbum(char* input)
{
    strcpy(input, tag.GameName);
}

void CSPCInfo::getComment(char* input)
{
    strcpy(input, tag.Comments);
}

void CSPCInfo::getGenre(char* input)
{
    *input = '\0';
}

void CSPCInfo::getTitle(wchar_t* input)
{
    wcscpy(input, AutoWide(tag.SongName, wndWinampAL.GetEncodingCodepage()));
}

void CSPCInfo::getArtist(wchar_t* input)
{
    wcscpy(input, AutoWide(tag.ArtistsName, wndWinampAL.GetEncodingCodepage()));
}

void CSPCInfo::getAlbum(wchar_t* input)
{
    wcscpy(input, AutoWide(tag.GameName, wndWinampAL.GetEncodingCodepage()));
}

void CSPCInfo::getComment(wchar_t* input)
{
    wcscpy(input, AutoWide(tag.Comments, wndWinampAL.GetEncodingCodepage()));
}

void CSPCInfo::getGenre(wchar_t* input)
{
    *input = '\0';
}

int CSPCInfo::getYear()
{
    return (unsigned)tag.CopyrightYear[0] + ((unsigned)tag.CopyrightYear[1] << 8);
}

int CSPCInfo::getTrackNumber()
{
    return tag.OSTtrack[0];
}

#endif
