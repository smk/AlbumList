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
#include "ConfigMain.h"
#include "ConfigProfile.h"
#include "ConfigOption.h"
#include "ConfigLanguage.h"
#include "ConfigHistory.h"
#include "ConfigAbout.h"

// one copy of the config page
CConfigMain preference;

//////////////////////////////////////////////////////////////////
// CConfigMain class

CConfigMain::CConfigMain()
{
}

CConfigMain::~CConfigMain()
{
	int size = m_wndConfigs.GetSize();
	for (int i=0; i<size; ++i)
	{
		CWnd *pWnd = (CWnd *)m_wndConfigs[i];
		if (pWnd) delete pWnd;
	}
}

BOOL CALLBACK ConfigMainProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		// hacky way to set our pointers with the proc
		preference.m_hWnd = hwndDlg;
		SetProp(hwndDlg, "CDialogAL", &preference);
	}

	return preference.DialogProc(hwndDlg, uMsg, wParam, lParam);
}

void CConfigMain::AddPages()
{
	m_strConfigs.Add(ALS("Profile"));	m_wndConfigs.Add(new CConfigProfile);
	m_strConfigs.Add(ALS("Option"));	m_wndConfigs.Add(new CConfigOption);
	m_strConfigs.Add(ALS("Language"));	m_wndConfigs.Add(new CConfigLanguage);
	m_strConfigs.Add(ALS("History"));	m_wndConfigs.Add(new CConfigHistory);
	m_strConfigs.Add(ALS("About"));		m_wndConfigs.Add(new CConfigAbout);

	prefsrec.hInst	= hDllInstance;
	prefsrec.dlgID	= IDD_CONFIG_v290;
	prefsrec.proc	= ConfigMainProc;
	prefsrec.name	= szAppName;
	prefsrec.where	= 0;
	prefsrec._id	= 0;
	prefsrec.next	= NULL;

	// winamp 5 dialogs?
	if (wndWinamp.IsWinamp5())
		prefsrec.dlgID = IDD_CONFIG_v500;

	wndWinamp.SendIPCMessage((int)&prefsrec, IPC_ADD_PREFS_DLG);
}

void CConfigMain::ShowPage()
{
	wndWinamp.SendIPCMessage(prefsrec._id, IPC_OPENPREFSTOPAGE);
	wndWinamp.PostIPCMessage(prefsrec._id, IPC_OPENPREFSTOPAGE);
}

BOOL CConfigMain::OnInitDialog()
{
	CTabCtrl wndTab = GetDlgItem(IDC_TAB);

	TCITEM tc;
	memset(&tc, 0, sizeof(TCITEM));
	tc.mask = TCIF_TEXT|TCIF_PARAM;

	int size = m_wndConfigs.GetSize();
	for (int i=0; i<size; ++i)
	{
		CWnd *pWnd = (CWnd *)m_wndConfigs[i];

		tc.pszText = (LPTSTR)ALS(m_strConfigs[i]);
		tc.lParam  = (LPARAM)m_wndConfigs[i];
		wndTab.InsertItem(i, &tc);
	}

	int last = wndWinampAL.GetSetting(settingLastConfig);

	wndTab.SetCurSel(last);
	OnNotifyTCNSelChanged();

	return TRUE;
}

BOOL CConfigMain::OnNotify(UINT nID, NMHDR *pnmh, LRESULT *pResult)
{
	switch(pnmh->code)
	{
	case TCN_SELCHANGING:
		switch (pnmh->idFrom)
		{
		case IDC_TAB:
			OnNotifyTCNSelChanging();
			break;
		}
		break;

	case TCN_SELCHANGE:
		switch (pnmh->idFrom)
		{
		case IDC_TAB:
			OnNotifyTCNSelChanged();
			break;
		}
		break;
	}

	return FALSE;
}

BOOL CConfigMain::OnDestroy()
{
	CTabCtrl wndTab = GetDlgItem(IDC_TAB);

	wndWinampAL.SetSetting(settingLastConfig, wndTab.GetCurSel());

	int count = wndTab.GetItemCount();

	TCITEM tc;
	memset(&tc, 0, sizeof(TCITEM));
	tc.mask = TCIF_PARAM;
	for (int i=0; i<count; i++)
	{
		if (wndTab.GetItem(i, &tc))
		{
			CWnd *pWnd = (CWnd *)tc.lParam;
			if (pWnd != NULL)
			{
				pWnd->DestroyWindow();
			}
		}
	}

	return FALSE;
}

BOOL CConfigMain::OnNotifyTCNSelChanging()
{
	CTabCtrl wndTab = GetDlgItem(IDC_TAB);

	int nSel = wndTab.GetCurSel();
	if (nSel == -1) return FALSE;

	TCITEM tc;
	memset(&tc, 0, sizeof(TCITEM));
	tc.mask = TCIF_PARAM;
	if (wndTab.GetItem(nSel, &tc))
	{
		CWnd *pWnd = (CWnd *)tc.lParam;
		if (pWnd != NULL)
		{
			pWnd->DestroyWindow();
		}
	}

	return FALSE;
}

BOOL CConfigMain::OnNotifyTCNSelChanged()
{
	CTabCtrl wndTab = GetDlgItem(IDC_TAB);

	int nSel = wndTab.GetCurSel();
	if (nSel == -1) return FALSE;

	TCITEM tc;
	memset(&tc, 0, sizeof(TCITEM));
	tc.mask = TCIF_PARAM;
	if (wndTab.GetItem(nSel, &tc))
	{
		CDialog *pDialog = (CDialog *)tc.lParam;
		if (pDialog != NULL)
		{
			RECT rc;
			wndTab.GetWindowRect(&rc);
			wndTab.AdjustRect(FALSE, &rc);
			ScreenToClient((LPPOINT)&rc.left);
			ScreenToClient((LPPOINT)&rc.right);

			pDialog->Create(this);
			pDialog->SetWindowPos(NULL, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_NOZORDER|SWP_NOACTIVATE);

			// fixes the dialog's background texture/color
			if (wndWinamp.IsWinamp5())
				WinXPEnableThemeDialogTexture(pDialog->GetSafeHwnd(), 6/*ETDT_ENABLETAB*/);
			
			pDialog->ShowWindow(SW_SHOW);
		}
	}

	return FALSE;
}

