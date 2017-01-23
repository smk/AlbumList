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
#include <mbstring.h>
#include <commctrl.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "Util.h"
#include "JumpToAlbum.h"
#include "AlbumList.h"

//////////////////////////////////////////////////////////////////
// CSearchEdit class

CSearchEdit::CSearchEdit()
{
}

CSearchEdit::~CSearchEdit()
{
}

LRESULT CSearchEdit::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
		if ((wParam == VK_DOWN) || (wParam == VK_UP))
		{
			CWnd wndList = GetParent().GetDlgItem(IDC_RESULTLIST);
			wndList.SendMessage(message, wParam, lParam);
			return TRUE;
		}
	}

	return CWnd::DefWindowProc(message, wParam, lParam);
}

//////////////////////////////////////////////////////////////////
// CJumpToAlbum class

CJumpToAlbum::CJumpToAlbum()
{
	m_pAlbumList = NULL;
	m_nIDTemplate = IDD_JUMPTOALBUM;
}

CJumpToAlbum::~CJumpToAlbum()
{
}

BOOL CJumpToAlbum::OnInitDialog()
{
	m_wndEdit.SubclassWindow(GetDlgItem(IDC_SEARCHTEXT));

	OnChangeSearchText();

	SetForegroundWindow(m_hWnd);

	SetFocus(m_wndEdit);

	return FALSE;
}

BOOL CJumpToAlbum::OnDestroy()
{
	return TRUE;
}

BOOL CJumpToAlbum::OnCommand(WORD wNotifyCode, WORD wID, CWnd wndCtl)
{
	if (wNotifyCode == EN_CHANGE)
	{
		if (wID == IDC_SEARCHTEXT)
		{
			OnChangeSearchText();
			return TRUE;
		}
	}

	else if (wNotifyCode == LBN_DBLCLK)
	{
		if (wID == IDC_RESULTLIST)
		{
			OnOK();
			return TRUE;
		}
	}

	else if (wNotifyCode == BN_CLICKED)
	{
		if (wID == IDC_PLAY)
		{
			OnPlay();
			return TRUE;
		}
		else if (wID == IDC_ENQUEUE)
		{
			OnEnqueue();
			return TRUE;
		}
	}

	return CDialog::OnCommand(wNotifyCode, wID, wndCtl);
}

void CJumpToAlbum::OnChangeSearchText()
{
	wchar_t szSearchText[MAX_PATH];
	m_wndEdit.GetWindowText(szSearchText, MAX_PATH);
	wcslwr(szSearchText);

	CListBox wndList = GetDlgItem(IDC_RESULTLIST);

	// remove all the contents
	wndList.ResetContent();

	wchar_t name[MAX_PATH] = L"";

	int len = wcslen(szSearchText);

	int codepage = wndWinampAL.GetEncodingCodepage();

	// no search string entered, list everything
	if (len == 0)
	{
		int nSel	= 0;
		GUID id		= m_pAlbumList->GetCurAlbum();
		int size	= m_pAlbumList->GetSize();
		for (int i=0; i<size; ++i)
		{
			CAlbum *pAlbum = (CAlbum *)m_pAlbumList->GetAlbum(i);
			if (pAlbum)
			{
				swprintf(name, L"%ld. %s", i+1, pAlbum->GetTitle());
				int nIndex = wndList.AddString(name, codepage);
				if ((nIndex != LB_ERR) && (nIndex != LB_ERRSPACE))
				{
					wndList.SetItemData(nIndex, (DWORD)pAlbum);

					if (memcmp(&pAlbum->id, &id, sizeof(GUID)) == 0)
						nSel = nIndex;
				}
				pAlbum->Release();
			}
		}

		wndList.SetCurSel(nSel);

		wchar_t text[MAX_PATH];
		AutoWide str(ALS("Search for text"), wndWinampAL.GetCodepage());
		swprintf(text, L"%s (%ld/%ld)", (LPCWSTR)str, size, size);
		GetDlgItem(IDC_SEARCH_STATIC).SetWindowText(text);
	}
	else
	{
		int nSel = 0;
		int nFound = 0;
		
		GUID id = m_pAlbumList->GetCurAlbum();
		int size = m_pAlbumList->GetSize();
		for (int i=0; i<size; i++)
		{
			CAlbum *pAlbum = (CAlbum *)m_pAlbumList->GetAlbum(i);
			if (pAlbum)
			{
				// do year search as well when the search string
				// is at least 4 characters
				if (len >= 4)	swprintf(name, L"%ld %s %ld", i+1, pAlbum->GetTitle(), pAlbum->GetYear());
				else			swprintf(name, L"%ld %s", i+1, pAlbum->GetTitle());

				wcslwr(name);

				BOOL bFound = TRUE;

				CTokensW token(szSearchText, L" ?,\"'~!.-:;\t\n");
				for (int t=0; t<token.GetSize(); ++t)
				{
					// While there are tokens in "szSearchText"
					if (wcsstr(name, token[t]) == NULL)
					{
						bFound = FALSE;
						break;
					}
				}

				if (bFound)
				{
					swprintf(name, L"%ld. %s", i+1, pAlbum->GetTitle());

					int nIndex = wndList.AddString(name, codepage);
					if ((nIndex != LB_ERR) && (nIndex != LB_ERRSPACE))
					{
						wndList.SetItemData(nIndex, (DWORD)pAlbum);

						if (memcmp(&pAlbum->id, &id, sizeof(GUID)) == 0)
							nSel = nIndex;

						nFound++;
					}
				}
				pAlbum->Release();
			}
		}

		wndList.SetCurSel(nSel);

		wchar_t text[MAX_PATH];
		AutoWide str(ALS("Search for text"), wndWinampAL.GetCodepage());
		swprintf(text, L"%s (%ld/%ld)", (LPCWSTR)str, nFound, size);
		GetDlgItem(IDC_SEARCH_STATIC).SetWindowText(text);
	}
}

void CJumpToAlbum::OnOK()
{
	CListBox wndList = GetDlgItem(IDC_RESULTLIST);

	int nCurSel = wndList.GetCurSel();
	if (nCurSel == LB_ERR) return;

	DWORD dwData = wndList.GetItemData(nCurSel);
	if (dwData == LB_ERR) return;

	CAlbum *pAlbum = (CAlbum *)dwData;

	if (GetKeyState(VK_SHIFT) & 0x8000)
	{
		pAlbum->Enqueue();
	}
	else
	{
		if (wndWinampAL.GetSetting(settingEnqueueDefault))
			pAlbum->Enqueue();
		else
			pAlbum->Play();
	}

	CDialog::OnOK();
}

void CJumpToAlbum::OnPlay()
{
	CListBox wndList = GetDlgItem(IDC_RESULTLIST);

	int nCurSel = wndList.GetCurSel();
	if (nCurSel == LB_ERR) return;

	DWORD dwData = wndList.GetItemData(nCurSel);
	if (dwData == LB_ERR) return;

	CAlbum *pAlbum = (CAlbum *)dwData;

	pAlbum->Play();

	CDialog::OnOK();
}

void CJumpToAlbum::OnEnqueue()
{
	CListBox wndList = GetDlgItem(IDC_RESULTLIST);

	int nCurSel = wndList.GetCurSel();
	if (nCurSel == LB_ERR) return;

	DWORD dwData = wndList.GetItemData(nCurSel);
	if (dwData == LB_ERR) return;

	CAlbum *pAlbum = (CAlbum *)dwData;

	pAlbum->Enqueue();

	CDialog::OnOK();
}
