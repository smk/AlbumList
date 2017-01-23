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
#include <commctrl.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "ConfigNaming.h"

//////////////////////////////////////////////////////////////////
// CConfigNaming class

CConfigNaming::CConfigNaming()
{
	m_nIDTemplate = IDD_CONFIGNAMING;

	m_nDirStyle		= 0;
    m_bFixTitles	= TRUE;
	m_bMultiDiscFix	= FALSE;
	m_bUseID3		= TRUE;
	m_bIgnoreTHE	= TRUE;
	memset(m_szMultiDiscNames, 0, sizeof(m_szMultiDiscNames));

	m_bNeedRefresh	= FALSE;
}

CConfigNaming::~CConfigNaming()
{
}

BOOL CConfigNaming::OnInitDialog()
{
	// add all the styles
	CComboBox wndStyle = GetDlgItem(IDC_STYLE);
	wndStyle.AddString(ALS("-- Auto detect --"));
	wndStyle.AddString(ALS("...\\Artist\\Album"));
	wndStyle.AddString(ALS("...\\Artist - Album"));
	wndStyle.AddString(ALS("...\\Artist - Album - Year"));
	wndStyle.AddString(ALS("...\\Artist\\Year - Album"));
	wndStyle.AddString(ALS("...\\Artist - Year - Album"));
	RecalcDropWidth();

	// select the current style
	wndStyle.SetCurSel(m_nDirStyle);
    
	CheckDlgButton(IDC_FIXTITLES,	 m_bFixTitles ?		BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_MULTIDISCFIX, m_bMultiDiscFix ?	BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_USEID3,		 m_bUseID3 ?		BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_IGNORETHE,	 m_bIgnoreTHE ?		BST_CHECKED : BST_UNCHECKED);

	int bi = wndWinampAL.GetSetting(settingBuiltInFileInfo);
	CheckDlgButton(IDC_USEBUILTIN, bi ? BST_CHECKED : BST_UNCHECKED);

	int wae = wndWinampAL.GetSetting(settingWinampFileInfo);
	CheckDlgButton(IDC_USEWAEINFO, wae ? BST_CHECKED : BST_UNCHECKED);

	GetDlgItem(IDC_MULTIDISCNAMES).SetWindowText(m_szMultiDiscNames);

	return TRUE;
}

BOOL CConfigNaming::OnDestroy()
{
	int useid3		= IsDlgButtonChecked(IDC_USEID3);
	int dirstyle	= SendDlgItemMessage(IDC_STYLE, CB_GETCURSEL, 0, 0);
	m_bFixTitles	= IsDlgButtonChecked(IDC_FIXTITLES);
	m_bMultiDiscFix	= IsDlgButtonChecked(IDC_MULTIDISCFIX);
	m_bIgnoreTHE	= IsDlgButtonChecked(IDC_IGNORETHE);

	wndWinampAL.SetSetting(settingBuiltInFileInfo, IsDlgButtonChecked(IDC_USEBUILTIN));
	wndWinampAL.SetSetting(settingWinampFileInfo, IsDlgButtonChecked(IDC_USEWAEINFO));

	GetDlgItem(IDC_MULTIDISCNAMES).GetWindowText(m_szMultiDiscNames, MAX_PATH);

	if ((useid3 != m_bUseID3) || (dirstyle != m_nDirStyle))
	{
		m_bUseID3 = useid3;
		m_nDirStyle = dirstyle;

		m_bNeedRefresh = TRUE;
	}

	return TRUE;
}

void CConfigNaming::RecalcDropWidth()
{
	CComboBox wndCombo = GetDlgItem(IDC_STYLE);

	HDC		hDC = GetDC(wndCombo);
	HFONT	hFont = (HFONT)SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));
	int		nWidest = 0;
	SIZE	size;

	int nCount = wndCombo.GetCount();
	if (nCount && nCount != LB_ERR)
	{
		for (int i=0; i<nCount; i++)
		{
			if (IsWindowUnicode(wndCombo))
			{
				WCHAR szBuffer[MAX_PATH] = L"";
				wndCombo.GetLBText(i, szBuffer);
				GetTextExtentPointW(hDC, szBuffer, lstrlenW(szBuffer), &size);
			}
			else
			{
				char szBuffer[MAX_PATH] = "";
				wndCombo.GetLBText(i, szBuffer);
				GetTextExtentPoint(hDC, szBuffer, lstrlen(szBuffer), &size);
			}
			nWidest = max(nWidest, size.cx);
		}
	}
	
	wndCombo.SendMessage(CB_SETDROPPEDWIDTH, nWidest + 16, 0);

	// cleanup
	SelectObject(hDC, hFont);
	ReleaseDC(wndCombo, hDC);
}
