#ifndef _apeinfo_h_
#define _apeinfo_h_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "fileinfo.h"

#define COMPRESSION_LEVEL_FAST			1000
#define COMPRESSION_LEVEL_NORMAL		2000
#define COMPRESSION_LEVEL_HIGH			3000
#define COMPRESSION_LEVEL_EXTRA_HIGH	4000

#define MAC_FORMAT_FLAG_8_BIT				1	// is 8-bit
#define MAC_FORMAT_FLAG_CRC					2	// uses the new CRC32 error detection
#define MAC_FORMAT_FLAG_HAS_PEAK_LEVEL		4	// unsigned __int32 Peak_Level after the header
#define MAC_FORMAT_FLAG_24_BIT				8	// is 24-bit
#define MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS	16	// has the number of seek elements after the peak level
#define MAC_FORMAT_FLAG_CREATE_WAV_HEADER	32  // create the wave header on decompression (not stored)

class CAPEInfo : public CFileInfo {

    public:
    enum tag_t {
        no_tag     = 0,
        ID3v1_tag  = 1,
        APE1_tag   = 2,
        APE2_tag   = 3
    };
    struct APETagFooterStruct
    {
        unsigned char   ID       [8];    // should equal 'APETAGEX'
        unsigned char   Version  [4];    // currently 1000 (version 1.000)
        unsigned char   Length   [4];    // the complete size of the tag, including this footer
        unsigned char   TagCount [4];    // the number of fields in the tag
        unsigned char   Flags    [4];    // the tag flags (none currently defined)
        unsigned char   Reserved [8];    // reserved for later use
    };
    typedef struct {
        char cID[4];                            // should equal 'MAC '
        unsigned __int16 nVersion;              // version number * 1000 (3.81 = 3810)
        unsigned __int16 nCompressionLevel;     // the compression level
        unsigned __int16 nFormatFlags;          // any format flags (for future use)
        unsigned __int16 nChannels;             // the number of channels (1 or 2)
        unsigned __int32 nSampleRate;           // the sample rate (typically 44100)
        unsigned __int32 nHeaderBytes;          // the bytes after the MAC header that compose the WAV header
        unsigned __int32 nTerminatingBytes;     // the bytes after that raw data (for extended info)
        unsigned __int32 nTotalFrames;          // the number of frames in the file
        unsigned __int32 nFinalFrameBlocks;     // the number of samples in the final frame
    } APE_HEADER;

    typedef struct {
        char  Title     [512];
        char  Artist    [512];
        char  Album     [512];
        char  Comment   [512];
        char  Year      [ 32];
        char  TrackStr  [ 32];
        char  GenreStr  [128];
        int            Track;
        unsigned int   Genre;
        enum tag_t     TagType;
	    unsigned int   TagSize;
    } APETagData;

    // function to load a file into this structure
    // the argument passed is the path to a APE file
    int   loadInfo( const char* srcAPE );
    int   loadInfo( const wchar_t* srcAPE );

    // functions used to get information about the "file"
    int   getFileSize() { return fileSize; };
    void  getFileName(char* input);

    // functions to calculate the length of the song
    // and to present it nicely
    int   getLengthInSeconds();
    void  getFormattedLength(char* input);

    // information that is available in tags
    void  getTitle(char* input);
    void  getArtist(char* input);
    void  getAlbumArtist(char* input) { if (input) *input = 0; };
    void  getAlbum(char* input);
    void  getComment(char* input);
    void  getGenre(char* input);

    int   getYear();
    int   getTrackNumber();
	int   getDiscNumber() { return 0; };

    private:

    // these are the "sub-classes"
	//MPCStreamInfo   header;
    APETagData      tag;

    // the file information can not be found elsewhere
    wchar_t fileName[260];
    int     fileSize;

    int     Length;

