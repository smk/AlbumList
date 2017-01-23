//	Album List for Winamp
//	Copyright (C) 1999-2006 Safai Ma
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "ammo.h"				// Required for ATI Remote Wonder plug-in SDK API
#include "..\alfront.h"			// Required for AL message definitions
#include "resource.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

// List of mapped functions for the programmable keys
#define kViewCover					(0)
#define kViewDetails				(1)
#define kPlayPrevAlbum				(2)
#define kPlayNextAlbum				(3)
#define kPlayPrevAlbumDiffArtist	(4)
#define kPlayNextAlbumDiffArtist	(5)
#define kPlayRandomAlbum			(6)
#define kPlayAllAlbums				(7)
#define kPlayFirstAlbum				(8)
#define kPlayLastAlbum				(9)
#define kOpensJumpToAlbumDialog		(10)
#define kOpensProperties			(11)

// The number of mapped programmable functions provided by the plug-in
#define kNumFunctions				(12)

// List of strings for the programmable keys (A-F). Each index in this list
// is mapped to a function in the defintions above.
static char *functions[kNumFunctions] = 
{	
	// To create groups in the tree control use the "|" symbol to seperate
	// the branch from the root. For Winamp, we have three types of groups.
	// The general options are put into the root while settings that are
	// toggled and equalizer settings each have a seperate branch.

	// Group the view settings into a seperate branch.
	"View|Cover",
	"View|Details",

	// All of the general options are at the root level.
	"Plays previous album",
	"Plays next album",
	"Plays previous album (different artist)",
	"Plays next album (different artist)",
	"Plays random album",
	"Plays all albums",
	"Plays first album",
	"Plays last album",
	"Opens jump to album dialog",
	"Opens properties dialog"
};

// Function prototype to a function that handles programmed key events.
static BOOL HandleProgrammedKey (HWND hWinamp, WORD wKeyEvent, WORD wState);

/*
 * Function:	WhatKeysDoYouWant
 *
 * Purpose:		Determines what groups of keys should be re-directed to
 *				the plug-in.
 *
 * Inputs:		None
 *
 * Returns:		A bit mask which indicates the groups of keys that should
 *				be re-directed to this plug-in.
 *
 * When called: This function is called once when the plug-in is first loaded,
 *				and again after calling Configure().
 *
 * Author:		ATI
 *
 * Copyright:	Copyright (c) 2002 ATI Technologies Inc
 */
DWORD WhatKeysDoYouWant (void)
{
	// For winamp we need to control the following keys:
	//
	// CUSTOM_MAPPED - provide list of programmable options
	// VOLUME_GROUP - adjust the playback volume
	// PLAY_GROUP - play, pause, stop, FF, RW
	// SETUP - toggle the preferences dialog
	// MENU - toggle the playlist editor
	// RESIZE - toggle doublesize mode
	// STOPWATCH - add current track as a bookmark

	return (CUSTOM_MAPPED);
}


/*
 * Function:	EnumerateProgrammableFunction
 *
 * Purpose:		Enumerate the descriptions of the programamble
 *				functions provided by this plug-in.
 *
 * Inputs:		wIndex - index of requested description
 *
 * Returns:		Pointer to the programmable function description
 *				or NULL if wIndex is greater than the number of
 *				programmable functions provided by this plug-in.
 *
 * When called:	This function is called after any call to WhatKeysDoYouWant()
 *				that returns a request for custom mapped keys (CUSTOM_MAPPED).
 *
 * Author:		ATI
 *
 * Copyright:	Copyright (c) 2002 ATI Technologies Inc
 */
char *EnumerateProgrammableFunction (WORD wIndex)
{	
	if (wIndex >= kNumFunctions) 
		return NULL;

	// Return a pointer to the requested description (see table above)
	return functions[wIndex];
}


/*
 * Function:	Configure
 *
 * Purpose:		Displays any configuration/setup page required by
 *				this plug-in. 
 *
 * Inputs:		hWnd - handle to the parent window. The parent window
 *				is used to ensure correct modal behaviour when creating 
 *				dialog and message boxes in this plug-in.
 *
 * Returns:		None
 *
 * When called: This function is called when the "Configure" button in the
 *				ATI Remote Wonder software is pressed.
 *
 * Author:		ATI
 *
 * Copyright:	Copyright (c) 2002 ATI Technologies Inc
 */
void Configure (HANDLE hWnd)
{
	// Display an 'About' message box
	MessageBox((HWND)hWnd, "Album List for Winamp\nATI Remote Wonder Support", "About", MB_OK);
}


/*
 * Function:	AreYouInFocus
 *
 * Purpose:		Determines if this plug-in is currently in-focus.
 *
 * Inputs:		None
 *
 * Returns:		TRUE if the plug-in is currently in focus, FALSE
 *				otherwise.
 *
 * When called:	This function is called when a key event occurs that
 *				the plug-in has requested.
 *
 * Author:		ATI
 *
 * Copyright:	Copyright (c) 2002 ATI Technologies Inc
 */
