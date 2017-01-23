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
#ifndef __USERINTERFACE_LISTCTRL_H__
#define __USERINTERFACE_LISTCTRL_H__

#include "Util.h"

#define WM_HEADER_COLUMN_RCLICK		(WM_USER+2323)

//////////////////////////////////////////////////////////////////
// CUserInterfaceListCtrl class

class CUserInterfaceListCtrl : public CWnd
{
public:
	CUserInterfaceListCtrl();
	virtual ~CUserInterfaceListCtrl();

	CUserInterface *m_pUI;

	void HideAutoHideHeader		();
	BOOL EnsureVisible			(int nItem, BOOL bPartialOK);
	void SetCurSel				(UINT nSel);
	void UnSelectAll			();

protected:
	virtual BOOL OnNotify		(UINT nID, NMHDR *pnmh, LRESULT *pResult);
	virtual BOOL OnMouseMove	(UINT nFlags, POINT point);
	virtual BOOL OnNcMouseMove	(UINT nFlags, POINT point);
	virtual BOOL OnChar			(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	BOOL OnCustomDrawHeaderCtrl	(NMHDR *pnmh, LRESULT *pResult);
	BOOL OnCustomDrawInfoTip	(NMHDR *pnmh, LRESULT *pResult);

private:
	DWORD				m_dwLastInputCount;
	char				m_szInputBuffer[MAX_PATH];
};

//////////////////////////////////////////////////////////////////
// CUserInterfaceHeaderCtrl class

class CUserInterfaceHeaderCtrl : public CWnd
{
public:
	CUserInterfaceHeaderCtrl();
	virtual ~CUserInterfaceHeaderCtrl();

	CUserInterface *m_pUI;

protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT OnLayout			(HDLAYOUT *pLayout);
};

#endif /* __USERINTERFACE_LISTCTRL_H__ */
