#ifndef _mpcinfo_h_
#define _mpcinfo_h_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "fileinfo.h"
#include "AutoWide.h"
#include "AutoChar.h"

typedef struct {
    // MPC related data
    long            HeaderPosition; // byte position of header
    unsigned int    StreamVersion;  // Streamversion of current file
    unsigned int    Bitrate;        // bitrate of current file (bps)
    unsigned int    Frames;         // number of frames contained
    unsigned int    MaxBand;        // maximum band-index used (0...31)
    unsigned int    IS;             // Intensity Stereo (0: off, 1: on)
    unsigned int    MS;             // Mid/Side Stereo (0: off, 1: on)
    unsigned int    BlockSize;      // only needed for SV4...SV6 -> not supported
    unsigned int    ByteLength;     // length of the file in bytes
    unsigned int    Profile;        // quality profile
    unsigned int    Samplerate;     // sampling frequeny in Hz
    unsigned int    ChannelNumber;  // number of channels
} MPCStreamInfo;

enum tag_t {
    no_tag     = 0,
    ID3v1_tag  = 1,
    APE1_tag   = 2,
    APE2_tag   = 3
};

typedef struct {
    unsigned char  Title     [512];
    unsigned char  Artist    [512];
    unsigned char  Album     [512];
    unsigned char  Comment   [512];
    unsigned char  Year      [ 32];
    unsigned char  TrackStr  [ 32];
    unsigned char  GenreStr  [128];
    int            Track;
    unsigned int   Genre;
    enum tag_t     TagType;
	unsigned int   TagSize;
} MPCTagData;

class CMPCInfo : public CFileInfo {

    public:

    // function to load a file into this structure
    // the argument passed is the path to a MPC file
    int   loadInfo( const char* srcMPC );
    int   loadInfo( const wchar_t* srcMPC );

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

    private:

    // these are the "sub-classes"
	MPCStreamInfo   header;
    MPCTagData      tag;

    // the file information can not be found elsewhere
    wchar_t fileName[260];
    int     fileSize;

};

// From Winamp MPC Plugin

long
JumpID3v2 ( FILE* fp )
{
    unsigned char  tmp [10];
    unsigned int   Unsynchronisation;   // ID3v2.4-flag
    unsigned int   ExtHeaderPresent;    // ID3v2.4-flag
    unsigned int   ExperimentalFlag;    // ID3v2.4-flag
    unsigned int   FooterPresent;       // ID3v2.4-flag
    long           ret;

    fread  ( tmp, 1, sizeof(tmp), fp );

    // check id3-tag
    if ( 0 != memcmp ( tmp, "ID3", 3) )
        return 0;

    // read flags
    Unsynchronisation = tmp[5] & 0x80;
    ExtHeaderPresent  = tmp[5] & 0x40;
    ExperimentalFlag  = tmp[5] & 0x20;
    FooterPresent     = tmp[5] & 0x10;

    if ( tmp[5] & 0x0F )
        return -1;              // not (yet???) allowed
    if ( (tmp[6] | tmp[7] | tmp[8] | tmp[9]) & 0x80 )
        return -1;              // not allowed

    // read HeaderSize (syncsave: 4 * $0xxxxxxx = 28 significant bits)
    ret  = tmp[6] << 21;
    ret += tmp[7] << 14;
    ret += tmp[8] <<  7;
    ret += tmp[9]      ;
    ret += 10;
    if ( FooterPresent )
        ret += 10;

    return ret;
}

