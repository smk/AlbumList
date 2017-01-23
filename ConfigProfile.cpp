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
#include <stdlib.h>
#include "resource.h"
#include "AutoWide.h"
#include "AutoChar.h"

#include "Gen_m3a.h"
#include "ConfigProfile.h"
#include "UserInterface.h"

//////////////////////////////////////////////////////////////////
// CConfigProfile class

CConfigProfile::CConfigProfile()
{
	m_nIDTemplate = IDD_CONFIGPROFILE_v290;
	if (wndWinamp.IsWinamp5())
		m_nIDTemplate = IDD_CONFIGPROFILE_v500;
}

CConfigProfile::~CConfigProfile()
{
}

BOOL CConfigProfile::OnInitDialog()
{
	// mlibrary support
	long libhwndipc = (LONG)wndWinamp.SendIPCMessage(WPARAM("LibraryGetWnd"), IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (libhwndipc)
	{
		// now send the message and get the HWND 
		CWnd wndML = (HWND)wndWinamp.SendIPCMessage(-1, LPARAM(libhwndipc));
		if (!wndML.IsWindow())
		{
			GetDlgItem(IDC_INTEGRATEML).ShowWindow(SW_HIDE);
		}
	}

	int ml = wndWinampAL.GetSetting(settingEmbedMLSave);
	CheckDlgButton(IDC_INTEGRATEML, ml ? BST_CHECKED : BST_UNCHECKED);

	// MSN support
	int msn = wndWinampAL.GetSetting(settingMSN);
	CheckDlgButton(IDC_MSN, msn ? BST_CHECKED : BST_UNCHECKED);

	CListCtrl wndList = GetDlgItem(IDC_PROFILELIST);

	RECT rc;
	wndList.GetClientRect(&rc);

	ListView_SetExtendedListViewStyleEx(wndList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	LVCOLUMN lvc;
	memset(&lvc, 0, sizeof(LVCOLUMN));
	lvc.mask = LVCF_TEXT|LVCF_WIDTH;
	lvc.pszText = "Profile";
	lvc.cx = rc.right - rc.left;
	ListView_InsertColumn(wndList, 0, &lvc);

	int size = wndWinampAL.m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)wndWinampAL.m_UserInterfaces.GetAt(i);

		char str[MAX_PATH];

		int id = pUI->GetProfileID();
		LPCTSTR name = pUI->GetProfileName();

		if (lstrlen(name))
		{
			lstrcpy(str, name);
		}
		else
		{
			wsprintf(str, ALS("Profile %ld"), id);
		}

		LVITEM lvi;
		memset(&lvi, 0, sizeof(LVITEM));
		lvi.mask = LVIF_TEXT|LVIF_PARAM;
		lvi.iItem = ListView_GetItemCount(wndList);
		lvi.pszText = str;
		lvi.cchTextMax = lstrlen(str);
		lvi.lParam = (LPARAM)pUI;
		wndList.InsertItem(&lvi);
	}


	OnProfileChange();

	return FALSE;
}

BOOL CConfigProfile::OnNotify(UINT nID, NMHDR *pnmh, LRESULT *pResult)
{
	NMLVDISPINFO *plvdi = (NMLVDISPINFO*)pnmh;

	switch (pnmh->idFrom)
	{
	case IDC_PROFILELIST:
		switch(pnmh->code)
		{
		case LVN_ITEMCHANGED:
			OnProfileChange();
			break;

		case NM_DBLCLK:
			OnEdit();
			break;

		case LVN_ENDLABELEDIT:
			OnNameChange(plvdi);
			*pResult = TRUE;
			return TRUE;

		case LVN_ENDLABELEDITW:
			OnNameChangeUnicode((NMLVDISPINFOW*)plvdi);
			*pResult = TRUE;
			return TRUE;
		}
	}
		
	return FALSE;
}

BOOL CConfigProfile::OnCommand(WORD wNotifyCode, WORD wID, CWnd wndCtl)
{
	if (wNotifyCode == BN_CLICKED)
	{
		switch (wID)
		{
		case IDC_NEW:
			OnNew();
			return TRUE;

		case IDC_EDIT:
			OnEdit();
			return TRUE;

		case IDC_RENAME:
			OnRename();
			return TRUE;

		case IDC_DELETE:
			OnDelete();
			return TRUE;
		}
	}
	return FALSE;
}

