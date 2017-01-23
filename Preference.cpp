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
#include "Preference.h"

//////////////////////////////////////////////////////////////////
// CPreference class

CPreference::CPreference()
{
	m_nIDTemplate = IDD_PREFERENCE;

	m_nLastConfig = 0;
}

CPreference::~CPreference()
{
}

void CPreference::AddPage(LPCTSTR string, CDialog *pDialog)
{
	m_strConfigs.Add(string);
	m_wndConfigs.Add(pDialog);
}

BOOL CPreference::OnInitDialog()
{
	CTabCtrl wndTab = GetDlgItem(IDC_TAB);

	TCITEM tc;
	memset(&tc, 0, sizeof(TCITEM));
	tc.mask = TCIF_TEXT|TCIF_PARAM;

	int size = m_wndConfigs.GetSize();
	for (int i=0; i<size; ++i)
	{
		CWnd *pWnd = (CWnd *)m_wndConfigs[i];

		tc.pszText = (LPTSTR)m_strConfigs[i];
		tc.lParam  = (LPARAM)m_wndConfigs[i];

		wndTab.InsertItem(i, &tc);
	}

	wndTab.SetCurSel(m_nLastConfig);
	OnNotifyTCNSelChanged();

	CenterWindow();

	return TRUE;
}

BOOL CPreference::OnDestroy()
{
	CTabCtrl wndTab = GetDlgItem(IDC_TAB);

	m_nLastConfig = wndTab.GetCurSel();

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

BOOL CPreference::OnNotify(UINT nID, NMHDR *pnmh, LRESULT *pResult)
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

BOOL CPreference::OnNotifyTCNSelChanging()
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

BOOL CPreference::OnNotifyTCNSelChanged()
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