int
ReadMPCFileHeader ( const wchar_t* fn, MPCStreamInfo* Info )
{
    const long samplefreqs [4] = { 44100, 48000, 37800, 32000 };
    unsigned int    HeaderData [8];
    unsigned short  EstimatedPeakTitle = 0;
    FILE*           fp;

    memset ( Info, 0, sizeof(MPCStreamInfo) );				// Reset Info-Data
    
    if ( (fp = _wfopen ( fn, L"rb")) != NULL ) {				// load file
        Info->HeaderPosition =  JumpID3v2 (fp);				// get position of first MPC-byte

        fseek ( fp, Info->HeaderPosition, SEEK_SET );		// seek to first byte and read 5 dwords (contains full header except dummy-bytes)
        fread ( HeaderData, 1, sizeof HeaderData, fp );

        fseek ( fp, 0L, SEEK_END );					        // get filelength
        Info->ByteLength = ftell (fp);
    }
    else {
        return 1;                                           // file not found or read-protected
    }
    fclose (fp);

    if ( 0 == memcmp (HeaderData, "MP+", 3) )
        Info->StreamVersion = HeaderData[0] >> 24;

    if ( (Info->StreamVersion & 0x0F) == 7 ) {              // read the file-header (SV7)
        Info->Bitrate       = 0;
        Info->Frames        =  HeaderData[1];
        Info->IS            = 0;
        Info->MS            = (HeaderData[2]>>30) & 0x0001;
        Info->MaxBand       = (HeaderData[2]>>24) & 0x003F;
        Info->BlockSize     = 1;
        Info->Samplerate    = samplefreqs [(HeaderData[2]>>16) & 0x0003];
        Info->ChannelNumber = 2;
        if ( (HeaderData[2]<<8)>>31 ) Info->Profile = (HeaderData[2]<<9)>>29;
        else                          Info->Profile = 6;
    }
    else {
        Info->Bitrate       = (HeaderData[0]>>23) & 0x01FF;		    // read the file-header (SV6 and below)
        Info->IS            = (HeaderData[0]>>22) & 0x0001;
        Info->MS            = (HeaderData[0]>>21) & 0x0001;
        Info->StreamVersion = (HeaderData[0]>>11) & 0x03FF;
        Info->MaxBand       = (HeaderData[0]>> 6) & 0x001F;
        Info->BlockSize     = (HeaderData[0]    ) & 0x003F;
        Info->Profile       = 6;
        Info->Samplerate    = 44100;
        Info->ChannelNumber = 2;
        if (Info->StreamVersion>=5) Info->Frames =  HeaderData[1];      // 32bit
        else                        Info->Frames = (HeaderData[1]>>16); // 16bit

        if ( Info->StreamVersion == 7 ) return 1; 	                    // are there any unsupported parameters used?
        if ( Info->Bitrate       != 0 ) return 1;
        if ( Info->IS            != 0 ) return 1;
        if ( Info->BlockSize     != 1 ) return 1;
    }

    if ( (Info->StreamVersion & 0x0F) < 6 )
        Info->Frames -= 1;

    if ( (Info->StreamVersion & 0x0F) < 4  ||  (Info->StreamVersion & 0x0F) > 7 )
        return 1;

    return 0;
}

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

int GenreToInteger ( const char* GenreStr )
{
    size_t  i;

    if ( strlen (GenreStr) < 2 ) return 255;

    for ( i = 0; i < sizeof(GenreList) / sizeof(*GenreList); i++ ) {
        if ( 0 == _strnicmp ( GenreStr, GenreList[i], strlen (GenreStr) ) )
            return i;
    }

    return 255;
}

void GenreToString ( unsigned char* GenreStr, const int genre )
{
	if ( genre >= 0 && genre < sizeof(GenreList) / sizeof(*GenreList) ) {
		strcpy( (char *)GenreStr, GenreList [genre] );
	} else {
		GenreStr[0] = 0;
	}
}

void memcpy_crop ( char* dst, const char* src, size_t len )
{
    size_t  i;

    for ( i = 0; i < len; i++ )
        if  ( src[i] != '\0' )
            dst[i] = src[i];
        else
            break;

    // dst[i] points behind the string contents
    while ( i > 0  &&  dst [i-1] == ' ' )
        i--;

    dst [i] = '\0';
}

unsigned long Read_LE_Uint32 ( const unsigned char *p )
{
    return ((unsigned long)p[0] <<  0) |
           ((unsigned long)p[1] <<  8) |
           ((unsigned long)p[2] << 16) |
           ((unsigned long)p[3] << 24);
}

