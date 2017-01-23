#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "CMP3Info.h"
#include "Gen_m3a.h"
#include "AutoChar.h"
#include "AutoWide.h"

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

#define ERR_FILEOPEN    0x0001
#define ERR_NOSUCHFILE  0x0002
#define ERR_NOMP3FILE   0x0004
#define ERR_ID3TAG      0x0008

int CMP3Info::loadInfo( const char *srcMP3 )
{
	return loadInfo(AutoWide(srcMP3));
}

int CMP3Info::loadInfo( const wchar_t *srcMP3 )
{
    // open input-file stream to the specified file, name
	FILE *fp = _wfopen (srcMP3, L"rb");

//	DbgPrint("File = %S\n", srcMP3);
    
    if (fp)	// if the file was opened correctly
	{ 
        // get file size, by setting the pointer in the end and tell the position
        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);

        // get srcMP3 into fileName variable
        wcscpy(fileName, srcMP3);

        int pos = 0; // current position in file...

        /******************************************************/
        /* search and skip ID3v2 tag if exist at the beginning*/
        /******************************************************/

		char id3v2[4];
		id3v2[3] = 0;

		// try find id3v2.3 tag at the beginning of the file
        do
		{
            // if no header has been found after 1024Bytes
            // or the end of the file has been reached
            // then there's probably no id3v2 tag
            if ( pos>(1024) || feof(fp) )
			{
				break;
            }

            // read in four characters
            fseek(fp, pos, SEEK_SET);
            fread(id3v2, sizeof(char), 3, fp);

            // move file-position forward
            pos++;
        }
        while ( memcmp(id3v2, "ID3", 3) != 0 );  // test for correct header

		// couldn't find it at the beginning of the file?
		// try the end for id3v2.4
		if (memcmp(id3v2, "ID3", 3) != 0)
		{
			pos = 10;
			do
			{
				// if no header has been found after 2048Bytes
				// or the end of the file has been reached
				// then there's probably no id3v2 tag
				if ( pos>(2048) )
				{
					break;
				}

				// read in four characters
				fseek(fp, -pos, SEEK_END);
				fread (id3v2, sizeof(char), 3, fp);

				// move file-position forward
				pos++;
			}
			while ( memcmp(id3v2, "ID3", 3) != 0 );  // test for correct header
		}

		// ID3v2/file identifier      "ID3"		(3bytes)
		// ID3v2 version              $04 00	(2bytes)
		// ID3v2 flags                %abcd0000	(1byte)
		// ID3v2 size             4 * %0xxxxxxx (4bytes)
		int id3v2pos = -1;
		int endpos = 0;
		int version = 3;
		if (memcmp(id3v2, "ID3", 3) == 0)
		{
			pos = ftell(fp);

			// read version
			BYTE tagVersion[2];
			fread ((char *)tagVersion, sizeof(char), 2, fp);

			version = tagVersion[0];

			// read ID3v2 flags
			BYTE tagFlags[1];
			fread ((char *)tagFlags, sizeof(char), 1, fp);

			bool bExtendedHeader = ((tagFlags[0] & 0x40) == 0x40);

			// read ID3v2 size
			BYTE tagSize[4];
			fread ((char *)tagSize, sizeof(char), 4, fp);

			DWORD dwSize = GetSyncSafeSize(&tagSize[0]);

			pos = ftell(fp);
			endpos = pos + dwSize;
			fileSize -= dwSize + 10;

			// skip the extended header if present
			if (bExtendedHeader)
			{
				fseek(fp, pos, SEEK_SET);
				fread ((char *)tagSize, sizeof(char), 4, fp);
				if (version > 3)
					pos += (GetSyncSafeSize(&tagSize[0]) - 4);
				else
					pos += GetNonSyncSafeSize(&tagSize[0]);
			}

			id3v2pos = pos;		// update ID3v2 frame position in file
			pos = endpos;
		}
		else
		{
			pos = 0;
			fseek(fp, pos, SEEK_SET);
		}

        /******************************************************/
        /* search and load the first frame-header in the file */
        /******************************************************/
        
        char headerchars[4]; // char variable used for header-loading

		int curpos = pos;
        do {
            // if no header has been found after 200kB
            // or the end of the file has been reached
            // then there's probably no mp3-file
            if ( (pos-curpos)>(1024*200) || feof(fp) ) {
                fclose(fp);
                return ERR_NOMP3FILE;
            }

            // read in four characters
            fseek(fp, pos, SEEK_SET);
            fread (headerchars, sizeof(char), 4, fp);

            // move file-position forward
            pos++;
            
            // convert four chars to CFrameHeader structure
            header.loadHeader(headerchars);

        }
        while ( !header.isValidHeader() );  // test for correct header

        // to correct the position to be right after the frame-header
        // we'll need to add another 3 to the current position
        pos += 3;
		int posheader = pos;

        /******************************************************/
        /* check for an vbr-header, to ensure the info from a */
        /* vbr-mp3 is correct                                 */
        /******************************************************/

        char vbrchars[12];
        
        // determine offset from first frame-header
        // it depends on two things, the mpeg-version
        // and the mode(stereo/mono)

        if( header.getVersionIndex()==3 ) {  // mpeg version 1

            if( header.getModeIndex()==3 ) pos += 17; // Single Channel
            else                           pos += 32;

        } else {                             // mpeg version 2 or 2.5

            if( header.getModeIndex()==3 ) pos +=  9; // Single Channel
            else                           pos += 17;

        }

        // read next twelve bits in
        fseek(fp, pos, SEEK_SET);
        fread (vbrchars, sizeof(char), 12, fp);

        // turn 12 chars into a CVBitRate class structure
        VBitRate = vbr.loadHeader(vbrchars);        

        /******************************************************/
        /* get freeformat bitrate if we didn't get any above  */
        /******************************************************/

		FreeForm = FALSE;
		int ff = wndWinampAL.GetSetting(settingFreeFormMP3);
		if (ff && getBitrate() == 0)
		{
			// find next header
			CFrameHeader header2;

			curpos = pos = posheader;
			BOOL bError = FALSE;
			do {
				// if no header has been found after 200kB
				// or the end of the file has been reached
				// then there's probably no mp3-file
				if ( (pos-curpos)>(1024*200) || feof(fp) ) {
					bError = TRUE;
					break;
				}

				// read in four characters
				fseek(fp, pos, SEEK_SET);
				fread (headerchars, sizeof(char), 4, fp);

				// move file-position forward
				pos++;
        
				// convert four chars to CFrameHeader structure
				header2.loadHeader(headerchars);

			}
			while ( !header2.isValidHeader() );  // test for correct header

			if (!bError)
			{
				int N = pos + 3 - posheader;

				int pad_slot = header2.getPaddingBit() ? 1 : 0;
				int slots_per_frame = ((header2.getLayer() == 3) && (header2.getVersion() == 2.5)) ? 72 : 144;

				if (header2.getLayer() == 1)
				{
					FreeFormBitrate = (unsigned long) header2.getFrequency() * (N - 4 * pad_slot + 4) / 48 / 1000;
				}
				else
				{
					FreeFormBitrate = (unsigned long) header2.getFrequency() * (N - pad_slot + 1) / slots_per_frame / 1000;
				}

				if (FreeFormBitrate >= 8)
				{
					FreeForm = TRUE;
				}
			}
		}

        /******************************************************/
        /* get tag from the last 128 bytes in an .mp3-file    */
        /******************************************************/
        
        char tagchars[128]; // load tag as string

        // get last 128 bytes
        fseek (fp, -128, SEEK_END );
        fread ( tagchars, sizeof(char), 128, fp );

        // turn 128 chars into a CId3Tag structure
        Tagged = tag.loadTag(tagchars);

		if (Tagged) {
			fileSize -= 128;
		}
		
		if (id3v2pos != -1) {
			tag.loadTagv2(fp, id3v2pos, endpos, version);
		}
    }
    else {
        return ERR_NOSUCHFILE;
    }

    fclose(fp);
    return 0;

}

