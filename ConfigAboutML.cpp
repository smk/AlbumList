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
#include <time.h>
#include <commctrl.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "ConfigAboutML.h"


//////////////////////////////////////////////////////////////////
// CConfigAboutML class

CConfigAboutML::CConfigAboutML()
{
	m_nIDTemplate = IDD_CONFIGABOUT_ML;
}

CConfigAboutML::~CConfigAboutML()
{
}

BOOL CConfigAboutML::OnInitDialog()
{
	WADlg_init(wndWinamp);

	COLORREF textColor = WADlg_getColor(WADLG_WNDFG);
	char szColor[8];
	wsprintf(szColor, "#%02x%02x%02x", GetRValue(textColor), GetGValue(textColor), GetBValue(textColor));

	lstrcpy(m_clrAL, szColor);
	lstrcpy(m_clrNorm, szColor);
	lstrcpy(m_clrLink, szColor);

	return CConfigAbout::OnInitDialog();
}

LRESULT CConfigAboutML::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_ERASEBKGND)
	{
		HDC hDC = (HDC) wParam;

		RECT rc;
		GetClientRect(&rc);

		// fill with background color
		HBRUSH hbr = CreateSolidBrush(WADlg_getColor(WADLG_WNDBG));
		FillRect(hDC, &rc, hbr);
		DeleteObject(hbr);

		return 1;
	}

	return CConfigAbout::DefWindowProc(message, wParam, lParam);
}
