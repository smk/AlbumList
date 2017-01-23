#include "stdafx.h"
#include "CId3Tag.h"
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>
#include "resource.h"
#include "Gen_m3a.h"
#include "AutoWide.h"
#include "AutoChar.h"

#define mmioFOURCC(ch0, ch1, ch2, ch3) ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))

#define TPE2	mmioFOURCC('2','E','P','T')
#define TPE1	mmioFOURCC('1','E','P','T')
#define TALB	mmioFOURCC('B','L','A','T')
#define TIT2	mmioFOURCC('2','T','I','T')
#define TRCK	mmioFOURCC('K','C','R','T')
#define TPOS	mmioFOURCC('S','O','P','T')
#define TYER	mmioFOURCC('R','E','Y','T')
#define TDAT	mmioFOURCC('T','A','D','T')
#define TDRC	mmioFOURCC('C','R','D','T')
#define SEEK	mmioFOURCC('K','E','E','S')
#define APIC	mmioFOURCC('C','I','P','A')
#define TCON    mmioFOURCC('N','O','C','T')
#define PRIV    mmioFOURCC('V','I','R','P')

#define TT2	    mmioFOURCC( 0 ,'2','T','T')
#define TP2	    mmioFOURCC( 0 ,'1','P','T')
#define TAL	    mmioFOURCC( 0 ,'L','A','T')
#define TRK	    mmioFOURCC( 0 ,'K','R','T')
#define TPA		mmioFOURCC( 0 ,'A','P','T')
#define TYE	    mmioFOURCC( 0 ,'E','Y','T')
#define PIC	    mmioFOURCC( 0 ,'C','I','P')
#define TCO	    mmioFOURCC( 0 ,'O','C','T')

DWORD GetSyncSafeSize(BYTE *b)
{
#ifdef _DEBUG
	if ((b[0] | b[1] | b[2] | b[3]) & 0x80)
	{
		DbgPrint("SyncSafe tag contains non SyncSafe integer\n");
	}
#endif

	return  ((b[0] & 0x7f)<<21) +
			((b[1] & 0x7f)<<14) +
			((b[2] & 0x7f)<<7)  +
			 (b[3] & 0x7f);
}

DWORD GetNonSyncSafeSize(BYTE *b)
{
	return  ((DWORD)b[0] << 24) +
			((DWORD)b[1] << 16) +
			((DWORD)b[2] << 8)  +
					b[3];
}

DWORD GetNonSyncSafeSize3Bytes(BYTE *b)
{
	return  ((DWORD)b[0] << 16) +
			((DWORD)b[1] << 8)  +
					b[2];
}

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

// this function is specially designed to take
// care of ID3 tags generated with spaces instead
// of null characters as they are supposed to.
void gStrCpy( char* dst, char* src, int length ) {

    // what this does is that it searches for a
    // the first non-space or non-null from the
    // right side (and while doing so replacing)
    // " " with '\0'
    for	( int i = (length-1); i >= 0; i-- ) {

        if ( (src[i] == 0x20) || (src[i] == '\0') ) dst[i] = '\0';

        else break;
            
    }

    // this should copy the remaining part straight over
    memcpy(dst, src, (i+1) );

    // set last byte to zero to ensure string compability
    dst[length] = '\0';

}


BOOL CId3Tag::loadTag( char inputtag[128] ) {

	// initialize everything first
    memset(title, 0, 256);
	memset(artist, 0, 256);
	memset(albumartist, 0, 256);
	memset(album, 0, 256);
	memset(comment, 0, 256);
	memset(genrestr, 0, 256);
    year = 2000;
    trackNumber = 0;
	discNumber = 0;
    version = 1.0;
    genre = 255;
	cover_pos = 0;
	cover_size = 0;
	cover_unsynch = false;

    // in the ID3 tag the first 3 bytes are always TAG
    // so that you can separate a TAGed file from a
    // non TAGed one
    if ( memcmp(inputtag, "TAG", 3) ) return false;

	char str[128];
    // using special string-copying routine
    // to copy the title string
    gStrCpy( str, inputtag+3, 30 );
	wcscpy(title, AutoWide(str, wndWinampAL.GetEncodingCodepage()));

    // using special string-copying routine
    // to copy the artist string
    gStrCpy( str, inputtag+33, 30 );
	wcscpy(artist, AutoWide(str, wndWinampAL.GetEncodingCodepage()));

    // using special string-copying routine
    // to copy the title string
    gStrCpy( str, inputtag+63, 30 );
	wcscpy(album, AutoWide(str, wndWinampAL.GetEncodingCodepage()));

    // because I store year as an integer
    // and the ID3 tag stores it as a string
    // the year will be handled like this:
    char yearstr[5] = { 0, 0, 0, 0, 0 };
    memcpy( yearstr, inputtag+93, 4);
    year = atoi( yearstr );

    // now lets detect whether it's version 1.1
    // or 1.0 (if there is tracknumber info or not)
    // if there is some tracknumber info, then it's in
    // the last char of the comment field, and the
    // one before is null
    if ( (inputtag[125]=='\0' ) && (inputtag[126] != '\0') ) {

        version = (float)1.1;
        trackNumber = (int)inputtag[126];

        // using special string-copying routine
        // to copy the comment string
        gStrCpy(str, inputtag+97, 28);
		wcscpy(comment, AutoWide(str, wndWinampAL.GetEncodingCodepage()));

    } else {
        
        version = (float)1.0;
		trackNumber = 0;

        // using special string-copying routine
        // to copy the title string
        gStrCpy(str, inputtag+97, 30);
		wcscpy(comment, AutoWide(str, wndWinampAL.GetEncodingCodepage()));

    }

    genre = inputtag[127];

    return true;

}

