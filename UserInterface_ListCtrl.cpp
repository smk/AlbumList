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

#include "Gen_m3a.h"
#include "UserInterface.h"
#include "UserInterface_ListCtrl.h"

//////////////////////////////////////////////////////////////////
// CUserInterfaceListCtrl class
CUserInterfaceListCtrl::CUserInterfaceListCtrl()
{
	m_pUI = NULL;
	m_dwLastInputCount = 0;
	memset(m_szInputBuffer, 0, sizeof(m_szInputBuffer));
}

CUserInterfaceListCtrl::~CUserInterfaceListCtrl()
{
}

void CUserInterfaceListCtrl::HideAutoHideHeader()
{
	if (m_pUI && m_pUI->m_bAutoHideHeader)
	{
		DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);

		if ((dwStyle & LVS_NOCOLUMNHEADER) == 0)
		{
			int nTop = ListView_GetTopIndex(m_hWnd);

			// highlight the first one so that
			// the listview won't scroll
			int nState = ListView_GetItemState(m_hWnd, nTop, LVIS_SELECTED|LVIS_FOCUSED);
			ListView_SetItemState(m_hWnd, nTop, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

			ModifyStyle(0, LVS_NOCOLUMNHEADER);

			// restore the state
			ListView_SetItemState(m_hWnd, nTop, nState, LVIS_SELECTED|LVIS_FOCUSED);
		}
	}
}

BOOL CUserInterfaceListCtrl::OnNotify(UINT nID, NMHDR *pnmh, LRESULT *pResult)
{
	int size;

	if (pnmh->hwndFrom == ListView_GetHeader(m_hWnd))
	{
		switch(pnmh->code)
		{
		case NM_CUSTOMDRAW:
			return OnCustomDrawHeaderCtrl(pnmh, pResult);

		case NM_RCLICK:
			if ((size = Header_GetItemCount(GetDlgItem(0))) > 0)
			{
				// get cursor
				POINT pt;
				GetCursorPos(&pt);
				GetDlgItem(0).ScreenToClient(&pt);

				int nColumn = -1;
				for (int i=0; i<size; ++i)
				{
					RECT rc;
					Header_GetItemRect(GetDlgItem(0), i, &rc);
					if (PtInRect(&rc, pt))
					{
						nColumn = i;
						break;
					}
				}

				HDITEM hdi;
				memset(&hdi, 0, sizeof(HDITEM));
				hdi.mask = HDI_LPARAM;
				GetDlgItem(0).SendMessage(HDM_GETITEM, nColumn, (LPARAM)&hdi);

				GetParent().SendMessage(WM_HEADER_COLUMN_RCLICK, hdi.lParam);
			}
			*pResult = 0;
			return TRUE;
		}
	}

#if 0
	else if (pnmh->idFrom == 0)
	{
		switch(pnmh->code)
		{
		case NM_CUSTOMDRAW:
			return OnCustomDrawInfoTip(pnmh, pResult);
		}
	}
#endif

	return FALSE;
}