void Write_LE_Uint32 ( unsigned char *p , const unsigned long value )
{
	p[0] = (unsigned char)((value >> 0)  & 0xFF);
	p[1] = (unsigned char)((value >> 8)  & 0xFF);
	p[2] = (unsigned char)((value >> 16) & 0xFF);
	p[3] = (unsigned char)((value >> 24) & 0xFF);
}

int ReadID3v1Tags ( const wchar_t* filename, MPCTagData* info )
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

    memcpy_crop  ( (char *)info->Title  , tmp +  3, 30 );
    memcpy_crop  ( (char *)info->Artist , tmp + 33, 30 );
    memcpy_crop  ( (char *)info->Album  , tmp + 63, 30 );
    memcpy_crop  ( (char *)info->Year   , tmp + 93,  4 );
    memcpy_crop  ( (char *)info->Comment, tmp + 97, 30 );

    info->Genre = tmp [127];
    if ( tmp [125] == 0 )
        info->Track = tmp[126];
	GenreToString ( info->GenreStr, info->Genre );
	sprintf( (char *)info->TrackStr, "%d", info->Track );

    return 0;
}

struct APETagFooterStruct
{
    unsigned char   ID       [8];    // should equal 'APETAGEX'
    unsigned char   Version  [4];    // currently 1000 (version 1.000)
    unsigned char   Length   [4];    // the complete size of the tag, including this footer
    unsigned char   TagCount [4];    // the number of fields in the tag
    unsigned char   Flags    [4];    // the tag flags (none currently defined)
    unsigned char   Reserved [8];    // reserved for later use
};

#define TAG_ANALYZE(item,elem)                                      \
    if ( 0 == memicmp (p, #item, sizeof #item ) ) {                 \
        unsigned int tlen;                                          \
        tlen = len < sizeof info->elem ? len : sizeof info->elem-1; \
        p += sizeof #item;                                          \
        memcpy ( info->elem, p, tlen );                             \
        info->elem[tlen] = '\0';                                    \
        p += len;                                                   \
    } else

int ReadAPE1Tags ( const wchar_t* filename, MPCTagData* info )
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
int ConvertUTF8ToANSI ( unsigned char* buffer, const int buf_len )
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

int ReadAPE2Tags ( const wchar_t* filename, MPCTagData* info )
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

long file_size ( const wchar_t* filename )
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

int CMPCInfo::loadInfo ( const char* srcMPC )
{
	return loadInfo(AutoWide(srcMPC));
}

int CMPCInfo::loadInfo ( const wchar_t* srcMPC )
{
    memset ( &tag, 0, sizeof (tag) );
    if ( ReadMPCFileHeader ( srcMPC, &header ) != 0 ) return 1;
    wcscpy(fileName,srcMPC);
    fileSize = file_size(srcMPC);

    enum tag_t _tag = tag.TagType;
    ReadAPE2Tags ( srcMPC, &tag );
    if ( tag.TagType == _tag )
        ReadAPE1Tags ( srcMPC, &tag );
    if ( tag.TagType == _tag )
        ReadID3v1Tags ( srcMPC, &tag );

    return 0;
}

void CMPCInfo::getFileName(char* input) {

    strcpy(input, AutoChar(fileName));

}

int CMPCInfo::getLengthInSeconds()
{
    const int FRAMELEN = 36 * 32;
	return (int)((header.Frames-0.5) * FRAMELEN / ((float)header.Samplerate/1000.) + 0.5f) / 1000;
}

void CMPCInfo::getFormattedLength(char* input) {

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

void CMPCInfo::getTitle(char* input)
{
    // copy result into inputstring
    strcpy(input, (char *)tag.Title);
}

void CMPCInfo::getArtist(char* input)
{
    strcpy(input, (char *)tag.Artist);
}

void CMPCInfo::getAlbum(char* input)
{
    strcpy(input, (char *)tag.Album);
}

void CMPCInfo::getComment(char* input)
{
    strcpy(input, (char *)tag.Comment);
}

void CMPCInfo::getGenre(char* input)
{
    strcpy(input, (char *)tag.GenreStr);
}

int CMPCInfo::getYear()
{
    return atoi((char *)tag.Year);
}

int CMPCInfo::getTrackNumber()
{
    return tag.Track;
}

#endif
