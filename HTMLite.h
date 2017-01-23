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
#ifndef __HTMLITE_H__
#define __HTMLITE_H__

#include "Util.h"

//////////////////////////////////////////////////////////////////
// CAnchor class

class CAnchor
{
public:
	CAnchor();
	CAnchor(RECT rect, LPCTSTR str);
	virtual ~CAnchor();

	RECT rc;
	char url[MAX_PATH];
};

//////////////////////////////////////////////////////////////////
// CHTMLite class

class CHTMLite : public CWnd
{
public:
	CHTMLite();
	virtual ~CHTMLite();

	BOOL Create(int x, int y, int cx, int cy, CWnd wndParent);

protected:
	virtual int  OnCreate		(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnDestroy		();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnPaint		(HDC hdc);
	virtual BOOL OnMouseMove	(UINT nFlags, POINT point);
	virtual BOOL OnLButtonDown	(UINT nFlags, POINT point);

	int FormatText				(HDC hdc, LPCTSTR lpszText, RECT *pRect, int nInitialXOffset);
	COLORREF GetColorFromString	(LPCTSTR lpszColor);

protected:
	CPtrArray	m_AnchorList;
};

#endif /* __HTMLITE_H__ */