    void memcpy_crop ( char* dst, const char* src, size_t len );
    unsigned long Read_LE_Uint32 ( const unsigned char *p );
    void Write_LE_Uint32 ( unsigned char *p , const unsigned long value );
    int ReadID3v1Tags ( const wchar_t* filename, APETagData* info );
    int ReadAPE1Tags ( const wchar_t* filename, APETagData* info );
    int ConvertUTF8ToANSI ( char* buffer, const int buf_len );
    int ReadAPE2Tags ( const wchar_t* filename, APETagData* info );
    long file_size ( const wchar_t* filename );
};

void GenreToString ( char* GenreStr, const int genre )
{
    const char*  GenreList [] = {
        "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",
        "Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies",
        "Other", "Pop", "R&B", "Rap", "Reggae", "Rock",
        "Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks",
        "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
        "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House",
        "Game", "Sound Clip", "Gospel", "Noise", "AlternRock", "Bass",
        "Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
        "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk",
        "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta",
        "Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American", "Cabaret",
        "New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
        "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",
        "Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk", "Swing",
        "Fast-Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass", "Avantgarde",
        "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band",
        "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson",
        "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus",
        "Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
        "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet",
        "Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall",
        "Goa", "Drum & Bass", "Club House", "Hardcore", "Terror",
        "Indie", "BritPop", "NegerPunk", "Polsk Punk", "Beat",
        "Christian Gangsta", "Heavy Metal", "Black Metal", "Crossover", "Contemporary C",
        "Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop",
        "SynthPop",
    };

	if ( genre >= 0 && genre < sizeof(GenreList) / sizeof(*GenreList) ) {
		strcpy( (char *)GenreStr, GenreList [genre] );
	} else {
		GenreStr[0] = 0;
	}
}

void CAPEInfo::memcpy_crop ( char* dst, const char* src, size_t len )
{
    size_t  i;

    for ( i = 0; i < len; i++ )
        if  ( src[i] != '\0' ) dst[i] = src[i]; else break;

    // dst[i] points behind the string contents
    while ( i > 0  &&  dst [i-1] == ' ' )
        i--;

    dst [i] = '\0';
}

unsigned long CAPEInfo::Read_LE_Uint32 ( const unsigned char *p )
{
    return ((unsigned long)p[0] <<  0) |
           ((unsigned long)p[1] <<  8) |
           ((unsigned long)p[2] << 16) |
           ((unsigned long)p[3] << 24);
}

void CAPEInfo::Write_LE_Uint32 ( unsigned char *p , const unsigned long value )
{
	p[0] = (unsigned char)((value >> 0)  & 0xFF);
	p[1] = (unsigned char)((value >> 8)  & 0xFF);
	p[2] = (unsigned char)((value >> 16) & 0xFF);
	p[3] = (unsigned char)((value >> 24) & 0xFF);
}

int CAPEInfo::ReadID3v1Tags ( const wchar_t* filename, APETagData* info )
{
    FILE*  fp;
    char   tmp [128];

    if ( (fp = _wfopen (filename, L"rb")) == NULL )
        return 1;

    fseek  ( fp, -128, SEEK_END );
    fread  ( tmp, 1, sizeof tmp, fp );
    fclose ( fp );

    // reset variables
    info->Track =   0;
    info->Genre = 255;
	info->TrackStr[0] = 0;
	info->GenreStr[0] = 0;
    memset ( info->Title  , 0, sizeof(info->Title  ) );
    memset ( info->Artist , 0, sizeof(info->Artist ) );
    memset ( info->Album  , 0, sizeof(info->Album  ) );
    memset ( info->Year   , 0, sizeof(info->Year   ) );
    memset ( info->Comment, 0, sizeof(info->Comment) );

    // check for id3-tag
    if ( memcmp (tmp, "TAG", 3) != NULL )
        return 0;

    info->TagType = ID3v1_tag;
	info->TagSize = 128;

    memcpy_crop  ( info->Title  , tmp +  3, 30 );
    memcpy_crop  ( info->Artist , tmp + 33, 30 );
    memcpy_crop  ( info->Album  , tmp + 63, 30 );
    memcpy_crop  ( info->Year   , tmp + 93,  4 );
    memcpy_crop  ( info->Comment, tmp + 97, 30 );

    info->Genre = tmp [127];
    if ( tmp [125] == 0 )
        info->Track = tmp[126];
	GenreToString ( (unsigned char *)info->GenreStr, info->Genre );
	sprintf( info->TrackStr, "%d", info->Track );

    return 0;
}