BOOL CId3Tag::loadTagv2(FILE *fp, int pos, int endpos, int tagver )
{
	if (fp == NULL) return FALSE;

	int codepage = CP_ACP;

	int encoding = wndWinampAL.GetSetting(settingEncodingLangAL);
	switch (encoding)
	{
	case langTraditionalChinese:	codepage = 950;	break;
	case langSimplifiedChinese:		codepage = 936;	break;
	case langJapanese:				codepage = 932;	break;
	case langKorean:				codepage = 949;	break;
	}

	// read frames
	while (pos < endpos)
	{
		char *str = NULL, *ptr = NULL;
		BYTE tagFrameHeader[10];
		DWORD dwSize = 0;
		DWORD dwFrameID = 0;
		DWORD dwFrameSize = 0;
		BOOL bUnsynch = FALSE;
		fseek(fp, pos, SEEK_SET);

		if (tagver == 4)		// id3v2.4
		{
			fread ((char *)tagFrameHeader, sizeof(char), 10, fp);
			pos += 10;

			// padding starts
			if (tagFrameHeader[0] == 0) break;

			// id3v2.4 uses synchsafe integers (28bits)
			dwSize = GetSyncSafeSize(&tagFrameHeader[4]);
			dwFrameID = mmioFOURCC(tagFrameHeader[3],tagFrameHeader[2],tagFrameHeader[1],tagFrameHeader[0]);

			// Data length indicator
			if (tagFrameHeader[9] & 0x1)
			{
				BYTE tagFrameSize[4];
				fread (tagFrameSize, sizeof(char), 4, fp);
				pos += 4;
				
				dwFrameSize = GetSyncSafeSize(tagFrameSize);
			}
			if (tagFrameHeader[9] & 0x2)
			{
				bUnsynch = TRUE;
			}
		}
		else if (tagver == 3)	// id3v2.3
		{
			fread ((char *)tagFrameHeader, sizeof(char), 10, fp);
			pos += 10;

			// padding starts
			if (tagFrameHeader[0] == 0) break;

			// id3v2.3 uses regular integers (32bits)
			dwSize = GetNonSyncSafeSize(&tagFrameHeader[4]);
			dwFrameID = mmioFOURCC(tagFrameHeader[3],tagFrameHeader[2],tagFrameHeader[1],tagFrameHeader[0]);
		}
		else if (tagver == 2)	// id3v2.2
		{
			fread ((char *)tagFrameHeader, sizeof(char), 6, fp);
			pos += 6;

			// padding starts
			if (tagFrameHeader[0] == 0) break;

			// id3v2 uses regular integers (24bits)
			dwSize = GetNonSyncSafeSize3Bytes(&tagFrameHeader[3]);
			dwFrameID = mmioFOURCC(0,tagFrameHeader[2],tagFrameHeader[1],tagFrameHeader[0]);
		}
		else
		{
			return false;
		}

//		DbgPrint("FrameID = %c%c%c%c\n", tagFrameHeader[0],tagFrameHeader[1],tagFrameHeader[2],tagFrameHeader[3]);

		switch (dwFrameID)
		{
		case TPE2:	// Album Artist
			str = new char [dwSize+2];
			str[dwSize] = 0;
			str[dwSize+1] = 0;
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
			ReadStr(codepage, str, dwSize, albumartist, 127);
			delete [] str;
			break;

		case TPE1:	// Lead performer(s)/Soloist(s)
		case TP2:	// Lead performer(s)/Soloist(s)
			str = new char [dwSize+2];
			str[dwSize] = 0;
			str[dwSize+1] = 0;
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
			ReadStr(codepage, str, dwSize, artist, 127);
			delete [] str;
			break;

		case TALB:	// Album/Movie/Show title
		case TAL:	// Album/Movie/Show title
			str = new char [dwSize+2];
			str[dwSize] = 0;
			str[dwSize+1] = 0;
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
			ReadStr(codepage, str, dwSize, album, 127);
			delete [] str;
			break;

		case TIT2:	// Title/songname/content description
		case TT2:	// Title/songname/content description
			str = new char [dwSize+2];
			str[dwSize] = 0;
			str[dwSize+1] = 0;
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
			ReadStr(codepage, str, dwSize, title, 127);
			delete [] str;
			break;

		case TRCK:	// Track number/Position in set
		case TRK:	// Track number/Position in set
			str = new char [dwSize+2];
			str[dwSize] = 0;
			str[dwSize+1] = 0;
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
			version = (float)1.1;
			ptr = str+1;
			// 2005-01-23 Modified T-Matsuo
//			ptr = strchr(ptr, '/');
			ptr = (char *)_mbschr((const unsigned char *)ptr, '/');
			if (ptr) *ptr = 0;
			// ansi string
			if (str[0] == 0) trackNumber = atoi(str+1);
			// unicode string
			else if (str[0] == 1) trackNumber = _wtoi((wchar_t*)(str+3));
			delete [] str;
			break;

		case TCON:
		case TCO:	// Content type / Genre
			str = new char [dwSize+2];
			str[dwSize] = 0;
			str[dwSize+1] = 0;
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
			ReadStr(codepage, str, dwSize, genrestr, 127);
			// id3v1.1 style
			if (genrestr[0] == L'(') 
			{
				wchar_t *ptr = wcschr(genrestr, L')');
				if (ptr) *ptr = 0;
				genre = _wtoi(genrestr+1);
				genrestr[0] = 0;
			}
			delete [] str;
			break;

		case TPOS:	// Part of a set (disc 1/3)
		case TPA:	// Part of a set (disc 1/3)
			str = new char [dwSize+2];
			str[dwSize] = 0;
			str[dwSize+1] = 0;
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
			ptr = str+1;
			ptr = (char *)_mbschr((const unsigned char *)ptr, '/');
			if (ptr) *ptr = 0;
			// ansi string
			if (str[0] == 0) discNumber = atoi(str+1);
			// unicode string
			else if (str[0] == 1) discNumber = _wtoi((wchar_t*)(str+3));
			delete [] str;
			break;

		case TYER:	// Year
		case TYE:	// Year
			str = new char [dwSize+2];
			str[dwSize] = 0;
			str[dwSize+1] = 0;
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
		    // ansi string
			if (str[0] == 0) year = atoi( str+1 );
			// unicode string
			else if (str[0] == 1) year = _wtoi((wchar_t*)(str+3));
			delete [] str;
			break;

		case TDRC:	// Recording time
			str = new char [dwSize+2];
			str[dwSize] = 0;
			str[dwSize+1] = 0;
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
		    // ansi string
			if (str[0] == 0) year = atoi( str+1 );
			// unicode string
			else if (str[0] == 1) year = _wtoi((wchar_t*)(str+3));
			delete [] str;
			break;

		case SEEK:
			str = new char [dwSize];
			pos += GetNonSyncSafeSize((LPBYTE)&str[0]);
			delete [] str;
			break;

		case APIC:
			// Text encoding	$xx
			// MIME type		<text string> $00
			// Picture type 	$xx
			// Description		<text string according to encoding> $00 (00)
			// Picture data 	<binary data>
			str = new char [dwSize];
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
			if (str[0] == 0)
			{
				Unsynch((BYTE*)str, dwSize);
				int len			 = lstrlen(str+1);
				int len_desc	 = lstrlen(str+1+(len+1)+1);

				cover_unsynch = bUnsynch ? true : false;
				cover_pos = pos + (1/*enc*/ + (len+1) + 1/*type*/ + (len_desc+1));
				cover_size = dwSize - (1/*enc*/ + (len+1) + 1/*type*/ + (len_desc+1));
#if 0
				fseek(fp, cover_pos, SEEK_SET);
				fread(str, sizeof(char), cover_size, fp);
				FILE *f = fopen ("c:\\test.jpg", "wb");
				fwrite(str, 1, cover_size, f);
				fflush(f);
				fclose(f);
#endif
			}
			delete [] str;
			break;

		case PIC:
			// Text encoding      $xx
			// Image format       $xx xx xx
			// Picture type       $xx
			// Description        <textstring> $00 (00)
			// Picture data       <binary data>
			str = new char [dwSize];
			fseek(fp, pos, SEEK_SET);
			fread (str, sizeof(char), dwSize, fp);
			if (str[0] == 0)
			{
				int len = lstrlen(str + 5);
				cover_pos = pos + (5 + (len+1));
				cover_size = dwSize - (5 + (len+1));
#if 0
				fseek(fp, cover_pos, SEEK_SET);
				fread(str, sizeof(char), cover_size, fp);
				FILE *f = fopen ("c:\\test.jpg", "wb");
				fwrite(str, 1, cover_size, f);
				fflush(f);
				fclose(f);
#endif
			}
			delete [] str;
			break;
		}

		pos += dwSize;
	}

	return true;
}

