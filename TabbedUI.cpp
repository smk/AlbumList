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
#include "UserInterface.h"
#include "TabbedUI.h"
#include "VisualStylesXP.h"


//////////////////////////////////////////////////////////////////
// CCustomTab class

CCustomTab::CCustomTab()
{
}

CCustomTab::~CCustomTab()
{
}

BOOL CCustomTab::OnPaint(HDC hdc)
{
	DbgPrint("TabCtrl - OnPaint (BEGIN)\n");

	// begin draw
	PAINTSTRUCT ps;
	BeginPaint(m_hWnd, &ps);

	// use the default if none is specified
	if (hdc == NULL) 
		hdc = ps.hdc;

	HBRUSH hbr = CreateSolidBrush(WADlg_getColor(WADLG_WNDBG));
	RECT rc;
	GetClientRect(&rc);
	FillRect(hdc, &rc, hbr);

	RECT rc2 = rc;
	TabCtrl_AdjustRect(m_hWnd, FALSE, &rc2);
	HPEN pen1=CreatePen(PS_SOLID,0,WADlg_getColor(WADLG_WNDFG));
	HGDIOBJ o = SelectObject(hdc, pen1);
	MoveToEx(hdc,rc2.left,rc2.top,NULL);
//	LineTo(hdc,rc2.right,rc2.top);

	SelectObject(hdc, o);
	DeleteObject(pen1);
	DeleteObject(hbr);

	int nTab = TabCtrl_GetItemCount(m_hWnd);
	int nSel = TabCtrl_GetCurSel(m_hWnd);
	while (nTab--)
	{
		if (nTab != nSel)
		{
			RECT rc;
			TabCtrl_GetItemRect(m_hWnd, nTab, &rc);
			DrawItem(hdc, nTab, rc, FALSE);
		}
		
	}

	TabCtrl_GetItemRect(m_hWnd, nSel, &rc);
	DrawItem(hdc, nSel, rc, TRUE);

	// cleanup
	EndPaint(m_hWnd, &ps);

	DbgPrint("TabCtrl - OnPaint (END)\n");

	return TRUE;
}

void CCustomTab::DrawItem(HDC hdc, int item, RECT rc, BOOL bRev)
{
	if (!bRev) rc.top += 2;
	HBRUSH hbr = CreateSolidBrush(WADlg_getColor(WADLG_WNDBG));
	FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);

DbgPrint("Item = %ld, Rev = %ld\n", item, bRev);

	HPEN pen1=CreatePen(PS_SOLID,0,WADlg_getColor(WADLG_LISTHEADER_FRAME_TOPCOLOR));
	HPEN pen2=CreatePen(PS_SOLID,0,WADlg_getColor(WADLG_LISTHEADER_FRAME_BOTTOMCOLOR));
	HPEN pen3=CreatePen(PS_SOLID,0,0);
	HGDIOBJ o=SelectObject(hdc, pen1);

	// left, top
	MoveToEx(hdc,rc.right,rc.bottom,NULL);
	bRev ? MoveToEx(hdc,rc.left,rc.bottom,NULL) : LineTo(hdc,rc.left,rc.bottom);
	LineTo(hdc,rc.left,rc.top);
	LineTo(hdc,rc.right,rc.top);

	// right, bottom
	SelectObject(hdc, pen2);
	MoveToEx(hdc,rc.right-2,rc.bottom-2,NULL);
	LineTo(hdc,rc.right-2,rc.top);

	// right, bottom (black line)
	SelectObject(hdc, pen3);
	MoveToEx(hdc,rc.right-1,rc.bottom-1,NULL);
	LineTo(hdc,rc.right-1,rc.top-1);

	// recreate the font
	char *fontname = (char *) wndWinamp.SendIPCMessage(1, IPC_GET_GENSKINBITMAP);
	DWORD charset = (DWORD) wndWinamp.SendIPCMessage(2, IPC_GET_GENSKINBITMAP);
	int fontsize = (int) wndWinamp.SendIPCMessage(3, IPC_GET_GENSKINBITMAP);

	HFONT hFont = CreateFont(-fontsize, 0, 0, 0, FW_REGULAR, 0, 0, 0, charset, 0/*OUT_TT_PRECIS*/, 0, DRAFT_QUALITY, 0, fontname);

	SelectObject(hdc, hFont);
	SetTextColor(hdc, WADlg_getColor(WADLG_WNDFG));
	SetBkMode(hdc, TRANSPARENT);

	char name[MAX_PATH] = "";
	TCITEM tci;
	tci.mask = TCIF_TEXT;
	tci.pszText = name;
	tci.cchTextMax = MAX_PATH;
	TabCtrl_GetItem(m_hWnd, item, &tci);

	RECT rcItem = rc;
	rcItem.left += 2;
	rcItem.top += 1;

	DrawTextW(hdc, AutoWide(tci.pszText, wndWinampAL.GetCodepage()), -1, &rcItem, DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	// clean up
	SelectObject(hdc, o);
	DeleteObject(pen1);
	DeleteObject(pen2);
	DeleteObject(pen3);
	DeleteObject(hFont);
}