#undef TAG_ANALYZE
#define TAG_ANALYZE(item,elem)                                      \
    if ( 0 == memicmp (p, #item, sizeof #item ) ) {                 \
        unsigned int tlen;                                          \
        tlen = len < sizeof info->elem ? len : sizeof info->elem-1; \
        p += sizeof #item;                                          \
        memcpy ( info->elem, p, tlen );                             \
        info->elem[tlen] = '\0';                                    \
        p += len;                                                   \
    } else

int CAPEInfo::ReadAPE1Tags ( const wchar_t* filename, APETagData* info )
{
    unsigned long              len;
    unsigned long              flags;
    unsigned char*             buff;
    unsigned char*             p;
    unsigned char*             end;
    struct APETagFooterStruct  T;
    unsigned long              TagLen;
    unsigned long              TagCount;
    FILE*                      fp;

    info->Track =   0;
    info->Genre = 255;
	info->TrackStr[0] = 0;
	info->GenreStr[0] = 0;
    memset ( info->Title  , 0, sizeof(info->Title  ) );
    memset ( info->Artist , 0, sizeof(info->Artist ) );
    memset ( info->Album  , 0, sizeof(info->Album  ) );
    memset ( info->Year   , 0, sizeof(info->Year   ) );
    memset ( info->Comment, 0, sizeof(info->Comment) );

    if ( (fp = _wfopen (filename, L"rb")) == NULL )
        return 1;

    buff = NULL;
    if ( fseek ( fp, -(long)sizeof T, SEEK_END ) != NULL )
        goto notag;
    if ( sizeof T != fread ( &T, 1, sizeof T, fp ) )
        goto notag;
    if ( memcmp ( T.ID, "APETAGEX", sizeof T.ID ) != 0 )
        goto notag;
    if ( Read_LE_Uint32 (T.Version) != 1000 )
        goto notag;
    TagLen = Read_LE_Uint32 (T.Length);
    if ( TagLen <= sizeof T )
        goto notag;
    if ( fseek ( fp, -(long)TagLen, SEEK_END ) != NULL )
        goto notag;
    if ( (buff = (unsigned char *)malloc ( TagLen + 1 )) == NULL )
        goto notag;
    if ( TagLen - sizeof T != fread ( buff, 1, TagLen - sizeof T, fp ) )
        goto notag;

    TagCount = Read_LE_Uint32 (T.TagCount);
    end = buff + TagLen - sizeof (T);

    for ( p = buff; p < end && TagCount--; ) {
        len   = Read_LE_Uint32 ( p ); p += 4;
        flags = Read_LE_Uint32 ( p ); p += 4;
        TAG_ANALYZE ( Title     , Title   )
        TAG_ANALYZE ( Artist    , Artist  )
        TAG_ANALYZE ( Album     , Album   )
        TAG_ANALYZE ( Comment   , Comment )
        TAG_ANALYZE ( Year      , Year    )
        TAG_ANALYZE ( Genre     , GenreStr)
        TAG_ANALYZE ( Track     , TrackStr)
        {
            p += strlen((char *)p) + 1 + len;
        }
    }
    info->Track = atoi           ( (char *)info->TrackStr );
    info->Genre = GenreToInteger ( (char *)info->GenreStr );
    info->TagType = APE1_tag;
	info->TagSize = TagLen;

    free ( buff );
    fclose (fp);
    return 0;

notag:
    free ( buff );
    fclose (fp);
    return 0;
}

// convert UTF-8 to Windows ANSI
int CAPEInfo::ConvertUTF8ToANSI ( char* buffer, const int buf_len )
{
    WCHAR*  wszValue;          // Unicode value
    size_t  utf8_len = strlen ( (char *)buffer );
    size_t  len;

    if ( (wszValue = (WCHAR *)malloc ( (utf8_len + 1) * 2 )) == NULL )
        return 1;

    // Convert UTF-8 value to Unicode
    if ( (len = MultiByteToWideChar ( CP_UTF8, 0, (char *)buffer, utf8_len + 1, wszValue, (utf8_len + 1) * 2 )) == 0 ) {
        free ( wszValue );
        return 1;
    }

    // Convert Unicode value to ANSI
    if ( (len = WideCharToMultiByte ( CP_ACP, 0, wszValue, -1, (char *)buffer, buf_len, NULL, NULL )) == 0 ) {
        free ( wszValue );
        return 1;
    }

    return 0;
}

int CAPEInfo::ReadAPE2Tags ( const wchar_t* filename, APETagData* info )
{
    unsigned long              len;
    unsigned long              flags;
    unsigned char*             buff;
    unsigned char*             p;
    unsigned char*             end;
    struct APETagFooterStruct  T;
    unsigned long              TagLen;
    unsigned long              TagCount;
    FILE*                      fp;

    info->Track =   0;
    info->Genre = 255;
	info->TrackStr[0] = 0;
	info->GenreStr[0] = 0;
    memset ( info->Title  , 0, sizeof(info->Title  ) );
    memset ( info->Artist , 0, sizeof(info->Artist ) );
    memset ( info->Album  , 0, sizeof(info->Album  ) );
    memset ( info->Year   , 0, sizeof(info->Year   ) );
    memset ( info->Comment, 0, sizeof(info->Comment) );

    if ( (fp = _wfopen (filename, L"rb")) == NULL )
        return 1;

    buff = NULL;
    if ( fseek ( fp, -(long)sizeof T, SEEK_END ) != NULL )
        goto notag;
    if ( sizeof T != fread ( &T, 1, sizeof T, fp ) )
        goto notag;
    if ( memcmp ( T.ID, "APETAGEX", sizeof T.ID ) != 0 )
        goto notag;
    if ( Read_LE_Uint32 (T.Version) != 2000 )
        goto notag;
    TagLen = Read_LE_Uint32 (T.Length);
    if ( TagLen <= sizeof T )
        goto notag;
    if ( fseek ( fp, -(long)TagLen, SEEK_END ) != NULL )
        goto notag;
    if ( (buff = (unsigned char *)malloc ( TagLen + 1 )) == NULL )
        goto notag;
    if ( TagLen - sizeof T != fread ( buff, 1, TagLen - sizeof T, fp ) )
        goto notag;

    TagCount = Read_LE_Uint32 (T.TagCount);
    end = buff + TagLen - sizeof (T);

    for ( p = buff; p < end && TagCount--; ) {
        len   = Read_LE_Uint32 ( p ); p += 4;
        flags = Read_LE_Uint32 ( p ); p += 4;
        TAG_ANALYZE ( Title     , Title   )
        TAG_ANALYZE ( Artist    , Artist  )
        TAG_ANALYZE ( Album     , Album   )
        TAG_ANALYZE ( Comment   , Comment )
        TAG_ANALYZE ( Year      , Year    )
        TAG_ANALYZE ( Genre     , GenreStr)
        TAG_ANALYZE ( Track     , TrackStr)
        {
            p += strlen((char *)p) + 1 + len;
        }
    }

    ConvertUTF8ToANSI ( info->Artist  , sizeof (info->Artist  ) );
    ConvertUTF8ToANSI ( info->Title   , sizeof (info->Title   ) );
    ConvertUTF8ToANSI ( info->Album   , sizeof (info->Album   ) );
    ConvertUTF8ToANSI ( info->Comment , sizeof (info->Comment ) );
    ConvertUTF8ToANSI ( info->GenreStr, sizeof (info->GenreStr) );
    ConvertUTF8ToANSI ( info->TrackStr, sizeof (info->TrackStr) );
    ConvertUTF8ToANSI ( info->Year    , sizeof (info->Year    ) );

    info->Track = atoi           ( (char *)info->TrackStr );
    info->Genre = GenreToInteger ( (char *)info->GenreStr );
    info->TagType = APE2_tag;
	info->TagSize = TagLen;

    free ( buff );
    fclose (fp);
    return 0;

notag:
    free ( buff );
    fclose (fp);
    return 0;
}

long CAPEInfo::file_size ( const wchar_t* filename )
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

int CAPEInfo::loadInfo ( const char* srcAPE )
{
	return loadInfo (AutoWide(srcAPE));
}

int CAPEInfo::loadInfo ( const wchar_t* srcAPE )
{
    FILE*               fp;
    APE_HEADER          header;
    unsigned long       nBlocksPerFrame;
    int                 nBitsPerSample;
    unsigned __int64    samples;

    memset ( &tag, 0, sizeof (tag) );
    wcscpy(fileName,srcAPE);
    fileSize = file_size(srcAPE);

    if ( (fp = _wfopen ( srcAPE, L"rb" )) == NULL )
        return 1;

    if ( (fread ( &header, 1, sizeof (header), fp )) != sizeof (header) ) {
        fclose ( fp );
        return 1;
    }
    fclose ( fp );

    if ( memcmp ( header.cID, "MAC ", 4 ) != 0 )
        return 1;

    nBlocksPerFrame = ((header.nVersion >= 3900) || ((header.nVersion >= 3800) && (header.nCompressionLevel == COMPRESSION_LEVEL_EXTRA_HIGH))) ? 73728 : 9216;
    if ((header.nVersion >= 3950)) nBlocksPerFrame = 73728 * 4;
    nBitsPerSample = (header.nFormatFlags & MAC_FORMAT_FLAG_8_BIT) ? 8 : ((header.nFormatFlags & MAC_FORMAT_FLAG_24_BIT) ? 24 : 16);
    samples = (header.nTotalFrames == 0) ? 0 : ((header.nTotalFrames -  1) * nBlocksPerFrame) + header.nFinalFrameBlocks;

    Length = (header.nSampleRate > 0)  ?  (unsigned long)(samples / header.nSampleRate)  :  0;

    enum tag_t _tag = tag.TagType;
    ReadAPE2Tags ( srcAPE, &tag );
    if ( tag.TagType == _tag )
        ReadAPE1Tags ( srcAPE, &tag );
    if ( tag.TagType == _tag )
        ReadID3v1Tags ( srcAPE, &tag );

    return 0;
}

void CAPEInfo::getFileName(char* input) {

    strcpy(input, AutoChar(fileName));

}

int CAPEInfo::getLengthInSeconds()
{
    return Length;
}

void CAPEInfo::getFormattedLength(char* input) {

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

void CAPEInfo::getTitle(char* input)
{
    // copy result into inputstring
    strcpy(input, (char *)tag.Title);
}

void CAPEInfo::getArtist(char* input)
{
    strcpy(input, (char *)tag.Artist);
}

void CAPEInfo::getAlbum(char* input)
{
    strcpy(input, (char *)tag.Album);
}

void CAPEInfo::getComment(char* input)
{
    strcpy(input, (char *)tag.Comment);
}

void CAPEInfo::getGenre(char* input)
{
    strcpy(input, (char *)tag.GenreStr);
}

int CAPEInfo::getYear()
{
    return atoi((char *)tag.Year);
}

int CAPEInfo::getTrackNumber()
{
    return tag.Track;
}

#endif