// get title info
void CId3Tag::getTitle(char* input)
{
	wchar_t str[128];
	getTitle(str);
	lstrcpyn (input, AutoChar(str, wndWinampAL.GetEncodingCodepage()), 128);
}

// get artist info
void CId3Tag::getArtist(char* input)
{
	wchar_t str[128];
	getArtist(str);
	lstrcpyn (input, AutoChar(str, wndWinampAL.GetEncodingCodepage()), 128);
}

void CId3Tag::getAlbumArtist(char* input)
{
	wchar_t str[128];
	getAlbumArtist(str);
	lstrcpyn (input, AutoChar(str, wndWinampAL.GetEncodingCodepage()), 128);
}

// get album info
void CId3Tag::getAlbum(char* input)
{
	wchar_t str[128];
	getAlbum(str);
	lstrcpyn (input, AutoChar(str, wndWinampAL.GetEncodingCodepage()), 128);
}

// get comment info
void CId3Tag::getComment(char* input)
{
	wchar_t str[128];
	getComment(str);
	lstrcpyn (input, AutoChar(str, wndWinampAL.GetEncodingCodepage()), 128);
}

// get comment info
void CId3Tag::getGenre(char* input)
{
	wchar_t str[128];
	getGenre(str);
	lstrcpyn (input, AutoChar(str, wndWinampAL.GetEncodingCodepage()), 128);
}