BOOL CCustomTab::OnEraseBkgnd(HDC hdc)
{
	RECT rc;
	HBRUSH hbr = CreateSolidBrush(WADlg_getColor(WADLG_WNDBG));
	GetClientRect(&rc);
	FillRect(hdc, &rc, hbr);
	return TRUE;
}


//////////////////////////////////////////////////////////////////
// CTabbedUI class

CTabbedUI::CTabbedUI()
{
	m_nIDTemplate = IDD_TABBED_UI;
}

CTabbedUI::~CTabbedUI()
{
}

BOOL CTabbedUI::OnInitDialog()
{
	WADlg_init(wndWinamp);

	char *fontname = (char *) wndWinamp.SendIPCMessage(1, IPC_GET_GENSKINBITMAP);
	DWORD charset = (DWORD) wndWinamp.SendIPCMessage(2, IPC_GET_GENSKINBITMAP);
	int fontsize = (int) wndWinamp.SendIPCMessage(3, IPC_GET_GENSKINBITMAP);

	HFONT hFont = CreateFont(-fontsize, 0, 0, 0, FW_REGULAR, 0, 0, 0, charset, 0/*OUT_TT_PRECIS*/, 0, DRAFT_QUALITY, 0, fontname);


	m_wndTab.SubclassWindow(GetDlgItem(IDC_TAB));

	m_wndTab.SendMessage(WM_SETFONT, (WPARAM)hFont, 0);


	COLORREF textColor = WADlg_getColor(WADLG_WNDFG);
	char szColor[8];
	wsprintf(szColor, "#%02x%02x%02x", GetRValue(textColor), GetGValue(textColor), GetBValue(textColor));

	CTabCtrl wndTab = GetDlgItem(IDC_TAB);

	TCITEM tc;
	memset(&tc, 0, sizeof(TCITEM));
	tc.mask = TCIF_TEXT|TCIF_PARAM;

	int size = wndWinampAL.m_UserInterfaces.GetSize();
	for (int i=0; i<size; ++i)
	{
		CUserInterface *pUI = (CUserInterface *)wndWinampAL.m_UserInterfaces.GetAt(i);
		if (pUI)
		{
			tc.pszText = (LPTSTR)pUI->GetMLTitle();
			tc.lParam  = (LPARAM)pUI;
			wndTab.InsertItem(i, &tc);
		}
	}

	g_xpStyle.SetWindowTheme(wndTab, L" ", L" ");

	wndTab.SetCurSel(0);
	OnNotifyTCNSelChanged();

	return CDialog::OnInitDialog();
}

BOOL CTabbedUI::OnSize(UINT nType, int cx, int cy)
{
	CTabCtrl wndTab = GetDlgItem(IDC_TAB);

	wndTab.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE|SWP_NOZORDER);

	int nSel = wndTab.GetCurSel();
	if (nSel != -1)
	{
		TCITEM tc;
		memset(&tc, 0, sizeof(TCITEM));
		tc.mask = TCIF_PARAM;
		if (wndTab.GetItem(nSel, &tc))
		{
			CUserInterface *pUI = (CUserInterface *)tc.lParam;
			if (pUI != NULL)
			{
				RECT rc;
				wndTab.GetWindowRect(&rc);
				wndTab.AdjustRect(FALSE, &rc);
				ScreenToClient((LPPOINT)&rc.left);
				ScreenToClient((LPPOINT)&rc.right);

				pUI->SetWindowPos(HWND_TOP, rc.left, rc.top+5, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE);
				pUI->UpdateWindow();
			}
		}
	}

	return TRUE;
}