void CConfigProfile::OnNameChange(NMLVDISPINFO *plvdi)
{
	if (plvdi->item.pszText)
	{
		CUserInterface *pUI = (CUserInterface *)plvdi->item.lParam;
		if (pUI)
		{
			pUI->SetProfileName(plvdi->item.pszText);

			// reset the name
			if (lstrlen(pUI->GetProfileName()) == 0)
			{
				wsprintf(plvdi->item.pszText, ALS("Profile %ld"), pUI->GetProfileID());
			}
		}
	}
}

void CConfigProfile::OnNameChangeUnicode(NMLVDISPINFOW *plvdi)
{
	if (plvdi->item.pszText)
	{
		CUserInterface *pUI = (CUserInterface *)plvdi->item.lParam;
		if (pUI)
		{
			pUI->SetProfileName(AutoChar(plvdi->item.pszText, wndWinampAL.GetCodepage()));

			// reset the name
			if (lstrlen(pUI->GetProfileName()) == 0)
			{
				wsprintfW(plvdi->item.pszText, ALSW("Profile %ld"), pUI->GetProfileID());
			}
		}
	}
}

void CConfigProfile::OnProfileChange()
{
	CWnd wndList = GetDlgItem(IDC_PROFILELIST);
	int nSel = ListView_GetNextItem(wndList, -1, LVIS_SELECTED);
	BOOL bEnable = (BOOL)(nSel != -1);

	GetDlgItem(IDC_EDIT).EnableWindow(bEnable);
	GetDlgItem(IDC_RENAME).EnableWindow(bEnable);
	GetDlgItem(IDC_DELETE).EnableWindow(bEnable);
}

void CConfigProfile::OnNew()
{
	CListCtrl wndList = GetDlgItem(IDC_PROFILELIST);

	// find the first available ID
	int nID = 0;
	while (wndWinampAL.IsProfileIDUsed(nID)) nID++;

	CUserInterface *pUI = wndWinampAL.AddUserInterface(nID);
	if (pUI)
	{
		// add to the list
		char str[MAX_PATH];
		wsprintf(str, ALS("Profile %ld"), nID);

		LVITEM lvi;
		memset(&lvi, 0, sizeof(LVITEM));
		lvi.mask = LVIF_TEXT|LVIF_PARAM;
		lvi.iItem = ListView_GetItemCount(wndList);
		lvi.pszText = str;
		lvi.cchTextMax = lstrlen(str);
		lvi.lParam = (LPARAM)pUI;
		wndList.InsertItem(&lvi);
	}
}

void CConfigProfile::OnEdit()
{
	CWnd wndList = GetDlgItem(IDC_PROFILELIST);
	int nSel = ListView_GetNextItem(wndList, -1, LVIS_SELECTED);
	if (nSel == -1) return;

	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nSel;
	if (!ListView_GetItem(wndList, &lvi)) return;

	CUserInterface *pUI = (CUserInterface *)lvi.lParam;
	if (pUI == NULL) return;

	pUI->ShowPreference(GetSafeHwnd());
}

void CConfigProfile::OnRename()
{
	CWnd wndList = GetDlgItem(IDC_PROFILELIST);
	int nSel = ListView_GetNextItem(wndList, -1, LVIS_SELECTED);
	if (nSel == -1) return;

	SetFocus(wndList);
	ListView_EditLabel(wndList, nSel);
}

void CConfigProfile::OnDelete()
{
	CWnd wndList = GetDlgItem(IDC_PROFILELIST);
	int nSel = ListView_GetNextItem(wndList, -1, LVIS_SELECTED);
	if (nSel == -1) return;

	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nSel;
	if (!ListView_GetItem(wndList, &lvi)) return;

	CUserInterface *pUI = (CUserInterface *)lvi.lParam;
	if (pUI == NULL) return;

	// destroy the window
	if (!wndWinampAL.GetSetting(settingEmbedML))
	{
		pUI->GetParent().DestroyWindow();
	}
	else
	{
		pUI->DestroyWindow();
	}

	// find it in the list
	int nID = pUI->GetProfileID();

	if (wndWinampAL.RemoveUserInterface(nID))
	{
		// remove it from the listbox
		ListView_DeleteItem(wndList, nSel);
	}
}

BOOL CConfigProfile::OnDestroy()
{
	wndWinampAL.SetSetting(settingEmbedML, IsDlgButtonChecked(IDC_INTEGRATEML));
	wndWinampAL.SetSetting(settingMSN, IsDlgButtonChecked(IDC_MSN));

	return TRUE;
}
