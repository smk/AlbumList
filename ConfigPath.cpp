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
#include "ConfigPath.h"
#include "AutoWide.h"

//////////////////////////////////////////////////////////////////
// CConfigPath class

CConfigPath::CConfigPath()
{
	m_nIDTemplate = IDD_CONFIGPATH;

	memset(m_szExtList, 0, sizeof(m_szExtList));
	m_bStartupScan	= TRUE;
	m_bScanSubDir	= TRUE;
	m_bScanCDROM	= FALSE;
	m_bIgnorePL		= FALSE;
	m_bIgnoreRootPL	= FALSE;
	m_bPLOnly		= FALSE;

	m_bNeedRefresh	= FALSE;
}

CConfigPath::~CConfigPath()
{
}

BOOL CConfigPath::OnInitDialog()
{
	CListBox wndList = GetDlgItem(IDC_PATHLIST);

	int size = m_SearchPaths.GetSize();
	for (int i=0; i<size; ++i)
	{
		wndList.AddString(m_SearchPaths[i]);
	}

	GetDlgItem(IDC_FILETYPEEDIT).SetWindowText(m_szExtList);

	CheckDlgButton(IDC_SCAN_AT_STARTUP,	m_bStartupScan ?	BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_SCANSUBDIR,		m_bScanSubDir ?		BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_ADD_CDROM,		m_bScanCDROM ?		BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_IGNORE_M3U,		m_bIgnorePL ?		BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_IGNORE_M3U_ROOT,	m_bIgnoreRootPL ?	BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_M3U_ONLY,		m_bPLOnly ?			BST_CHECKED : BST_UNCHECKED);

	GetDlgItem(IDC_IGNORE_M3U_ROOT).EnableWindow(!m_bIgnorePL || m_bPLOnly);
	GetDlgItem(IDC_M3U_ONLY).EnableWindow(!m_bIgnorePL);
	GetDlgItem(IDC_IGNORE_M3U).EnableWindow(!m_bPLOnly);

	AdjustScrollWidth();

	return TRUE;
}

BOOL CConfigPath::OnDestroy()
{
	// scan directories at startup
	m_bStartupScan = IsDlgButtonChecked(IDC_SCAN_AT_STARTUP);

	// scan sub-directories
	if (m_bScanSubDir != (int)IsDlgButtonChecked(IDC_SCANSUBDIR))
	{
		m_bScanSubDir = IsDlgButtonChecked(IDC_SCANSUBDIR);
		m_bNeedRefresh = TRUE;
	}

	// search CD-ROMs automatically?
	if (m_bScanCDROM != (int)IsDlgButtonChecked(IDC_ADD_CDROM))
	{
		m_bScanCDROM = IsDlgButtonChecked(IDC_ADD_CDROM);
		m_bNeedRefresh = TRUE;
	}

	// ignore playlist at root of search path
	if (m_bIgnorePL != (int)IsDlgButtonChecked(IDC_IGNORE_M3U))
	{
		m_bIgnorePL = IsDlgButtonChecked(IDC_IGNORE_M3U);
		m_bNeedRefresh = TRUE;
	}

	// ignore all playlist
	if (m_bIgnoreRootPL != (int)IsDlgButtonChecked(IDC_IGNORE_M3U_ROOT))
	{
		m_bIgnoreRootPL = IsDlgButtonChecked(IDC_IGNORE_M3U_ROOT);
		m_bNeedRefresh = TRUE;
	}

	// playlist only
	if (m_bPLOnly != (int)IsDlgButtonChecked(IDC_M3U_ONLY))
	{
		m_bPLOnly = IsDlgButtonChecked(IDC_M3U_ONLY);
		m_bNeedRefresh = TRUE;
	}

	// file type
	GetDlgItem(IDC_FILETYPEEDIT).GetWindowText(m_szExtList, MAX_PATH);

	// search paths
	m_SearchPaths.RemoveAll();
	CListBox wndList = GetDlgItem(IDC_PATHLIST);
	int nCount = wndList.GetCount();
	if (nCount && nCount != LB_ERR)
	{
		wchar_t szBuffer[MAX_PATH] = L"";
		for (int i=0; i<nCount; i++)
		{
			wndList.GetText(i, szBuffer);
			m_SearchPaths.Add(szBuffer);
		}
	}

	return TRUE;
}

