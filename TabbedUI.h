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
#ifndef _TABBEDUI_H_
#define _TABBEDUI_H_

#include "TabbedUI.h"

//////////////////////////////////////////////////////////////////
// CCustomTab class

class CCustomTab : public CWnd
{
public:
	CCustomTab();
	virtual ~CCustomTab();

protected:
	virtual BOOL OnPaint		(HDC hdc);
	virtual BOOL OnEraseBkgnd	(HDC hdc);

private:
	void DrawItem(HDC hdc, int item, RECT rc, BOOL bRev);
};

//////////////////////////////////////////////////////////////////
// CTabbedUI class

class CTabbedUI : public CDialog
{
public:
	CTabbedUI();
	virtual ~CTabbedUI();

protected:
	virtual BOOL OnInitDialog	();
	virtual BOOL OnSize			(UINT nType, int cx, int cy);
	virtual BOOL OnNotify		(UINT nID, NMHDR *pnmh, LRESULT *pResult);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	BOOL OnNotifyTCNSelChanging();
	BOOL OnNotifyTCNSelChanged();
	BOOL OnCustomTab(NMHDR* pnmh, LRESULT* pResult);

private:
	CCustomTab m_wndTab;
};


#endif /* _TABBEDUI_H_ */

