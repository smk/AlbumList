#ifndef _flacinfo_h_
#define _flacinfo_h_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "..\\..\\fileinfo.h"
#include "..\\..\\AutoWide.h"
#include "..\\..\\AutoChar.h"

#include <FLAC/metadata.h>

const int MAX_LEN = 512;

class CFLACInfo : public CFileInfo {

    public:

    // function to load a file into this structure
    // the argument passed is the path to a Flac file
    int   loadInfo( const char* srcFLAC );
    int   loadInfo( const wchar_t* srcFLAC );

    // functions used to get information about the "file"
    int   getFileSize() { return fileSize; };
    void  getFileName(char* input);

    // functions to calculate the length of the song
    // and to present it nicely
    int   getLengthInSeconds();
    void  getFormattedLength(char* input);

    // information that is available in the Flac tag
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

    int ReadID3v1Tags ();

    // these are the "sub-classes"
    int Length;
    char Title[MAX_LEN];
    char Artist[MAX_LEN];
    char Album[MAX_LEN];
    char Comment[MAX_LEN];
    char Genre[MAX_LEN];
    int Year;
    int Track;

    // the file information can not be found elsewhere
    wchar_t fileName[260];
    int fileSize;

};

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

long _flac_file_size ( const wchar_t* filename )
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

// Convert UTF-8 coded string to UNICODE
// Return number of characters converted
int utf8ToUnicode ( const char* lpMultiByteStr, WCHAR* lpWideCharStr, int cmbChars )
{
    const unsigned char*    pmb = (unsigned char  *)lpMultiByteStr;
    unsigned short*         pwc = (unsigned short *)lpWideCharStr;
    const unsigned char*    pmbe;
    size_t  cwChars = 0;

    if ( cmbChars >= 0 ) {
        pmbe = pmb + cmbChars;
    } else {
        pmbe = NULL;
    }

    while ( (pmbe == NULL) || (pmb < pmbe) ) {
        char            mb = *pmb++;
        unsigned int    cc = 0;
        unsigned int    wc;

        while ( (cc < 7) && (mb & (1 << (7 - cc)))) {
            cc++;
        }

        if ( cc == 1 || cc > 6 )                    // illegal character combination for UTF-8
            continue;

        if ( cc == 0 ) {
            wc = mb;
        } else {
            wc = (mb & ((1 << (7 - cc)) - 1)) << ((cc - 1) * 6);
            while ( --cc > 0 ) {
                if ( pmb == pmbe )                  // reached end of the buffer
                    return cwChars;
                mb = *pmb++;
                if ( ((mb >> 6) & 0x03) != 2 )      // not part of multibyte character
                    return cwChars;
                wc |= (mb & 0x3F) << ((cc - 1) * 6);
            }
        }

        if ( wc & 0xFFFF0000 )
            wc = L'?';
        *pwc++ = wc;
        cwChars++;
        if ( wc == L'\0' )
            return cwChars;
    }

    return cwChars;
}

void tag_insert ( char* buffer, const char* value, UINT len, UINT maxlen )
{
    char* p;
    WCHAR wValue[MAX_LEN];
    char temp[MAX_LEN];
    UINT c;

    if ( len >= maxlen ) len = maxlen-1;
    if ( (c = utf8ToUnicode (value, wValue, len)) <= 0 ) return;
    if ( wValue[c] != L'\0' ) wValue[c++] = L'\0';
    if ( (c = WideCharToMultiByte (CP_ACP, 0, wValue, c, temp, MAX_LEN, NULL, NULL)) == 0 ) return;

    if ( *buffer == '\0' ) {    // new value
        p = buffer;
    } else {                    // append to existing value
        p = strchr (buffer, '\0' );
        p += sprintf ( p, ", " );
    }

    if ( (p-buffer) + c >= maxlen ) c = maxlen - (p-buffer) - 1;
    strncpy ( p, temp, c );
    p[c] = '\0';
}

void memcpy_crop ( char* dst, const char* src, UINT len )
{
    for ( UINT i = 0; i < len; i++ )
        if  ( src[i] != '\0' ) dst[i] = src[i]; else break;

    // dst[i] points behind the string contents
    while ( i > 0  &&  dst [i-1] == ' ' )
        i--;

    dst [i] = '\0';
}

void GenreToString ( char* GenreStr, const int genre )
{
	if ( genre >= 0 && genre < sizeof(GenreList) / sizeof(*GenreList) ) {
		strcpy( (char *)GenreStr, GenreList [genre] );
	} else {
		GenreStr[0] = '\0';
	}
}

CFLACInfo::ReadID3v1Tags () {

    FILE*  fp;
    char   tmp [128];

    if ( (fp = _wfopen (fileName, L"rb")) == NULL )
        return 1;

    fseek  ( fp, -128, SEEK_END );
    fread  ( tmp, 1, sizeof tmp, fp );
    fclose ( fp );

    // check for id3-tag
    if ( memcmp (tmp, "TAG", 3) != NULL )
        return 0;

    memcpy_crop  ( Title  , tmp +  3, 30 );
    memcpy_crop  ( Artist , tmp + 33, 30 );
    memcpy_crop  ( Album  , tmp + 63, 30 );
    memcpy_crop  ( Comment, tmp + 97, 30 );
    Year = atoi ( tmp + 93 );
    if ( tmp [125] == 0 ) Track = tmp[126];
	GenreToString ( Genre, tmp [127] );

    return 0;
}

