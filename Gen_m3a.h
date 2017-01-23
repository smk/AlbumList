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
#ifndef __GEN_M3A_H__
#define __GEN_M3A_H__

// winamp header files
#include "gen.h"
#include "ipc_pe.h"
#include "wa_dlg.h"
#include "wa_hotkeys.h"
#include "wa_ipc.h"
#include "wa_msgids.h"
#include "ml.h"
#include "ml_ipc.h"

// main project header files
#include "Util.h"
#include "WinampMain.h"
#include "WinampPE.h"
#include "WinampAL.h"

extern HINSTANCE hDllInstance;
extern winampGeneralPurposePlugin pluginGP;
extern winampMediaLibraryPlugin pluginML;
extern prefsDlgRec prefsrec;
extern genHotkeysAddStruct genHotKeys[];
extern char szAppName[];
extern CWinamp wndWinamp;
extern CWinampPE wndWinampPE;
extern CWinampAL wndWinampAL;
extern BOOL bWin9x;
extern BOOL bWinNT;
extern int IPC_AL_ADDMENU;

extern char dirPlugin[];
extern char iniPlugin[];

#endif /* __GEN_M3A_H__ */
