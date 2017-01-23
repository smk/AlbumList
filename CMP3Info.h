#ifndef CMP3INFO_H
#define CMP3INFO_H

#include <windows.h>
#include "FileInfo.h"
#include "CFrameHeader.h"
#include "CId3Tag.h"
#include "CVBitRate.h"

/* ----------------------------------------------------------
   CMP3Info class is your complete guide to the 
   MP3 file format in the C++ language. It's a large class
   with three different, quite large "sub-classes" so it's
   a fair amount of code to look into.

   This code will be well commented, so that everyone can
   understand, as it's made for the public and not for
   private use, although private use is allowed. :)

   all functions specified both in the header and .cpp file
   will have explanations in both locations.

   everything here by: Gustav "Grim Reaper" Munkby
                       http://home.swipnet.se/grd/
                       grd@swipnet.se
   ---------------------------------------------------------- */


class CMP3Info : public CFileInfo {

    public:

    // function to load a file into this structure
    // the argument passed is the path to a MP3 file
    int   loadInfo( const char* srcMP3 );
    int   loadInfo( const wchar_t* srcMP3 );

    // function to save tag
    int   saveTag( char* title, char* artist, char* album, char* year, char* comment, char* tracknumber, char* genre);

    // functions used to get information about the "file"
    int   getFileSize() { return fileSize; };
    void  getFileName(char* input);
    void  getFileName(wchar_t* input);

    // information that is avaliable in FrameHeader & VBR header
    void  getVersion(char* input);
    int   getBitrate();
    int   getFrequency() { return header.getFrequency(); };
    void  getMode(char* input);
    
    int   getNumberOfFrames();

    // functions to calculate the length of the song
    // and to present it nicely
    int   getLengthInSeconds();
    void  getFormattedLength(char* input);

    // information that is avaliable in the ID3 tag
    void  getTitle(char* input);
    void  getArtist(char* input);
    void  getAlbumArtist(char* input);
    void  getAlbum(char* input);
    void  getComment(char* input);
    void  getGenre(char* input);

	void  getTitle(wchar_t* input);
    void  getArtist(wchar_t* input);
    void  getAlbumArtist(wchar_t* input);
    void  getAlbum(wchar_t* input);
    void  getComment(wchar_t* input);
    void  getGenre(wchar_t* input);

    int   getYear() { return tag.getYear(); };
    int   getTrackNumber() { return tag.getTrackNumber(); };
	int   getDiscNumber() { return tag.getDiscNumber(); };

    // just to know what kind of file it is.
    BOOL  isVBitRate() { return VBitRate; };
    BOOL  isTagged() { return Tagged; };

	bool  getCoverImage(LPBYTE *input, DWORD &dwSize);
    

    private:

    // this alerts some error messages, so that it's
    // easier to see what's wrong
    void showErrorMessage(UINT msg, const char* szMsgPlus = "");

    // these are the "sub-classes"
    CFrameHeader header;
    CVBitRate    vbr;
    CId3Tag      tag;

    // just to know what kind of file it is
    BOOL VBitRate;
    BOOL Tagged;

	// FreeForm MP3s
	BOOL FreeForm;
	int FreeFormBitrate;

    // the file information can not be found elsewhere
    wchar_t fileName[MAX_PATH];
    int fileSize;

};

#endif