BOOL CUserInterfaceListCtrl::OnCustomDrawHeaderCtrl(NMHDR *pnmh, LRESULT *pResult)
{
    NMCUSTOMDRAW* pCD = (NMCUSTOMDRAW*)pnmh;

	if (CDDS_PREPAINT == pCD->dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
		return TRUE;
	}
	else if (CDDS_ITEMPREPAINT == pCD->dwDrawStage)
	{
		HBRUSH hbr = CreateSolidBrush(WADlg_getColor(WADLG_LISTHEADER_BGCOLOR));
		FillRect(pCD->hdc, &pCD->rc, hbr);
		DeleteObject(hbr);

		HPEN pen1=CreatePen(PS_SOLID,0,WADlg_getColor(WADLG_LISTHEADER_FRAME_TOPCOLOR));
		HPEN pen2=CreatePen(PS_SOLID,0,WADlg_getColor(WADLG_LISTHEADER_FRAME_MIDDLECOLOR));
		HPEN pen3=CreatePen(PS_SOLID,0,0);
		HGDIOBJ o=SelectObject(pCD->hdc, pen1);

		if ((pCD->uItemState & CDIS_SELECTED) == CDIS_SELECTED)
		{
			SelectObject(pCD->hdc, pen3);
			MoveToEx(pCD->hdc,pCD->rc.left,pCD->rc.bottom,NULL);
			LineTo(pCD->hdc,pCD->rc.left,pCD->rc.top);
			LineTo(pCD->hdc,pCD->rc.right-1,pCD->rc.top);
			LineTo(pCD->hdc,pCD->rc.right-1,pCD->rc.bottom-1);
			LineTo(pCD->hdc,pCD->rc.left,pCD->rc.bottom-1);
		}
		else
		{
			// left, top
			MoveToEx(pCD->hdc,pCD->rc.left,pCD->rc.bottom,NULL);
			LineTo(pCD->hdc,pCD->rc.left,pCD->rc.top);
			LineTo(pCD->hdc,pCD->rc.right,pCD->rc.top);

			// right, bottom
			SelectObject(pCD->hdc, pen2);
			MoveToEx(pCD->hdc,pCD->rc.left+1,pCD->rc.bottom-2,NULL);
			LineTo(pCD->hdc,pCD->rc.right-2,pCD->rc.bottom-2);
			LineTo(pCD->hdc,pCD->rc.right-2,pCD->rc.top);

			// right, bottom (black line)
			SelectObject(pCD->hdc, pen3);
			MoveToEx(pCD->hdc,pCD->rc.left,pCD->rc.bottom-1,NULL);
			LineTo(pCD->hdc,pCD->rc.right-1,pCD->rc.bottom-1);
			LineTo(pCD->hdc,pCD->rc.right-1,pCD->rc.top-1);
		}

		SetTextColor(pCD->hdc, WADlg_getColor(WADLG_LISTHEADER_FONTCOLOR));
		SetBkMode(pCD->hdc, TRANSPARENT);

		char name[MAX_PATH] = "";
		HDITEM hdi;
		hdi.mask = HDI_TEXT|HDI_FORMAT;
		hdi.pszText = name;
		hdi.cchTextMax = MAX_PATH;
		Header_GetItem(pCD->hdr.hwndFrom, pCD->dwItemSpec, &hdi);

		RECT rcItem = pCD->rc;
		rcItem.left += 5;
		rcItem.right -= 5;
		rcItem.top ++;

		if ((pCD->uItemState & CDIS_SELECTED) == CDIS_SELECTED)
		{
			rcItem.left ++;
			rcItem.right ++;
		}

		int align = (hdi.fmt & HDF_JUSTIFYMASK) == LVCFMT_RIGHT ? DT_RIGHT : DT_LEFT;

		DrawTextW(pCD->hdc, AutoWide(hdi.pszText, wndWinampAL.GetCodepage()), -1, &rcItem, align | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

		// clean up
		SelectObject(pCD->hdc, o);
		DeleteObject(pen1);
		DeleteObject(pen2);
		DeleteObject(pen3);

		*pResult = CDRF_SKIPDEFAULT;
		return TRUE;
	}
	else if (CDDS_POSTPAINT == pCD->dwDrawStage)
	{
		int nCount = Header_GetItemCount(pCD->hdr.hwndFrom);
		int index = Header_OrderToIndex(pCD->hdr.hwndFrom, nCount - 1);

		RECT rc1;
		Header_GetItemRect(pCD->hdr.hwndFrom, index, &rc1);

		RECT rc;
		GetClientRect(&rc);
		if (rc1.right < rc.right)
		{
			rc.left = rc1.right;
			rc.right += 10;
			HBRUSH hbr = CreateSolidBrush(WADlg_getColor(WADLG_LISTHEADER_EMPTY_BGCOLOR));
			FillRect(pCD->hdc, &rc, hbr);
			DeleteObject(hbr);
		}

		*pResult = CDRF_DODEFAULT;
		return TRUE;
	}
	return FALSE;
}

BOOL CUserInterfaceListCtrl::OnCustomDrawInfoTip(NMHDR *pnmh, LRESULT *pResult)
{
    NMCUSTOMDRAW* pCD = (NMCUSTOMDRAW*)pnmh;

	if (CDDS_PREPAINT == pCD->dwDrawStage)
	{
		SetBkColor(pCD->hdc, WADlg_getColor(WADLG_ITEMBG));
		SetTextColor(pCD->hdc, WADlg_getColor(WADLG_ITEMFG));
		*pResult = CDRF_NOTIFYITEMDRAW;
		return TRUE;
	}
	else
	{
		DbgPrint("%ld\n", pCD->dwDrawStage);
	}

	return FALSE;
}

BOOL CUserInterfaceListCtrl::OnMouseMove(UINT nFlags, POINT point)
{
	if (m_pUI && m_pUI->m_bAutoHideHeader && m_pUI->m_bShowHeader)
	{
		DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
		if ((dwStyle & LVS_REPORT) == 0)
			return FALSE;

		DWORD dwAdd = 0, dwRemove = 0;

		if (point.y < 2)
		{
			if ((dwStyle & LVS_NOCOLUMNHEADER) == LVS_NOCOLUMNHEADER)
				dwRemove = LVS_NOCOLUMNHEADER;
		}
		else
		{
			if ((dwStyle & LVS_NOCOLUMNHEADER) == 0)
				dwAdd = LVS_NOCOLUMNHEADER;
		}

		if (dwAdd || dwRemove)
		{
			int nTop = ListView_GetTopIndex(m_hWnd);

			// highlight the first one so that
			// the listview won't scroll
			int nState = ListView_GetItemState(m_hWnd, nTop, LVIS_SELECTED|LVIS_FOCUSED);
			ListView_SetItemState(m_hWnd, nTop, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

			ModifyStyle(dwRemove, dwAdd);

			// restore the state
			ListView_SetItemState(m_hWnd, nTop, nState, LVIS_SELECTED|LVIS_FOCUSED);
		}

		return TRUE;
	}
	return FALSE;
}

BOOL CUserInterfaceListCtrl::OnNcMouseMove(UINT nFlags, POINT point)
{
	HideAutoHideHeader();

	return FALSE;
}

LRESULT CUserInterfaceListCtrl::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if ((message == WM_KEYDOWN) ||
		(message == WM_SYSKEYDOWN))
	{
		if (m_pUI)
		{
			if (m_pUI->ProcessAccelerator(m_hWnd, message, wParam, lParam))
				return TRUE;
		}
	}

	// want all the keys (including ENTER)
	else if (message == WM_GETDLGCODE)
		return DLGC_WANTALLKEYS;

	return CWnd::DefWindowProc(message, wParam, lParam);
}

BOOL CUserInterfaceListCtrl::EnsureVisible(int nItem, BOOL bPartialOK)
{
	if (m_hWnd == NULL) return FALSE;

	return ListView_EnsureVisible(m_hWnd, nItem, bPartialOK);
}

void CUserInterfaceListCtrl::SetCurSel(UINT nSel)
{
	if (m_hWnd == NULL) return;

	// un-select everything first
	UnSelectAll();

	// select the current one
	ListView_SetItemState(m_hWnd, nSel, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	ListView_SetSelectionMark(m_hWnd, nSel);
	EnsureVisible(nSel, FALSE);
}

void CUserInterfaceListCtrl::UnSelectAll()
{
	if (m_hWnd == NULL) return;

	// go through all the selected items
	int nItem = -1;
	while ((nItem = ListView_GetNextItem(m_hWnd, nItem, LVNI_SELECTED)) != -1)
	{
		ListView_SetItemState(m_hWnd, nItem, 0, LVIS_SELECTED|LVIS_FOCUSED);
	}
}

BOOL CUserInterfaceListCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	DbgPrint("CUserInterfaceListCtrl::OnChar(%c, %d, %d)\n", nChar, nRepCnt, nFlags);

	if (lstrlen(m_szInputBuffer) > sizeof(m_szInputBuffer) - 2) return TRUE;

	char szInput[2] = { (char)nChar, 0 };

	// reset text buffer
	if ((GetTickCount() - m_dwLastInputCount) > 500)
	{
		lstrcpy(m_szInputBuffer, szInput);
	}
	else
	{
		
		lstrcat(m_szInputBuffer, szInput);
	}

	DbgPrint("CUserInterfaceListCtrl::OnChar - Input buffer = %s\n", m_szInputBuffer);

	if (m_pUI) m_pUI->Search(m_szInputBuffer);

	m_dwLastInputCount = GetTickCount();

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// CUserInterfaceHeaderCtrl class

CUserInterfaceHeaderCtrl::CUserInterfaceHeaderCtrl()
{
}

CUserInterfaceHeaderCtrl::~CUserInterfaceHeaderCtrl()
{
}

LRESULT CUserInterfaceHeaderCtrl::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == HDM_LAYOUT)
	{
		return OnLayout((HDLAYOUT*) lParam);
	}

	return CWnd::DefWindowProc(message, wParam, lParam);
}