// get title info
void CId3Tag::getTitle(wchar_t* input)
{
    wcsncpy (input, title, 128);
}

// get artist info
void CId3Tag::getArtist(wchar_t* input)
{
    wcsncpy (input, artist, 128);
}

// get album artist info
void CId3Tag::getAlbumArtist(wchar_t* input)
{
    wcsncpy (input, albumartist, 128);
}

// get album info
void CId3Tag::getAlbum(wchar_t* input)
{
    wcsncpy (input, album, 128);
}

// get comment info
void CId3Tag::getComment(wchar_t* input)
{
    wcsncpy (input, comment, 128);
}

void CId3Tag::getGenre(wchar_t* input)
{
    // this table of constant strings will be used in all cases..
    const char* table[148] = {
                              "Blues","Classic Rock","Country","Dance","Disco","Funk","Grunge","Hip-Hop","Jazz",
                              "Metal","New Age","Oldies","Other","Pop","R&B","Rap","Reggae","Rock","Techno",
                              "Industrial","Alternative","Ska","Death Metal","Pranks","Soundtrack","Euro-Techno",
                              "Ambient","Trip-Hop","Vocal","Jazz+Funk","Fusion","Trance","Classical","Instrumental",
                              "Acid","House","Game","Sound Clip","Gospel","Noise","Alt. Rock","Bass","Soul","Punk",
                              "Space","Meditative","Instrumental Pop","Instrumental Rock","Ethnic","Gothic",
                              "Darkwave","Techno-Industrial","Electronic","Pop-Folk","Eurodance","Dream",
                              "Southern Rock","Comedy","Cult","Gangsta","Top 40","Christian Rap","Pop/Funk","Jungle",
                              "Native American","Cabaret","New Wave","Psychadelic","Rave","Showtunes","Trailer",
                              "Lo-Fi","Tribal","Acid Punk","Acid Jazz","Polka","Retro","Musical","Rock & Roll",
                              "Hard Rock","Folk","Folk-Rock","National Folk","Swing","Fast Fusion","Bebob","Latin",
                              "Revival","Celtic","Bluegrass","Avantgarde","Gothic Rock","Progressive Rock",
                              "Psychedelic Rock","Symphonic Rock","Slow Rock","Big Band","Chorus","Easy Listening",
                              "Acoustic","Humour","Speech","Chanson","Opera","Chamber Music","Sonata","Symphony",
                              "Booty Bass","Primus","Porn Groove","Satire","Slow Jam","Club","Tango","Samba",
                              "Folklore","Ballad","Power Ballad","Rhythmic Soul","Freestyle","Duet","Punk Rock",
                              "Drum Solo","Acapella","Euro-House","Dance Hall","Goa","Drum & Bass","Club-House",
							  "Hardcore","Terror","Indie","BritPop","Negerpunk","Polsk Punk","Beat",
							  "Christian Gangsta Rap","Heavy Metal","Black Metal","Crossover","Contemporary Christian",
							  "Christian Rock","Merengue","Salsa","Thrash Metal","Anime","Jpop","Synthpop" 
                             };

	// if get operation then check whether it's supported
	// and if it is set it according table above
	if (wcslen(genrestr))	wcscpy(input, genrestr);
	else if (genre>=148)	wcscpy(input, L"");
	else					wcscpy(input, AutoWide(table[genre]));
}