CFLACInfo::loadInfo ( const char* srcFLAC )
{
	return loadInfo (AutoWide(srcFLAC));
}

CFLACInfo::loadInfo ( const wchar_t* srcFLAC )
{
    FLAC__StreamMetadata info;

    wcsncpy (fileName, srcFLAC, 260);
    fileSize = _flac_file_size (srcFLAC);

    Title[0]   = '\0';
    Artist[0]  = '\0';
    Album[0]   = '\0';
    Comment[0] = '\0';
    Genre[0]   = '\0';
    Year       =   0;
    Track      =   0;
    Length     =   0;

	AutoChar src(srcFLAC);

    if ( !FLAC__metadata_get_streaminfo (src, &info) )
        return 1;
    if ( info.type != FLAC__METADATA_TYPE_STREAMINFO )
        return 1;

    Length = (info.data.stream_info.sample_rate > 0)  ?  (unsigned long)(info.data.stream_info.total_samples / info.data.stream_info.sample_rate)  :  0;

    FLAC__Metadata_SimpleIterator* si;

    si = FLAC__metadata_simple_iterator_new ();
    if ( !FLAC__metadata_simple_iterator_init (si, src, 0, 0) ) {
        FLAC__metadata_simple_iterator_delete ( si );
        ReadID3v1Tags ();
        return 0;
    }

    do {
        FLAC__MetadataType      type;
        FLAC__StreamMetadata*   data;

        type = FLAC__metadata_simple_iterator_get_block_type ( si );

        // Vorbis comment block
        if ( type == FLAC__METADATA_TYPE_VORBIS_COMMENT ) {
            data = FLAC__metadata_simple_iterator_get_block ( si );

            if ( data != NULL ) {
                FLAC__StreamMetadata_VorbisComment_Entry*   vc = data->data.vorbis_comment.comments;

                for ( UINT i = 0; i < data->data.vorbis_comment.num_comments; i++ ) {
                    if ( vc[i].entry == NULL || vc[i].entry[0] == '\0' )  // empty field
                        continue;

                    UINT item_len = 0;
                    while ( vc[i].entry[item_len] != '=' && vc[i].entry[item_len] != '\0' )
                        item_len++;
                    if ( vc[i].entry[item_len] == '\0' || vc[i].entry[item_len + 1] == '\0' )
                        continue;

                    UINT value_len = vc[i].length - item_len - 1;
                    char *value = (char *)(vc[i].entry + item_len + 1);

                    switch ( item_len ) {
                    case 4:
                        if ( !strnicmp ((char *)vc[i].entry, "DATE", item_len) ) {
                            Year = atoi ( value );
                        }
                        break;
                    case 5:
                        if ( !strnicmp ((char *)vc[i].entry, "TITLE", item_len) ) {
                            tag_insert ( Title, value, value_len, sizeof(Title) );
                        } else
                        if ( !strnicmp ((char *)vc[i].entry, "ALBUM", item_len) ) {
                            tag_insert ( Album, value, value_len, sizeof(Album) );
                        } else
                        if ( !strnicmp ((char *)vc[i].entry, "GENRE", item_len) ) {
                            tag_insert ( Genre, value, value_len, sizeof(Genre) );
                        }
                        break;
                    case 6:
                        if ( !strnicmp ((char *)vc[i].entry, "ARTIST", item_len) ) {
                            tag_insert ( Artist, value, value_len, sizeof(Artist) );
                        }
                        break;
                    case 7:
                        if ( !strnicmp ((char *)vc[i].entry, "COMMENT", item_len) ) {
                            tag_insert ( Comment, value, value_len, sizeof(Comment) );
                        }
                        break;
                    case 11:
                        if ( !strnicmp ((char *)vc[i].entry, "TRACKNUMBER", item_len) ) {
                            Track = atoi ( value );
                        }
                        break;
                    }
                }
                
                FLAC__metadata_object_delete ( data );
            }
        }
    } while ( FLAC__metadata_simple_iterator_next (si) );

    FLAC__metadata_simple_iterator_delete ( si );

    if ( !Title[0] || !Artist[0] || !Album[0] ) {
        ReadID3v1Tags ();
    }

    return 0;
}

void CFLACInfo::getFileName(char* input) {

    strcpy(input, AutoChar(fileName));

}

int CFLACInfo::getLengthInSeconds()
{
    return Length;
}

void CFLACInfo::getFormattedLength(char* input) {

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

void CFLACInfo::getTitle(char* input)
{
    // copy result into inputstring
    strcpy(input, (char *)Title);
}

void CFLACInfo::getArtist(char* input)
{
    strcpy(input, (char *)Artist);
}

void CFLACInfo::getAlbum(char* input)
{
    strcpy(input, (char *)Album);
}

void CFLACInfo::getComment(char* input)
{
    strcpy(input, (char *)Comment);
}

void CFLACInfo::getGenre(char* input)
{
    strcpy(input, (char *)Genre);
}

int CFLACInfo::getYear()
{
    return Year;
}

int CFLACInfo::getTrackNumber()
{
    return Track;
}

#endif
