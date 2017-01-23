#ifndef CID3TAG_H
#define CID3TAG_H

#include <windows.h>
#include <stdio.h>

DWORD GetSyncSafeSize(BYTE*);
DWORD GetNonSyncSafeSize(BYTE*);

/* ----------------------------------------------------------
   CId3Tag class is used to retrieve and to save(!!) an
   information from an ID3v1.0/v1.1 tag and load thatinto
   a usable structure.

   This code will be well commented, so that everyone can
   understand, as it's made for the public and not for
   private use, although private use is allowed. :)

   all functions specified both in the header and .cpp file
   will have explanations in both locations.

   everything here by: Gustav "Grim Reaper" Munkby
                       http://home.swipnet.se/grd/
                       grd@swipnet.se
   ---------------------------------------------------------- */


class CId3Tag {

    public:

    // this function takes 128 chars and if it's
    // an ID3 tag structure it'll be read into
    // this usable structure
    BOOL loadTag( char inputtag[128] );
    BOOL loadTagv2( FILE *fp, int pos, int endpos, int tagver );

    // function to set & get Title info [ char[30] ]
    void  getTitle(char* input);

    // function to set & get Artist info [ char[30] ]
    void  getArtist(char* input);
    void  getAlbumArtist(char* input);

    // function to set & get Album info [ char[30] ]
    void  getAlbum(char* input);

    // function to set & get Comment info [ char[30]/char[28] ]
    void  getComment(char* input);

    // the genre info.
    void  getGenre(char* input);

	// unicode version
    void  getTitle(wchar_t* input);
    void  getArtist(wchar_t* input);
    void  getAlbumArtist(wchar_t* input);
    void  getAlbum(wchar_t* input);
    void  getComment(wchar_t* input);
	void  getGenre(wchar_t* input);

    // functions to set & get the integer 
    // for the genre info
    int   getGenreIndex() { return (int)genre; };
    void  setGenreIndex(int input);

    // functions to set & get the year info [1000->3000]
    int   getYear() { return year; };
    void  setYear(int input);

    // functions to set & get the track number [1->255]
    int  getTrackNumber() { return (version==1.1f)?trackNumber:0; };
    void setTrackNumber(int input); 

	// function to get disc number
    int  getDiscNumber() { return discNumber; };

    // function to get the version [1.0/1.1]
    float getVersion() { return version; };

	bool getCoverImage(const char *src, LPBYTE *input, DWORD &dwSize);
	bool getCoverImage(const wchar_t *src, LPBYTE *input, DWORD &dwSize);
	DWORD Unsynch(BYTE *data, DWORD dwSize);

	void ReadStr(int codepage, char *src, DWORD srcSize, wchar_t *dst, DWORD dstSize);

    private:

    wchar_t title   [256],
            artist  [256],
            albumartist  [256],
            album   [256],
            comment [256],
		    genrestr[256];
    int     year;
    int     trackNumber; //ID3v1.1 specific
    float   version;
	int     discNumber;

    UCHAR genre;

	int	  cover_pos;
	DWORD cover_size;
	bool cover_unsynch;
};

#endif