int CMP3Info::getBitrate() {

	if (FreeForm)
	{
		return FreeFormBitrate;
	}
    else if (VBitRate) {

        // get average frame size by deviding fileSize by the number of frames
		if (getNumberOfFrames() == 0) return 0;

        float medFrameSize = (float)fileSize / (float)getNumberOfFrames();
        
        /* Now using the formula for FrameSizes which looks different,
           depending on which mpeg version we're using, for mpeg v1:
        
           FrameSize = 12 * BitRate / SampleRate + Padding (if there is padding)

           for mpeg v2 the same thing is:

           FrameSize = 144 * BitRate / SampleRate + Padding (if there is padding)

           remember that bitrate is in kbps and sample rate in Hz, so we need to
           multiply our BitRate with 1000.

           For our purpose, just getting the average frame size, will make the
           padding obsolete, so our formula looks like:

           FrameSize = (mpeg1?12:144) * 1000 * BitRate / SampleRate;
        */

        return (int)( 
                     ( medFrameSize * (float)header.getFrequency() ) / 
                     ( 1000.0 * ( (header.getLayerIndex()==3) ? 12.0 : 144.0))
                    );

    }
    else return header.getBitrate();

}

int CMP3Info::getLengthInSeconds() {

    // kiloBitFileSize to match kiloBitPerSecond in bitrate...
    int kiloBitFileSize = (8 * fileSize) / 1000;

	int bitrate = getBitrate();
	if (bitrate == 0) return 0;

    return (int)(kiloBitFileSize/bitrate);
}