void CConfigPath::OnAdd()
{
	CListBox wndList = GetDlgItem(IDC_PATHLIST);

	wchar_t szBuffer[MAX_PATH] = L"";

	AutoWide title(ALS("Select the folder where you want to include."), wndWinampAL.GetCodepage());

	if (BrowseForFolder(m_hWnd, title, szBuffer, MAX_PATH))
	{
		wndList.AddString(szBuffer);
		m_bNeedRefresh = TRUE;
	}

	AdjustScrollWidth();
}

void CConfigPath::OnRemove()
{
	CWnd wndList = GetDlgItem(IDC_PATHLIST);

	int nSel = wndList.SendMessage(LB_GETCURSEL, 0, 0);
	if (nSel == LB_ERR) return;

	wndList.SendMessage(LB_DELETESTRING, nSel, 0);
	m_bNeedRefresh = TRUE;

	int nCount = wndList.SendMessage(LB_GETCOUNT, 0, 0);
	if (nCount)
	{
		if (nCount <= nSel) nSel = nCount - 1;
		wndList.SendMessage(LB_SETCURSEL, nSel, 0);
	}
	
	AdjustScrollWidth();
}

void CConfigPath::OnRemoveAll()
{
	CWnd wndList = GetDlgItem(IDC_PATHLIST);

	int nSel = wndList.SendMessage(LB_RESETCONTENT, 0, 0);
	if (nSel == LB_ERR) return;

	m_bNeedRefresh = TRUE;

	AdjustScrollWidth();
}

void CConfigPath::OnIgnoreM3U()
{
	GetDlgItem(IDC_IGNORE_M3U_ROOT).EnableWindow(!IsDlgButtonChecked(IDC_IGNORE_M3U));
	GetDlgItem(IDC_M3U_ONLY).EnableWindow(!IsDlgButtonChecked(IDC_IGNORE_M3U));
}

void CConfigPath::OnM3UOnly()
{
	GetDlgItem(IDC_IGNORE_M3U_ROOT).EnableWindow(TRUE);
	GetDlgItem(IDC_IGNORE_M3U).EnableWindow(!IsDlgButtonChecked(IDC_M3U_ONLY));
}

BOOL CConfigPath::OnCommand(WORD wNotifyCode, WORD wID, CWnd wndCtl)
{
	switch (wNotifyCode)
	{
	case BN_CLICKED:
		switch (wID)
		{
		case IDC_ADD:
			OnAdd();
			return TRUE;

		case IDC_REMOVE:
			OnRemove();
			return TRUE;

		case IDC_REMOVEALL:
			OnRemoveAll();
			return TRUE;

		case IDC_IGNORE_M3U:
			OnIgnoreM3U();
			return TRUE;

		case IDC_M3U_ONLY:
			OnM3UOnly();
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void CConfigPath::AdjustScrollWidth()
{
	CWnd wndList = GetDlgItem(IDC_PATHLIST);

	HDC		hDC = GetDC(wndList);
	RECT	rc;
	HFONT	hFont = (HFONT)SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));
	int		nWidest = 0;

	rc.left		= 0;
	rc.top		= 0;
	rc.right	= 1;
	rc.bottom	= 1;

	int nCount = wndList.SendMessage(LB_GETCOUNT, 0, 0);
	if (nCount && nCount != LB_ERR)
	{
		char szBuffer[MAX_PATH] = "";
		for (int i=0; i<nCount; i++)
		{
			wndList.SendMessage(LB_GETTEXT, i, (LPARAM)szBuffer);
			DrawText(hDC, szBuffer, -1, &rc, DT_CALCRECT);
			
			if(rc.right > nWidest) {
				nWidest = rc.right;
			}
		}
	}
	
	wndList.SendMessage(LB_SETHORIZONTALEXTENT, nWidest + 8, 0);
	ReleaseDC(wndList, hDC);
}