BOOL CTabbedUI::OnNotify(UINT nID, NMHDR *pnmh, LRESULT *pResult)
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

	case NM_CUSTOMDRAW:
		switch (pnmh->idFrom)
		{
		case IDC_TAB:
			return OnCustomTab(pnmh, pResult);
		}
		break;
	}

	return FALSE;
}

LRESULT CTabbedUI::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpdis;
	HWND hTabCtrl;

#if 0
	if (message == WM_ERASEBKGND)
	{
		HDC hDC = (HDC) wParam;

		RECT rc;
		GetClientRect(&rc);

		// fill with background color
		HBRUSH hbr = CreateSolidBrush(RGB(0xff,0,0));//WADlg_getColor(WADLG_WNDBG));
		FillRect(hDC, &rc, hbr);
		DeleteObject(hbr);

		return 1;
	}
#endif

	HBRUSH hbr = CreateSolidBrush(RGB(0xff,0,0));
	COLORREF bkColor = RGB(0xff,0,0);

#if 0
	switch (message)
	{
	case WM_DRAWITEM:
		lpdis = (LPDRAWITEMSTRUCT) lParam; // item drawing information
		hTabCtrl = m_wndTab.GetSafeHwnd();
		
		if (hTabCtrl == lpdis->hwndItem)   // is this the tab control?
		{
			char szTabText[MAX_PATH];
			memset(szTabText, '\0', sizeof(szTabText));
			
			TCITEM tci;
			tci.mask = TCIF_TEXT;
			tci.pszText = szTabText;
			tci.cchTextMax = sizeof(szTabText)-1;
			
			TabCtrl_GetItem(hTabCtrl, lpdis->itemID, &tci);
			
			FillRect(lpdis->hDC, &lpdis->rcItem, hbr);
			SetBkColor(lpdis->hDC, bkColor);
			
			TextOut(lpdis->hDC,
				lpdis->rcItem.left,
				lpdis->rcItem.top,
				tci.pszText,
				lstrlen(tci.pszText));
		}
		return TRUE;
	}

	LRESULT lRet = WADlg_handleDialogMsgs(m_hWnd, message, wParam, lParam);
	if (lRet) return lRet;
#endif

	return CDialog::DefWindowProc(message, wParam, lParam);
}

BOOL CTabbedUI::OnNotifyTCNSelChanging()
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

BOOL CTabbedUI::OnNotifyTCNSelChanged()
{
	CTabCtrl wndTab = GetDlgItem(IDC_TAB);

	int nSel = wndTab.GetCurSel();
	if (nSel == -1) return FALSE;

	TCITEM tc;
	memset(&tc, 0, sizeof(TCITEM));
	tc.mask = TCIF_PARAM;
	if (wndTab.GetItem(nSel, &tc))
	{
		CUserInterface *pUI = (CUserInterface *)tc.lParam;
		if (pUI != NULL)
		{
			RECT rc;
			wndTab.GetWindowRect(&rc);
			wndTab.AdjustRect(FALSE, &rc);
			ScreenToClient((LPPOINT)&rc.left);
			ScreenToClient((LPPOINT)&rc.right);

			pUI->MLPluginMessageProc(ML_MSG_TREE_ONCREATEVIEW, pUI->GetTreeItemID(), (LPARAM)GetSafeHwnd(), 0);
			pUI->SetWindowPos(HWND_TOP, rc.left, rc.top+5, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE);
			pUI->ShowWindow(SW_SHOW);
			pUI->UpdateWindow();
		}
	}

	return FALSE;
}

BOOL CTabbedUI::OnCustomTab(NMHDR* pnmh, LRESULT* pResult)
{
	NMCUSTOMDRAW *pnm = (NMCUSTOMDRAW *)pnmh;

	// Take the default processing unless we set this to something else below.
	*pResult = CDRF_DODEFAULT;

	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.
	if (CDDS_PREPAINT == pnm->dwDrawStage)
	{
			*pResult = CDRF_NOTIFYITEMDRAW;
			DbgPrint("PrePaint\n");
	}
	else if (CDDS_ITEMPREPAINT == pnm->dwDrawStage)
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		
		DbgPrint("ItemPrePaint\n");
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pnm->dwDrawStage )
	{
		*pResult = CDRF_SKIPDEFAULT;

		DbgPrint("SubItemPrePaint\n");
	}
	else if ( CDDS_ITEMPOSTPAINT == pnm->dwDrawStage )
	{
		DbgPrint("ItemPostPaint\n");
	}

	return TRUE;
}