// set the genre index info
void CId3Tag::setGenreIndex(int input) {

    genre = (UCHAR)(input%256);

}
// set the track number info
// to remove a track number execute this
// function
void CId3Tag::setTrackNumber(int input) {

    // no number avaliable
    if (input==-1) { 
        
        version = (float)1.0;
        trackNumber = -1;

    } else if (input<0 || input>255) {

        // an invalid tracknumber was specified
        MessageBox(0,"The tracknumber, should be between 0 and 255", "MP3Info Example", MB_OK);

    } else { // no errors, set the tracknumber

        version = (float)1.1;
        trackNumber = input;

    }

}
// set year info
void CId3Tag::setYear(int input) {

    // check whether input is out of range
    if (input<1000 || input>3000) {
         
        char mybuf[200];
        sprintf(mybuf, "The way the year is stored, it's best\nto use a year between 1000 and 3000,\nPersonally I don't think other years is needed\n\n%d seams like a stupid number",input);

        MessageBox(0, mybuf, "MP3Info Example", MB_OK|MB_ICONEXCLAMATION);

    } else { // everything alright, just set it

        year = input;

    }

}

bool CId3Tag::getCoverImage(const char *src, LPBYTE *input, DWORD &dwSize)
{
	return getCoverImage(AutoWide(src), input, dwSize);
}

bool CId3Tag::getCoverImage(const wchar_t *src, LPBYTE *input, DWORD &dwSize)
{
	if (cover_size == 0) return false;
	if (input == NULL) return false;

	FILE *fp = _wfopen(src, L"rb");

	if (fp)
	{
		*input = new BYTE [cover_size];

		if (fseek(fp, cover_pos, SEEK_SET)==0)
		{
			if (fread((char *)*input, sizeof(char), cover_size, fp))
			{
				dwSize = cover_size;

				if (cover_unsynch)
				{
					dwSize = Unsynch(*input, cover_size);
				}
			}
		}
		fclose(fp);
		return true;
	}

	return false;
}

DWORD CId3Tag::Unsynch(BYTE *data, DWORD dwSize)
{
	DWORD j=0;
	for (DWORD i=0; i<dwSize; i++)
	{
		data[j] = data[i];
		if ((data[i] == 0xff) && (data[i+1] == 0x0))
		{
			i++;
		}
		j++;
	}
	return j;
}

void CId3Tag::ReadStr(int codepage, char *src, DWORD srcSize, wchar_t *dst, DWORD dstSize)
{
	if (src == NULL) return;
	if (dst == NULL) return;

	memset(dst, 0, sizeof(wchar_t)*dstSize);

	// get text encoding method
	int encoding = *src++;
	srcSize--;

	switch (encoding)
	{
	case 0:		// ANSI
		MultiByteToWideChar(codepage, 0, src, min(srcSize, dstSize), dst, dstSize);
		break;

	case 1:		// UTF-16 with BOM
		if (*((LPWORD)src) == 0xfeff)		// UTF-16LE
		{
			wcsncpy(dst, (wchar_t*)(src+2), dstSize);
		}
		else if (*((LPWORD)src) == 0xfffe)	// UTF-16BE
		{
			for (unsigned int i=2; i<srcSize; i+=2)
			{
				char tmp = src[i];
				src[i] = src[i+1];
				src[i+1] = tmp;
			}
			wcsncpy(dst, (wchar_t*)(src+2), dstSize);
		}
		break;

	case 2:		// UTF-16BE without BOM
		wcsncpy(dst, (wchar_t*)src, dstSize);
		break;

	case 3:		// UTF-8
		MultiByteToWideChar(CP_UTF8, 0, src, srcSize, dst, dstSize);
		break;
	}
}