int AreYouInFocus (void)
{
	// The winamp main window uses a class name of 'Winamp v1.x'. Winamp
	// 2.X uses this name for compatibility with previous versions.
	
	HWND hWinamp = FindWindow("Winamp v1.x", NULL);
	HWND hForeground = GetForegroundWindow();

	if (hWinamp == NULL)
	{
		// Winamp is not running
		return FALSE;
	}

	if (hWinamp == hForeground)
	{
		// Winamp main window is in focus
		return TRUE;
	}
	
	if (hWinamp == GetWindow(hForeground, GW_OWNER))
	{
		// EQ, Minibrowser, Playlist or another Winamp window is in focus
		return TRUE;
	}

	// Winamp is running but is not in focus
	return FALSE;
}


/*
 * Function:	HandleKey
 *
 * Purpose:		Handles remote control keys.
 *
 * Inputs:		bCustom   - TRUE if wKeyEvent has been mapped to a custom
 *						    programamble option.
 *				wKeyEvent - Remote control key code for button that was
 *							pressed.
 *				wState    - RMCTRL_KEY_ON, when button is first pressed
 *						    RMCTRL_KEY_REPEAT, when button is held down
 *						    RMCTRL_KEY_OFF, when button is released
 *
 * Returns:		TRUE if the plug-in handled the command, FALSE otherwise.
 *
 * When called: This function is called whenever a key event occurs that
 *				the plug-in has requested when the plug-in is in focus.
 *
 * Author:		ATI
 *
 * Copyright:	Copyright (c) 2002 ATI Technologies Inc
 */
BOOL HandleKey (BOOL bCustom, WORD wKeyEvent, WORD wState)
{
	// Get the Winamp main window.
	HWND hWinamp = FindWindow("Winamp v1.x", NULL);
	HWND hWinampAL = FindWindow("Winamp AL",NULL);

	// If the album list window is not found, AL is not running.
	if (hWinampAL == NULL)
	{
		return FALSE;
	}
	
	// Handle programmed keys seperately
	if (bCustom)	
	{
		return HandleProgrammedKey(hWinampAL, wKeyEvent, wState);	
	}

	// The plug-in did not process this key. It is important to return FALSE
	// if the key was not processed so that other plug-ins or the ATI Remote Wonder
	// software can handle the key press.
	return FALSE;
}


/*
 * Function:	HandleProgrammedKey
 *
 * Purpose:		Handles remote control programmed keys
 *
 * Inputs:		hWinampAL - Handle to the AL window
 *				wKeyEvent - Remote control key code for button that was
 *							pressed.
 *				wState    - RMCTRL_KEY_ON, when button is first pressed
 *						    RMCTRL_KEY_REPEAT, when button is held down
 *						    RMCTRL_KEY_OFF, when button is released	
 *
 * Returns:		TRUE if the plug-in handled the command, FALSE otherwise.
 *
 * When called:	This function is called when a custom mapped programmable
 *				key has been pressed and the plug-in is in focus. This is
 *				a helper function for HandleKey().
 *
 * Author:		ATI
 *
 * Copyright:	Copyright (c) 2002 ATI Technologies Inc
 */
static BOOL HandleProgrammedKey (HWND hWinampAL, WORD wKeyEvent, WORD wState)
{
	int album_index = 0;
	if (wState != RMCTRL_KEY_ON)
		return FALSE;

	switch (wKeyEvent)
	{		
	case kViewCover:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_COVER_VIEW_CUR, 0);
		return TRUE;
	case kViewDetails:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_LIST_VIEW_CUR, 0);
		return TRUE;
	case kPlayPrevAlbum:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYPREVALBUM_CUR, 0);
		return TRUE;
	case kPlayNextAlbum:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYNEXTALBUM_CUR, 0);
		return TRUE;
	case kPlayPrevAlbumDiffArtist:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYPREVALBUMARTIST_CUR, 0);
		return TRUE;
	case kPlayNextAlbumDiffArtist:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYNEXTALBUMARTIST_CUR, 0);
		return TRUE;
	case kPlayRandomAlbum:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYRANDOMALBUM_CUR, 0);
		return TRUE;
	case kPlayAllAlbums:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYALLALBUMS_CUR, 0);
		return TRUE;
	case kPlayFirstAlbum:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYALBUM_CUR, 0);
		return TRUE;
	case kPlayLastAlbum:
		if (album_index = SendMessage(hWinampAL, WM_AL_IPC, IPC_GETALBUMSIZE_CUR, 0))
			PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYALBUM_CUR, album_index);
		return TRUE;
	case kOpensJumpToAlbumDialog:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_JUMPTOALBUM_CUR, 0);
		return TRUE;
	case kOpensProperties:
		PostMessage(hWinampAL, WM_AL_IPC, IPC_SHOWPREFERENCE_CUR, 0);
		return TRUE;
	}

	// The programmed key was not processed
	return FALSE;
}
