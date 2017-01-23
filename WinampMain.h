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
#ifndef __WINAMP_MAIN_H__
#define __WINAMP_MAIN_H__

#include "Util.h"

//////////////////////////////////////////////////////////////////
// CWinamp class

class CWinamp : public CWnd
{
public:
	CWinamp();
	virtual ~CWinamp();

	BOOL IsWinamp551() { return (SendIPCMessage() >= 0x5051); }
	BOOL IsWinamp531() { return (SendIPCMessage() >= 0x5031); }
	BOOL IsWinamp530() { return (SendIPCMessage() >= 0x5030); }
	BOOL IsWinamp525() { return (SendIPCMessage() >= 0x5025); }
	BOOL IsWinamp522() { return (SendIPCMessage() >= 0x5022); }
	BOOL IsWinamp513() { return (SendIPCMessage() >= 0x5013); }
	BOOL IsWinamp511() { return (SendIPCMessage() >= 0x5011); }
	BOOL IsWinamp5()   { return (SendIPCMessage() >= 0x5000); }
	BOOL IsWinamp29()  { return (SendIPCMessage() >= 0x2090); }

	void KeepSongPlaying		();
	void OnSongChange			();
	BOOL IsQuitting				();

	LRESULT SendIPCMessage		(WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT PostIPCMessage		(WPARAM wParam = 0, LPARAM lParam = 0);

	LRESULT SendMLMessage		(WPARAM wParam = 0, LPARAM lParam = 0);

protected:
	virtual BOOL OnCommand		(WORD wNotifyCode, WORD wID, CWnd wndCtl);
	virtual BOOL OnSysCommand	(UINT uCmdType, short xPos, short yPos);
	virtual BOOL OnClose		();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	BOOL m_bKeepSongPlaying;
	BOOL m_bQuitting;
};

#endif /* __WINAMP_MAIN_H__ */