void CMP3Info::getFormattedLength(char* input) {

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

int CMP3Info::getNumberOfFrames() {

    if (!VBitRate) {

        /* Now using the formula for FrameSizes which looks different,
           depending on which mpeg version we're using, for layer 1:
        
           FrameSize = 12 * BitRate / SampleRate + Padding (if there is padding)

           for layer 2 & 3 the same thing is:

           FrameSize = 144 * BitRate / SampleRate + Padding (if there is padding)

           remember that bitrate is in kbps and sample rate in Hz, so we need to
           multiply our BitRate with 1000.

           For our purpose, just getting the average frame size, will make the
           padding obsolete, so our formula looks like:

           FrameSize = (layer1?12:144) * 1000 * BitRate / SampleRate;
        */
		if (header.getFrequency() == 0) return 0;
           
        float medFrameSize = (float)( 
                                     ( (header.getLayerIndex()==3) ? 12 : 144 ) *
                                     (
                                      (1000.0 * (float)header.getBitrate() ) /
                                      (float)header.getFrequency()
                                     )
                                    );
        
        return (int)(fileSize/medFrameSize);

    }
    else return vbr.getNumberOfFrames();

}

void CMP3Info::getVersion(char* input) {

    char versionchar[32]; // temporary string
    char tempchar2[4]; // layer

    // call CFrameHeader member function
    float ver = header.getVersion();

    // create the layer information with the amounts of I
    for( int i=0; i<header.getLayer(); i++ ) tempchar2[i] = 'I';
    tempchar2[i] = '\0';

    // combine strings
    sprintf(versionchar,"MPEG %g Layer %s", (double)ver, tempchar2);

    // copy result into inputstring
    strcpy(input, versionchar);

}

void CMP3Info::getMode(char* input) {

    char modechar[32]; // temporary string

    // call CFrameHeader member function
    header.getMode(modechar);

    // copy result into inputstring
    strcpy(input, modechar);

}

bool CMP3Info::getCoverImage(LPBYTE *input, DWORD &dwSize)
{
	return tag.getCoverImage(fileName, input, dwSize);
}

void CMP3Info::getTitle(char* input)
{
    tag.getTitle(input);
}

void CMP3Info::getTitle(wchar_t* input)
{
    tag.getTitle(input);
}

void CMP3Info::getArtist(char* input)
{
    tag.getArtist(input);
}

void CMP3Info::getAlbumArtist(char* input)
{
    tag.getAlbumArtist(input);
}

void CMP3Info::getArtist(wchar_t* input)
{
    tag.getArtist(input);
}

void CMP3Info::getAlbumArtist(wchar_t* input)
{
    tag.getAlbumArtist(input);
}

void CMP3Info::getAlbum(char* input)
{
    tag.getAlbum(input);
}

void CMP3Info::getAlbum(wchar_t* input)
{
    tag.getAlbum(input);
}

void CMP3Info::getComment(char* input)
{
    tag.getComment(input);
}

void CMP3Info::getComment(wchar_t* input)
{
    tag.getComment(input);
}

void CMP3Info::getGenre(char* input)
{
    tag.getGenre(input);
}

void CMP3Info::getGenre(wchar_t* input)
{
    tag.getGenre(input);
}

void CMP3Info::getFileName(char* input)
{
    if (input) strcpy(input, AutoChar(fileName));
}

void CMP3Info::getFileName(wchar_t* input)
{
	if (input) wcscpy(input, fileName);
}