LRESULT CUserInterfaceHeaderCtrl::OnLayout(HDLAYOUT *pLayout)
{
	LRESULT lResult = CWnd::DefWindowProc(HDM_LAYOUT, 0, (LPARAM)pLayout);

	LOGFONT lf;
    GetObject((HFONT)SendMessage(WM_GETFONT), sizeof(LOGFONT), &lf);

    int height = abs(lf.lfHeight) + 9;

	// autohide header?
	if ((GetWindowLong(GetParent(), GWL_STYLE) & LVS_NOCOLUMNHEADER) == 0)
	{
		HDC hDC = GetDC(m_hWnd);
		if (hDC)
		{
			HFONT hFont = (HFONT)SendMessage(WM_GETFONT);
			HFONT hFontOld = (HFONT)SelectObject(hDC, hFont);

			TEXTMETRIC tm;
			if (GetTextMetrics(hDC, &tm))
			{
				height = tm.tmHeight + tm.tmExternalLeading + 9;
			}
			SelectObject(hDC, hFontOld);
			ReleaseDC(m_hWnd, hDC);
		}

		pLayout->pwpos->cy = height;
		pLayout->prc->top = height;

		if (m_pUI && m_pUI->m_bAutoHideHeader && m_pUI->m_bShowHeader)
		{
			pLayout->prc->top = 0;
		}
	}

	return lResult;
}
