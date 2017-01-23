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
#include <ShellAPI.h>
#include <stdlib.h>
#include <mbstring.h>
#include <sys/stat.h>
#include <ole2.h>
#include <olectl.h>
#include <iimgctx.h>
#include <multimon.h>
#include <shlobj.h>
#include "util.h"
#include "resource.h"

#include "Gen_m3a.h"
#include "AutoWide.h"
#include "AutoChar.h"

// assume win2k/winxp
BOOL bWin9x = FALSE;
BOOL bWinNT = TRUE;

//////////////////////////////////////////////////////////////////
// CWnd class

CWnd::CWnd()
{
	m_hWnd = NULL;
	pWindowProc = NULL;
}

CWnd::CWnd(HWND hwnd)
{
	m_hWnd = hwnd;
	pWindowProc = NULL;
}

CWnd::~CWnd()
{
}

HWND CWnd::GetSafeHwnd()
{
	return ((this == NULL) || !IsWindow()) ? NULL : m_hWnd;
}

BOOL CWnd::IsWindow()
{
	if (m_hWnd == NULL) return FALSE;

	return ::IsWindow(m_hWnd);
}

CWnd CWnd::GetDlgItem(int nIDDlgItem)
{
	if (m_hWnd == NULL) return CWnd();

	return CWnd(::GetDlgItem(m_hWnd, nIDDlgItem));
}

int CWnd::CheckDlgButton(int nIDButton, UINT nCheck)
{
	if (m_hWnd == NULL) return 0;

	return ::CheckDlgButton(m_hWnd, nIDButton, nCheck);
}

UINT CWnd::IsDlgButtonChecked(int nIDButton)
{
	if (m_hWnd == NULL) return 0;

	return ::IsDlgButtonChecked(m_hWnd, nIDButton);
}

void CWnd::SetDlgItemText(int nID, LPCTSTR lpszString, int codepage /*=-1*/)
{
	if (m_hWnd == NULL) return;

	if (IsWindowUnicode(m_hWnd))
	{
		if (codepage == -1) codepage = wndWinampAL.GetCodepage();

		::SetDlgItemTextW(m_hWnd, nID, AutoWide(lpszString, codepage));
	}
	else
	{
		::SetDlgItemTextA(m_hWnd, nID, lpszString);
	}
}

void CWnd::SetDlgItemText(int nID, LPCWSTR lpszString, int codepage /*=-1*/)
{
	if (m_hWnd == NULL) return;

	if (IsWindowUnicode(m_hWnd))
	{
		::SetDlgItemTextW(m_hWnd, nID, lpszString);
	}
	else
	{
		if (codepage == -1) codepage = wndWinampAL.GetCodepage();

		::SetDlgItemTextA(m_hWnd, nID, AutoChar(lpszString, codepage));
	}
}

int CWnd::GetDlgItemText(int nID, LPTSTR lpStr, int nMaxCount, int codepage /*=-1*/)
{
	if (m_hWnd == NULL) return 0;

	if (IsWindowUnicode(m_hWnd))
	{
		if (codepage == -1) codepage = wndWinampAL.GetCodepage();

		WCHAR wstr[MAX_PATH];
		int nRet = ::GetDlgItemTextW(m_hWnd, nID, wstr, MAX_PATH);
		lstrcpyn(lpStr, AutoChar(wstr, codepage), nMaxCount);
		return nRet;
	}
	else
	{
		return ::GetDlgItemTextA(m_hWnd, nID, lpStr, nMaxCount);
	}
}

int CWnd::GetDlgItemText(int nID, LPWSTR lpStr, int nMaxCount, int codepage /*=-1*/)
{
	if (m_hWnd == NULL) return 0;

	if (IsWindowUnicode(m_hWnd))
	{
		return ::GetDlgItemTextW(m_hWnd, nID, lpStr, nMaxCount);
	}
	else
	{
		if (codepage == -1) codepage = wndWinampAL.GetCodepage();

		char str[MAX_PATH];
		int nRet = ::GetDlgItemTextA(m_hWnd, nID, str, MAX_PATH);
		wcsncpy(lpStr, AutoWide(str, codepage), nMaxCount);
		return nRet;
	}
}

int CWnd::SetDlgItemInt(int nID, UINT nValue, BOOL bSigned /*=TRUE*/)
{
	if (m_hWnd == NULL) return 0;

	return ::SetDlgItemInt(m_hWnd, nID, nValue, bSigned);
}

int CWnd::GetDlgItemInt(int nID, BOOL *lpTrans /*=NULL*/, BOOL bSigned /*=TRUE*/)
{
	if (m_hWnd == NULL) return 0;

	return ::GetDlgItemInt(m_hWnd, nID, lpTrans, bSigned);
}

LRESULT CWnd::SendDlgItemMessage(int nID, UINT message, WPARAM wParam /*=0*/, LPARAM lParam /*=0*/)
{
	if (m_hWnd == NULL) return 0;

	return ::SendDlgItemMessage(m_hWnd, nID, message, wParam, lParam);
}

BOOL CWnd::EndDialog(int nResult)
{
	if (m_hWnd == NULL) return 0;

	return ::EndDialog(m_hWnd, nResult);
}

CWnd CWnd::GetParent()
{
	if (m_hWnd == NULL) return CWnd();

	return CWnd(::GetParent(m_hWnd));
}

int CWnd::EnableWindow(BOOL bEnable /*=TRUE*/)
{
	if (m_hWnd == NULL) return 0;

	return ::EnableWindow(m_hWnd, bEnable);
}

BOOL CWnd::ShowWindow(int nCmdShow)
{
	if (m_hWnd == NULL) return FALSE;

	return ::ShowWindow(m_hWnd, nCmdShow);
}

BOOL CWnd::UpdateWindow()
{
	if (m_hWnd == NULL) return FALSE;

	return ::UpdateWindow(m_hWnd);
}

void CWnd::SetWindowText(LPCTSTR lpszString)
{
	if (m_hWnd == NULL) return;

	if (IsWindowUnicode(m_hWnd))
	{
		::SetWindowTextW(m_hWnd, AutoWide(lpszString, wndWinampAL.GetCodepage()));
	}
	else
	{
		::SetWindowTextA(m_hWnd, lpszString);
	}
}

void CWnd::SetWindowText(LPCWSTR lpszString)
{
	if (m_hWnd == NULL) return;

	if (IsWindowUnicode(m_hWnd))
	{
		::SetWindowTextW(m_hWnd, lpszString);
	}
	else
	{
		::SetWindowTextA(m_hWnd, AutoChar(lpszString, wndWinampAL.GetCodepage()));
	}
}

void CWnd::GetWindowText(LPTSTR lpszStringBuf, int nMaxCount)
{
	if (m_hWnd == NULL) return;

	if (IsWindowUnicode(m_hWnd))
	{
		WCHAR wstr[MAX_PATH];
		::GetWindowTextW(m_hWnd, wstr, MAX_PATH);
		lstrcpyn(lpszStringBuf, AutoChar(wstr, wndWinampAL.GetCodepage()), nMaxCount);
	}
	else
	{
		::GetWindowTextA(m_hWnd, lpszStringBuf, nMaxCount);
	}
}

void CWnd::GetWindowText(LPWSTR lpszStringBuf, int nMaxCount)
{
	if (m_hWnd == NULL) return;

	if (IsWindowUnicode(m_hWnd))
	{
		::GetWindowTextW(m_hWnd, lpszStringBuf, nMaxCount);
	}
	else
	{
		char str[MAX_PATH];
		::GetWindowTextA(m_hWnd, str, MAX_PATH);
		wcsncpy(lpszStringBuf, AutoWide(str, wndWinampAL.GetCodepage()), nMaxCount);
	}
}

BOOL CWnd::DestroyWindow()
{
	if (m_hWnd == NULL) return FALSE;

	return ::DestroyWindow(m_hWnd);
}

void CWnd::ScreenToClient(LPPOINT lpPoint)
{
	if (m_hWnd == NULL) return;

	::ScreenToClient(m_hWnd, lpPoint);
}

void CWnd::ScreenToClient(LPRECT lpRect)
{
	if (m_hWnd == NULL) return;

	::ScreenToClient(m_hWnd, (LPPOINT)lpRect);
	::ScreenToClient(m_hWnd, ((LPPOINT)lpRect)+1);
}

void CWnd::GetClientRect(LPRECT lpRect)
{
	if (m_hWnd == NULL) return;

	::GetClientRect(m_hWnd, lpRect);
}

void CWnd::GetWindowRect(LPRECT lpRect)
{
	if (m_hWnd == NULL) return;

	::GetWindowRect(m_hWnd, lpRect);
}

LRESULT CWnd::SendMessage(UINT message, WPARAM wParam /*=0*/, LPARAM lParam /*=0*/)
{
	if (!IsWindow()) return 0;

	return ::SendMessage(m_hWnd, message, wParam, lParam);
}
	
LRESULT CWnd::PostMessage(UINT message, WPARAM wParam /*=0*/, LPARAM lParam /*=0*/)
{
	if (m_hWnd == NULL) return 0;

	return ::PostMessage(m_hWnd, message, wParam, lParam);
}

BOOL CWnd::SetWindowPos(CWnd wndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
	if (m_hWnd == NULL) return FALSE;

	return ::SetWindowPos(m_hWnd, wndInsertAfter, x, y, cx, cy, nFlags);
}

HDWP CWnd::DeferWindowPos(HDWP hWinPosInfo, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
{
	if (m_hWnd == NULL) return NULL;

	return ::DeferWindowPos(hWinPosInfo, m_hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
}

void CWnd::CenterWindow(CWnd *pAlternateOwner /*=NULL*/)
{
	// determine owner window to center against
	DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	HWND hWndCenter = pAlternateOwner->GetSafeHwnd();
	if (hWndCenter == NULL)
	{
		hWndCenter = (dwStyle & WS_CHILD) ? ::GetParent(m_hWnd) : ::GetWindow(m_hWnd, GW_OWNER);
	}

	// get coordinates of the window relative to its parent
	CRect rcDlg;
	GetWindowRect(&rcDlg);
	CRect rcArea;
	CRect rcCenter;
	HWND hWndParent;
	if (!(dwStyle & WS_CHILD))
	{
		// don't center against invisible or minimized windows
		if (hWndCenter != NULL)
		{
			DWORD dwStyle = ::GetWindowLong(hWndCenter, GWL_STYLE);
			if (!(dwStyle & WS_VISIBLE) || (dwStyle & WS_MINIMIZE))
				hWndCenter = NULL;
		}

		MONITORINFO mi;
		mi.cbSize = sizeof(mi);

		// center within appropriate monitor coordinates
		::GetWindowRect(hWndCenter, &rcCenter);
		GetMonitorInfo(MonitorFromWindow(hWndCenter, MONITOR_DEFAULTTONEAREST), &mi);
		rcArea = mi.rcWork;
	}
	else
	{
		// center within parent client coordinates
		hWndParent = ::GetParent(m_hWnd);

		::GetClientRect(hWndParent, &rcArea);
		::GetClientRect(hWndCenter, &rcCenter);
		::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	}

	// find dialog's upper left based on rcCenter
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - rcDlg.Width() / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - rcDlg.Height() / 2;

	// if the dialog is outside the screen, move it inside
	if (xLeft < rcArea.left)
		xLeft = rcArea.left;
	else if (xLeft + rcDlg.Width() > rcArea.right)
		xLeft = rcArea.right - rcDlg.Width();

	if (yTop < rcArea.top)
		yTop = rcArea.top;
	else if (yTop + rcDlg.Height() > rcArea.bottom)
		yTop = rcArea.bottom - rcDlg.Height();

	// map screen coordinates to child coordinates
	SetWindowPos(NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL CWnd::OnInitDialog()
{
	//TODO: add localization stuff
	return FALSE;
}

int CWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return 0;
}

BOOL CWnd::OnPaint(HDC hdc)
{
	return FALSE;
}

BOOL CWnd::OnEraseBkgnd(HDC hdc)
{
	return FALSE;
}

BOOL CWnd::OnSize(UINT nType, int cx, int cy)
{
	return FALSE;
}

BOOL CWnd::OnCommand(WORD wNotifyCode, WORD wID, CWnd wndCtl)
{
	switch (wID)
	{
	case IDOK:
		OnOK();
		return TRUE;

	case IDCANCEL:
		OnCancel();
		return TRUE;
	}
	return FALSE;
}

void CWnd::OnOK()
{
	EndDialog(IDOK);
}

void CWnd::OnCancel()
{
	EndDialog(IDCANCEL);
}

BOOL CWnd::OnSysCommand(UINT uCmdType, short xPos, short yPos)
{
	return FALSE;
}

BOOL CWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	return FALSE;
}

BOOL CWnd::OnMouseMove(UINT nFlags, POINT point)
{
	return FALSE;
}

BOOL CWnd::OnLButtonDown(UINT nFlags, POINT point)
{
	return FALSE;
}

BOOL CWnd::OnNcMouseMove(UINT nHitTest, POINT point)
{
	return FALSE;
}

BOOL CWnd::OnNotify(UINT nID, NMHDR *pnmh, LRESULT *pResult)
{
	return FALSE;
}

BOOL CWnd::OnDisplayChange(int cBitsPerPixel, int cx, int cy)
{
	return FALSE;
}

BOOL CWnd::OnClose()
{
	return FALSE;
}

BOOL CWnd::OnDestroy()
{
	return FALSE;
}

BOOL CWnd::OnEndSession(BOOL bEnding)
{
	return FALSE;
}

BOOL CWnd::OnQueryEndSession()
{
	return TRUE;
}

BOOL CWnd::OnDeviceChange(UINT nEventType, DWORD dwData)
{
	return FALSE;
}

BOOL CWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	return FALSE;
}

BOOL CWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	return FALSE;
}

LRESULT CWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (IsWindowUnicode(m_hWnd))
	{
		if (pWindowProc)
			return CallWindowProcW(pWindowProc, m_hWnd, message, wParam, lParam);

		else
			return ::DefWindowProcW(m_hWnd, message, wParam, lParam);
	}
	else
	{
		if (pWindowProc)
			return CallWindowProcA(pWindowProc, m_hWnd, message, wParam, lParam);

		else
			return ::DefWindowProcA(m_hWnd, message, wParam, lParam);
	}
}

BOOL CWnd::SubclassWindow(CWnd wnd)
{
	if (!wnd.IsWindow()) return FALSE;

	m_hWnd = wnd;
	if (IsWindowUnicode(m_hWnd))
	{
		pWindowProc = (LRESULT(CALLBACK *)(HWND, UINT, WPARAM, LPARAM)) GetWindowLongW(wnd, GWL_WNDPROC);

		SetProp(wnd, "CWndAlbumList", this);
		SetWindowLongW(wnd, GWL_WNDPROC, (LONG)SubclassWindowProc);
	}
	else
	{
		pWindowProc = (LRESULT(CALLBACK *)(HWND, UINT, WPARAM, LPARAM)) GetWindowLongA(wnd, GWL_WNDPROC);

		m_hWnd = wnd;
		SetProp(wnd, "CWndAlbumList", this);
		SetWindowLongA(wnd, GWL_WNDPROC, (LONG)SubclassWindowProc);
	}

	return TRUE;
}

HWND CWnd::UnsubclassWindow()
{
	HWND hWnd = m_hWnd;

	if (pWindowProc)
	{
		RemoveProp(m_hWnd, "CWndAlbumList");
		if (GetWindowLong(m_hWnd, GWL_WNDPROC) == (LONG)SubclassWindowProc)
			SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)pWindowProc);
		m_hWnd = NULL;
	}

	return hWnd;
}

BOOL CWnd::ModifyStyle(int nStyleOffset, DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	if (m_hWnd == NULL) return FALSE;

	DWORD dwStyle = ::GetWindowLong(m_hWnd, nStyleOffset);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if (dwStyle == dwNewStyle)
		return FALSE;

	::SetWindowLong(m_hWnd, nStyleOffset, dwNewStyle);
	if (nFlags != 0)
	{
		SetWindowPos(NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
	}
	return TRUE;
}

BOOL CWnd::ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags /*=0*/)
{
	return ModifyStyle(GWL_STYLE, dwRemove, dwAdd, nFlags);
}

BOOL CWnd::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags /*=0*/)
{
	return ModifyStyle(GWL_EXSTYLE, dwRemove, dwAdd, nFlags);
}

LRESULT CALLBACK CWnd::SubclassWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CWnd *pWnd = (CWnd *)GetProp(hWnd, "CWndAlbumList");
	if (pWnd == NULL) return FALSE;

	BOOL	bRet	= FALSE;
	LRESULT lResult = 0;
	POINT	pt;

	switch (message)
	{
	case WM_NCDESTROY:
		RemoveProp(hWnd, "CWndAlbumList");
		return CallWindowProc(pWnd->pWindowProc, hWnd, message, wParam, lParam);

	case WM_INITDIALOG:
		bRet = pWnd->OnInitDialog();
		pWnd->TranslateStrings();
		break;

	case WM_PAINT:
		bRet = pWnd->OnPaint((HDC)wParam);
		break;

	case WM_ERASEBKGND:
		bRet = pWnd->OnEraseBkgnd((HDC)wParam);
		break;

	case WM_SIZE:
		bRet = pWnd->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_COMMAND:
		bRet = pWnd->OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;

	case WM_SYSCOMMAND:
		bRet = pWnd->OnSysCommand(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		break;

	case WM_NOTIFY:
		if (pWnd->OnNotify(wParam, (NMHDR *)lParam, &lResult))
		{
			return lResult;
		}
		break;

	case WM_DISPLAYCHANGE:
		bRet = pWnd->OnDisplayChange(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_SHOWWINDOW:
		bRet = pWnd->OnShowWindow(wParam, lParam);
		break;

	case WM_MOUSEMOVE:
		pt.x = (short)LOWORD(lParam);
		pt.y = (short)HIWORD(lParam);
		bRet = pWnd->OnMouseMove(wParam, pt);
		break;

	case WM_LBUTTONDOWN:
		pt.x = (short)LOWORD(lParam);
		pt.y = (short)HIWORD(lParam);
		bRet = pWnd->OnLButtonDown(wParam, pt);
		break;

	case WM_NCMOUSEMOVE:
		pt.x = (short)LOWORD(lParam);
		pt.y = (short)HIWORD(lParam);
		bRet = pWnd->OnNcMouseMove(wParam, pt);
		break;

	case WM_CLOSE:
		bRet = pWnd->OnClose();
		break;

	case WM_DESTROY:
		bRet = pWnd->OnDestroy();
		break;

	case WM_KEYDOWN:
		bRet = pWnd->OnKeyDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CHAR:
		bRet = pWnd->OnChar(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	}

	return bRet ? bRet : pWnd->DefWindowProc(message, wParam, lParam);
}

LRESULT CALLBACK CWnd::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ((message == WM_CREATE) && lParam)
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
		SetProp(hWnd, "CWndAlbumList", (HANDLE)lpcs->lpCreateParams);
		CWnd *pWnd = (CWnd *)lpcs->lpCreateParams;
		pWnd->m_hWnd = hWnd;
	}

	CWnd *pWnd = (CWnd *)GetProp(hWnd, "CWndAlbumList");
	if (pWnd == NULL) return ::DefWindowProc(hWnd, message, wParam, lParam);

	BOOL	bRet	= FALSE;
	LRESULT lResult = 0;
	POINT	pt;

	switch (message)
	{
	case WM_NCDESTROY:
		RemoveProp(hWnd, "CWndAlbumList");
		return ::DefWindowProc(hWnd, message, wParam, lParam);

	case WM_INITDIALOG:
		bRet = pWnd->OnInitDialog();
		pWnd->TranslateStrings();
		break;

	case WM_CREATE:
		bRet = pWnd->OnCreate((LPCREATESTRUCT)lParam);
		break;

	case WM_PAINT:
		bRet = pWnd->OnPaint((HDC)wParam);
		break;

	case WM_ERASEBKGND:
		bRet = pWnd->OnEraseBkgnd((HDC)wParam);
		break;

	case WM_SIZE:
		bRet = pWnd->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_COMMAND:
		bRet = pWnd->OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;

	case WM_SYSCOMMAND:
		bRet = pWnd->OnSysCommand(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		break;

	case WM_NOTIFY:
		if (pWnd->OnNotify(wParam, (NMHDR *)lParam, &lResult))
		{
			return lResult;
		}
		break;

	case WM_DISPLAYCHANGE:
		bRet = pWnd->OnDisplayChange(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_SHOWWINDOW:
		bRet = pWnd->OnShowWindow(wParam, lParam);
		break;

	case WM_MOUSEMOVE:
		pt.x = (short)LOWORD(lParam);
		pt.y = (short)HIWORD(lParam);
		bRet = pWnd->OnMouseMove(wParam, pt);
		break;

	case WM_LBUTTONDOWN:
		pt.x = (short)LOWORD(lParam);
		pt.y = (short)HIWORD(lParam);
		bRet = pWnd->OnLButtonDown(wParam, pt);
		break;

	case WM_CLOSE:
		bRet = pWnd->OnClose();
		break;

	case WM_DESTROY:
		bRet = pWnd->OnDestroy();
		break;

	case WM_ENDSESSION:
		bRet = pWnd->OnEndSession((BOOL)wParam);
		break;

	case WM_QUERYENDSESSION:
		if (pWnd->OnQueryEndSession())
			return pWnd->DefWindowProc(message, wParam, lParam);
		break;

	case WM_KEYDOWN:
		bRet = pWnd->OnKeyDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CHAR:
		bRet = pWnd->OnChar(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_DEVICECHANGE:
		pWnd->OnDeviceChange(wParam, lParam);
		break;
	}

	return bRet ? bRet : pWnd->DefWindowProc(message, wParam, lParam);
}

void CWnd::TranslateStrings()
{
	char str[MAX_PATH+1] = "";

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	GetWindowText(str, MAX_PATH);
	SendMessage(WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SetWindowText(ALS(str));

	CWnd child;
	while (child = FindWindowEx(m_hWnd, child, NULL, NULL))
	{
		GetClassName(child, str, MAX_PATH);

		if ((0   == lstrcmpi(str, "Static")) ||
			(0   == lstrcmpi(str, "Button")))
		{
			int nID = GetDlgCtrlID(child);
			if (nID != IDC_VERSION_STATIC)
				child.TranslateStrings();
			else
				child.SendMessage(WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
		}
		else
		{
			child.SendMessage(WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
		}
	}
}

//////////////////////////////////////////////////////////////////
// CDialog class

CDialog::CDialog()
{
	m_nIDTemplate = 0;
}

CDialog::CDialog(UINT nIDTemplate)
{
	m_nIDTemplate = nIDTemplate;
}

BOOL CDialog::Create(CWnd* pParentWnd /*=NULL*/, BOOL bUnicode /*=FALSE*/)
{
	return Create(m_nIDTemplate, pParentWnd, bUnicode);
}

BOOL CDialog::Create(UINT nIDTemplate, CWnd* pParentWnd /*=NULL*/, BOOL bUnicode /*=FALSE*/)
{
	HWND hParent = pParentWnd ? pParentWnd->GetSafeHwnd() : NULL;

	if (bUnicode)
	{
		m_hWnd = CreateDialogParamW(hDllInstance, MAKEINTRESOURCEW(nIDTemplate), hParent, DialogProc, (LPARAM)this);
	}
	else
	{
		m_hWnd = CreateDialogParamA(hDllInstance, MAKEINTRESOURCEA(nIDTemplate), hParent, DialogProc, (LPARAM)this);
	}

	return (m_hWnd != NULL);
}

BOOL CDialog::DoModal(CWnd* pParentWnd /*=NULL*/, BOOL bUnicode /*=FALSE*/)
{
	return DoModal(m_nIDTemplate, pParentWnd, bUnicode);
}

BOOL CDialog::DoModal(UINT nIDTemplate, CWnd* pParentWnd /*=NULL*/, BOOL bUnicode /*=FALSE*/)
{
	CWnd wnd = GetFocus();

	HWND hParent = pParentWnd ? pParentWnd->GetSafeHwnd() : NULL;

	BOOL bRet = FALSE;

	if (bUnicode)
	{
		bRet = DialogBoxParamW(hDllInstance, MAKEINTRESOURCEW(nIDTemplate), hParent, DialogProc, (LPARAM)this);
	}
	else
	{
		bRet = DialogBoxParamA(hDllInstance, MAKEINTRESOURCEA(nIDTemplate), hParent, DialogProc, (LPARAM)this);
	}

	SetFocus(wnd);

	return bRet;
}

LRESULT CDialog::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

BOOL CALLBACK CDialog::DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INITDIALOG)
	{
		if (lParam && !GetProp(hwnd, "CDialogAL"))
		{
			SetProp(hwnd, "CDialogAL", (HANDLE)lParam);
			CDialog *pWnd = (CDialog *)lParam;
			pWnd->m_hWnd = hwnd;
		}
	}

	CDialog *pWnd = (CDialog *)GetProp(hwnd, "CDialogAL");
	if (pWnd == NULL) return FALSE;

	BOOL	bRet	= FALSE;
	LRESULT lResult = 0;

	switch (message)
	{
	case WM_NCDESTROY:
		RemoveProp(hwnd, "CDialogAL");
		return FALSE;

	case WM_INITDIALOG:
		bRet = pWnd->OnInitDialog();
		pWnd->TranslateStrings();
		break;

	case WM_PAINT:
		bRet = pWnd->OnPaint((HDC)wParam);
		break;

	case WM_ERASEBKGND:
		bRet = pWnd->OnEraseBkgnd((HDC)wParam);
		break;

	case WM_SIZE:
		bRet = pWnd->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_COMMAND:
		bRet = pWnd->OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;

	case WM_NOTIFY:
		if (pWnd->OnNotify(wParam, (NMHDR *)lParam, &lResult))
		{
	        // This is the way a dialog box can send back lresults.. 
		    // See documentation for DialogProc for more information
			SetWindowLong(hwnd, DWL_MSGRESULT, lResult); 
			return TRUE;
		}
		break;

	case WM_DISPLAYCHANGE:
		bRet = pWnd->OnDisplayChange(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:
		bRet = pWnd->OnClose();
		break;

	case WM_DESTROY:
		bRet = pWnd->OnDestroy();
		break;
	}

	return bRet ? bRet : pWnd->DefWindowProc(message, wParam, lParam);
}

//////////////////////////////////////////////////////////////////
// CComboBox class

CComboBox::CComboBox() : CWnd()
{
}

CComboBox::CComboBox(HWND hwnd) : CWnd(hwnd)
{
}

int CComboBox::AddString(LPCTSTR lpszString, int codepage /*=-1*/)
{
	if (m_hWnd == NULL) return 0;

	if (IsWindowUnicode(m_hWnd))
	{
		if (codepage == -1) codepage = wndWinampAL.GetCodepage();
		return ::SendMessageW(m_hWnd, CB_ADDSTRING, 0, (LPARAM)(LPCWSTR)AutoWide(lpszString, codepage));
	}
	else
	{
		return ::SendMessageA(m_hWnd, CB_ADDSTRING, 0, (LPARAM)lpszString);
	}
}

int CComboBox::SetCurSel(int nSelect)
{
	if (m_hWnd == NULL) return CB_ERR;

	return SendMessage(CB_SETCURSEL, nSelect);
}

int CComboBox::SetItemData(int nIndex, DWORD dwItemData)
{
	if (m_hWnd == NULL) return CB_ERR;

	return SendMessage(CB_SETITEMDATA, nIndex, dwItemData);
}

int CComboBox::GetCurSel()
{
	if (m_hWnd == NULL) return CB_ERR;

	return SendMessage(CB_GETCURSEL);
}

DWORD CComboBox::GetItemData(int nIndex)
{
	if (m_hWnd == NULL) return CB_ERR;

	return SendMessage(CB_GETITEMDATA, nIndex);
}

int CComboBox::GetCount()
{
	if (m_hWnd == NULL) return CB_ERR;

	return SendMessage(CB_GETCOUNT);
}

int CComboBox::GetLBText(int nIndex, LPTSTR lpszText)
{
	if (m_hWnd == NULL) return CB_ERR;

	if (IsWindowUnicode(m_hWnd))
	{
		WCHAR wcText[MAX_PATH];
		int nRet = ::SendMessageW(m_hWnd, CB_GETLBTEXT, nIndex, (LPARAM)wcText);
		if (nRet) lstrcpy(lpszText, AutoChar(wcText, wndWinampAL.GetCodepage()));
		return nRet;
	}
	else
	{
		return ::SendMessageA(m_hWnd, CB_GETLBTEXT, nIndex, (LPARAM)lpszText);
	}
}

int CComboBox::GetLBText(int nIndex, LPWSTR lpszText)
{
	if (m_hWnd == NULL) return CB_ERR;

	if (IsWindowUnicode(m_hWnd))
	{
		return ::SendMessageW(m_hWnd, CB_GETLBTEXT, nIndex, (LPARAM)lpszText);
	}
	else
	{
		char szText[MAX_PATH];
		int nRet = ::SendMessageA(m_hWnd, CB_GETLBTEXT, nIndex, (LPARAM)szText);
		if (nRet) lstrcpyW(lpszText, AutoWide(szText, wndWinampAL.GetCodepage()));
		return nRet;
	}
}

//////////////////////////////////////////////////////////////////
// CTabCtrl class

CTabCtrl::CTabCtrl() : CWnd()
{
}

CTabCtrl::CTabCtrl(HWND hwnd) : CWnd(hwnd)
{
}

BOOL CTabCtrl::InsertItem(int nItem, TCITEM* pTabCtrlItem)
{
	if (m_hWnd == NULL) return -1;
	if (pTabCtrlItem == NULL) return -1;

	if (IsWindowUnicode(m_hWnd))
	{
		AutoWide wc(pTabCtrlItem->pszText, wndWinampAL.GetCodepage());

		TCITEMW itemW;
		itemW.mask			= pTabCtrlItem->mask;
		itemW.dwState		= pTabCtrlItem->dwState;
		itemW.dwStateMask	= pTabCtrlItem->dwStateMask;
		itemW.pszText		= wc;
		itemW.cchTextMax	= pTabCtrlItem->cchTextMax;
		itemW.iImage		= pTabCtrlItem->iImage;
		itemW.lParam		= pTabCtrlItem->lParam;

		return ::SendMessageW(m_hWnd, TCM_INSERTITEMW, nItem, (LPARAM)&itemW);
	}
	else
	{
		return ::SendMessageA(m_hWnd, TCM_INSERTITEMA, nItem, (LPARAM)pTabCtrlItem);
	}
}

int CTabCtrl::SetCurSel(int nItem)
{
	if (m_hWnd == NULL) return -1;

	return SendMessage(TCM_SETCURSEL, nItem);
}

int CTabCtrl::GetCurSel()
{
	if (m_hWnd == NULL) return -1;

	return SendMessage(TCM_GETCURSEL);
}

int CTabCtrl::GetItemCount()
{
	if (m_hWnd == NULL) return -1;

	return SendMessage(TCM_GETITEMCOUNT);
}

BOOL CTabCtrl::GetItem(int nItem, TCITEM* pTabCtrlItem)
{
	if (m_hWnd == NULL) return -1;
	if (pTabCtrlItem == NULL) return -1;

	if (IsWindowUnicode(m_hWnd))
	{
		WCHAR *pwc = pTabCtrlItem->cchTextMax ? (WCHAR*)malloc(pTabCtrlItem->cchTextMax * sizeof(WCHAR)) : NULL;
		
		TCITEMW itemW;
		itemW.mask			= pTabCtrlItem->mask;
		itemW.dwState		= pTabCtrlItem->dwState;
		itemW.dwStateMask	= pTabCtrlItem->dwStateMask;
		itemW.pszText		= pwc;
		itemW.cchTextMax	= pTabCtrlItem->cchTextMax;
		itemW.iImage		= pTabCtrlItem->iImage;
		itemW.lParam		= pTabCtrlItem->lParam;

		BOOL bRet = ::SendMessageW(m_hWnd, TCM_GETITEMW, nItem, (LPARAM)&itemW);

		pTabCtrlItem->mask			= itemW.mask;
		pTabCtrlItem->dwState		= itemW.dwState;
		pTabCtrlItem->dwStateMask	= itemW.dwStateMask;
		if (pwc) lstrcpyn(pTabCtrlItem->pszText, AutoChar(pwc, wndWinampAL.GetCodepage()), pTabCtrlItem->cchTextMax);
		pTabCtrlItem->cchTextMax	= itemW.cchTextMax;
		pTabCtrlItem->iImage		= itemW.iImage;
		pTabCtrlItem->lParam		= itemW.lParam;

		if (pwc) free(pwc);

		return bRet;
	}
	else
	{
		return ::SendMessageA(m_hWnd, TCM_GETITEMA, nItem, (LPARAM)pTabCtrlItem);
	}
}

void CTabCtrl::AdjustRect(BOOL bLarger, LPRECT lpRect)
{
	SendMessage(TCM_ADJUSTRECT, bLarger, (LPARAM)lpRect);
}

//////////////////////////////////////////////////////////////////
// CListBox class

CListBox::CListBox() : CWnd()
{
}

CListBox::CListBox(HWND hwnd) : CWnd(hwnd)
{
}

int CListBox::AddString(LPCTSTR lpszString, int codepage /*=-1*/)
{
	if (m_hWnd == NULL) return 0;

	if (IsWindowUnicode(m_hWnd))
	{
		if (codepage == -1) codepage = wndWinampAL.GetCodepage();
		return ::SendMessageW(m_hWnd, LB_ADDSTRING, 0, (LPARAM)(LPCWSTR)AutoWide(lpszString, codepage));
	}
	else
	{
		return ::SendMessageA(m_hWnd, LB_ADDSTRING, 0, (LPARAM)lpszString);
	}
}

int CListBox::AddString(LPCWSTR lpszString, int codepage /*=-1*/)
{
	if (m_hWnd == NULL) return 0;

	if (IsWindowUnicode(m_hWnd))
	{
		return ::SendMessageW(m_hWnd, LB_ADDSTRING, 0, (LPARAM)lpszString);
	}
	else
	{
		if (codepage == -1) codepage = wndWinampAL.GetCodepage();
		return ::SendMessageA(m_hWnd, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)AutoChar(lpszString, codepage));
	}
}

int CListBox::SetCurSel(int nSelect)
{
	if (m_hWnd == NULL) return LB_ERR;

	return SendMessage(LB_SETCURSEL, nSelect);
}

int CListBox::SetItemData(int nIndex, DWORD dwItemData)
{
	if (m_hWnd == NULL) return LB_ERR;

	return SendMessage(LB_SETITEMDATA, nIndex, dwItemData);
}

int CListBox::GetCurSel()
{
	if (m_hWnd == NULL) return LB_ERR;

	return SendMessage(LB_GETCURSEL);
}

DWORD CListBox::GetItemData(int nIndex)
{
	if (m_hWnd == NULL) return LB_ERR;

	return SendMessage(LB_GETITEMDATA, nIndex);
}

int CListBox::GetCount()
{
	if (m_hWnd == NULL) return LB_ERR;

	return SendMessage(LB_GETCOUNT);
}

void CListBox::ResetContent()
{
	if (m_hWnd == NULL) return;

	SendMessage(LB_RESETCONTENT);
}

int CListBox::GetText(int nIndex, LPTSTR lpszBuffer)
{
	if (m_hWnd == NULL) return LB_ERR;

	if (IsWindowUnicode(m_hWnd))
	{
		int nTextLen = GetTextLen(nIndex);
		if (nTextLen == LB_ERR) return LB_ERR;

		wchar_t *wstr = new wchar_t [nTextLen+1];
		int nRet = SendMessageW(m_hWnd, LB_GETTEXT, nIndex, (LPARAM)wstr);
		lstrcpy(lpszBuffer, AutoChar(wstr));
		delete [] wstr;
		return nRet;
	}
	else
	{
		return ::SendMessageA(m_hWnd, LB_GETTEXT, nIndex, (LPARAM)lpszBuffer);
	}
}

int CListBox::GetText(int nIndex, LPWSTR lpszBuffer)
{
	if (m_hWnd == NULL) return LB_ERR;

	if (IsWindowUnicode(m_hWnd))
	{
		return ::SendMessageW(m_hWnd, LB_GETTEXT, nIndex, (LPARAM)lpszBuffer);
	}
	else
	{
		int nTextLen = GetTextLen(nIndex);
		if (nTextLen == LB_ERR) return LB_ERR;

		char *str = new char [nTextLen+1];
		int nRet = ::SendMessageA(m_hWnd, LB_GETTEXT, nIndex, (LPARAM)str);
		wcscpy(lpszBuffer, AutoWide(str));
		delete [] str;
		return nRet;
	}
}

int CListBox::GetTextLen(int nIndex)
{
	if (m_hWnd == NULL) return LB_ERR;

	if (IsWindowUnicode(m_hWnd))
	{
		return ::SendMessageW(m_hWnd, LB_GETTEXTLEN, nIndex, 0);
	}
	else
	{
		return ::SendMessageA(m_hWnd, LB_GETTEXTLEN, nIndex, 0);
	}
}

//////////////////////////////////////////////////////////////////
// CListCtrl class

CListCtrl::CListCtrl() : CWnd()
{
}

CListCtrl::CListCtrl(HWND hwnd) : CWnd(hwnd)
{
}

int CListCtrl::InsertItem(const LVITEM* pItem)
{
	if (ListView_GetUnicodeFormat(m_hWnd))
	{
		AutoWide wc(pItem->pszText, wndWinampAL.GetCodepage());

		LVITEMW lvi;
		memset(&lvi, 0, sizeof(LVITEMW));
		lvi.mask		= pItem->mask;
		lvi.iItem		= pItem->iItem;
		lvi.iSubItem	= pItem->iSubItem;
		lvi.state		= pItem->state;
		lvi.stateMask	= pItem->stateMask;
		lvi.pszText		= wc;
		lvi.cchTextMax	= pItem->cchTextMax;
		lvi.iImage		= pItem->iImage;
		lvi.lParam		= pItem->lParam;
		lvi.iIndent		= pItem->iIndent;

		return ::SendMessageW(m_hWnd, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
	}
	else
	{
		return ::SendMessageA(m_hWnd, LVM_INSERTITEMA, 0, (LPARAM)pItem);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////
// CMenu class

CMenu::CMenu()
{
	m_hMenu = NULL;
	m_bUnicode = TRUE;
}

CMenu::CMenu(HMENU hMenu)
{
	m_hMenu = hMenu;
	m_bUnicode = TRUE;
}

CMenu::~CMenu()
{
	DestroyMenu();
}

BOOL CMenu::CreateMenu()
{
	return ((m_hMenu = ::CreateMenu()) != NULL);
}

BOOL CMenu::CreatePopupMenu()
{
	return ((m_hMenu = ::CreatePopupMenu()) != NULL);
}

BOOL CMenu::DestroyMenu()
{
	BOOL bRet = 0;
	if (m_hMenu) bRet = ::DestroyMenu(m_hMenu);
	m_hMenu = NULL;
	return bRet;
}

BOOL CMenu::DeleteMenu(UINT nPosition, UINT nFlags)
{
	if (!m_hMenu) return FALSE;

	return ::DeleteMenu(m_hMenu, nPosition, nFlags);
}

BOOL CMenu::TrackPopupMenu(UINT nFlags, int x, int y, CWnd* pWnd, LPCRECT lpRect /*=NULL*/)
{
	if (!m_hMenu) return FALSE;

	return ::TrackPopupMenu(m_hMenu, nFlags, x, y, 0, pWnd->GetSafeHwnd(), lpRect);
}

BOOL CMenu::AppendMenu(UINT nFlags, UINT nIDNewItem /*=0*/, LPCTSTR lpszNewItem /*=NULL*/)
{
	if (!m_hMenu) return FALSE;

	if ((nFlags & (MF_BITMAP|MF_OWNERDRAW)) == 0)	// MF_STRING = 0
	{
		if (m_bUnicode)
		{
			return ::AppendMenuW(m_hMenu, nFlags, nIDNewItem, AutoWide(lpszNewItem, wndWinampAL.GetCodepage()));
		}
	}

	return ::AppendMenuA(m_hMenu, nFlags, nIDNewItem, lpszNewItem);
}

BOOL CMenu::AppendMenu(UINT nFlags, UINT nIDNewItem /*=0*/, LPCWSTR lpszNewItem /*=NULL*/)
{
	if (!m_hMenu) return FALSE;

	if ((nFlags & (MF_BITMAP|MF_OWNERDRAW)) == 0)	// MF_STRING = 0
	{
		if (!m_bUnicode)
		{
			return ::AppendMenuA(m_hMenu, nFlags, nIDNewItem, AutoChar(lpszNewItem, wndWinampAL.GetCodepage()));
		}
	}

	return ::AppendMenuW(m_hMenu, nFlags, nIDNewItem, lpszNewItem);
}

UINT CMenu::CheckMenuItem(UINT nIDCheckItem, UINT nCheck)
{
	if (!m_hMenu) return 0xFFFFFFFF;

	return ::CheckMenuItem(m_hMenu, nIDCheckItem, nCheck);
}

UINT CMenu::EnableMenuItem(UINT nIDEnableItem, UINT nEnable)
{
	if (!m_hMenu) return 0xFFFFFFFF;

	return ::EnableMenuItem(m_hMenu, nIDEnableItem, nEnable);
}

BOOL CMenu::CheckMenuRadioItem(UINT nIDFirst, UINT nIDLast, UINT nIDItem, UINT nFlags)
{
	if (!m_hMenu) return 0;

	return ::CheckMenuRadioItem(m_hMenu, nIDFirst, nIDLast, nIDItem, nFlags);
}

BOOL CMenu::SetDefaultItem(UINT uItem, UINT fByPos /*=FALSE*/)
{
	if (!m_hMenu) return 0;

	return ::SetMenuDefaultItem(m_hMenu, uItem, fByPos);
}

//////////////////////////////////////////////////////////////////
// CPtrArray class

CPtrArray::CPtrArray()
{
	m_nAllocated= 0;
	m_nGrowBy	= 16;
	m_pData		= NULL;

	m_nSize		= 0;
	m_pCompare	= NULL;

	InitializeCriticalSection(&m_csMutex);
}

CPtrArray::~CPtrArray()
{
	DeleteCriticalSection(&m_csMutex);

	if (m_pData) delete [] (LPBYTE)m_pData;
	m_nSize = 0;
}

int CPtrArray::GetSize()
{
	CCriticalSection cs(&m_csMutex);
	return m_nSize;
}

void CPtrArray::SetSize( int nNewSize, int nGrowBy /*=-1*/)
{
	CCriticalSection cs(&m_csMutex);

	if (nNewSize == 0)
	{
		RemoveAll();
		if (nGrowBy > 0) m_nGrowBy = nGrowBy;
		return;
	}
	// array already allocated?
	else if (m_pData)
	{
		LPVOID *pNewData = (LPVOID *) new BYTE [ nNewSize*sizeof(LPVOID) ];
		memcpy(pNewData, m_pData, m_nSize*sizeof(LPVOID));
		delete [] (LPBYTE)m_pData;
		m_pData = pNewData;
	}
	else
	{
		m_pData = (LPVOID *) new BYTE [ nNewSize*sizeof(LPVOID) ];
		memset(m_pData, 0, nNewSize*sizeof(LPVOID));
	}

	// update some internal variables
	m_nAllocated = nNewSize;
	if (nGrowBy > 0) m_nGrowBy = nGrowBy;

	// reducing the size? update the number of valid elements
	if (m_nSize > nNewSize) m_nSize = nNewSize;
}

int CPtrArray::Add(LPVOID pItem)
{
	CCriticalSection cs(&m_csMutex);

	// allocate memory if neccessary
	if (m_nAllocated <= m_nSize)
	{
		SetSize(m_nAllocated + m_nGrowBy);
	}

	m_pData[m_nSize] = pItem;
	m_nSize++;

	return m_nSize;
}

void CPtrArray::SetAt(int nIndex, LPVOID pItem)
{
	CCriticalSection cs(&m_csMutex);

	if (nIndex < 0) return;
	if (m_nSize < nIndex + 1) return;

	m_pData[nIndex] = pItem;
	if (m_nSize <= nIndex) m_nSize = nIndex + 1;
}

LPVOID CPtrArray::GetAt(int nIndex)
{
	CCriticalSection cs(&m_csMutex);

	if (nIndex < 0) return NULL;
	if (m_nSize < nIndex + 1) return NULL;

	return m_pData[nIndex];
}

void CPtrArray::Swap(int from, int to)
{
	CCriticalSection cs(&m_csMutex);

	if (m_nSize <= from) return;
	if (m_nSize <= to) return;

	LPVOID temp		= m_pData[to];
	m_pData[to]		= m_pData[from];
	m_pData[from]	= temp;
}

void CPtrArray::InsertAt(int nIndex, LPVOID pItem)
{
	CCriticalSection cs(&m_csMutex);

	if (nIndex < 0) return;
	if (m_nSize < nIndex) return;

	// allocate memory if neccessary
	if (m_nAllocated <= m_nSize)
	{
		SetSize(m_nAllocated + m_nGrowBy);
	}

	memmove(&m_pData[nIndex+1], &m_pData[nIndex], sizeof(LPVOID)*(m_nSize - nIndex));
	m_pData[nIndex] = pItem;

	m_nSize ++;
}

void CPtrArray::RemoveAt(int nIndex, int nCount /*=1*/)
{
	CCriticalSection cs(&m_csMutex);

	if (nIndex < 0) return;
	if (m_nSize < nIndex + 1) return;

	if (m_nSize >= nIndex + nCount)
	{
		memmove(&m_pData[nIndex], &m_pData[nIndex+nCount], sizeof(LPVOID)*(m_nSize-nIndex-nCount));
	}

	m_nSize -= nCount;
}

void CPtrArray::RemoveAll()
{
	CCriticalSection cs(&m_csMutex);

	if (m_pData) delete [] (LPBYTE)m_pData;

	m_nAllocated = 0;
	m_pData = NULL;
	m_nSize = 0;
}

LPVOID CPtrArray::operator[](int nIndex)
{
	CCriticalSection cs(&m_csMutex);

	if (nIndex < 0) return NULL;
	if (nIndex < m_nSize) return (LPVOID)m_pData[nIndex];

	return NULL;
}

LPVOID *CPtrArray::GetData()
{
	CCriticalSection cs(&m_csMutex);

	return m_pData;
}

void CPtrArray::Sort()
{
	CCriticalSection cs(&m_csMutex);

	if (m_nSize && m_pCompare)
	{
		qsort(m_pData, m_nSize, sizeof(LPVOID), m_pCompare);
	}
}

//////////////////////////////////////////////////////////////////
// CStringArray class

CStringArray::CStringArray() : CPtrArray()
{
	m_pCompare = Compare;
}

CStringArray::~CStringArray()
{
	RemoveAll();
}

int CStringArray::Add(LPCTSTR szItem)
{
	CCriticalSection cs(&m_csMutex);

	int len = lstrlen(szItem);

	char *str = new char [len+1];
	lstrcpy(str, szItem);

	return CPtrArray::Add(str);
}

void CStringArray::SetAt(int nIndex, LPCTSTR szItem)
{
	CCriticalSection cs(&m_csMutex);

	int len = lstrlen(szItem);

	char *str = new char [len+1];
	lstrcpy(str, szItem);

	CPtrArray::SetAt(nIndex, str);
}

void CStringArray::InsertAt(int nIndex, LPCTSTR szItem)
{
	CCriticalSection cs(&m_csMutex);

	int len = lstrlen(szItem);

	char *str = new char [len+1];
	lstrcpy(str, szItem);

	CPtrArray::InsertAt(nIndex, str);
}

void CStringArray::RemoveAt(int nIndex, int nCount /*=1*/)
{
	CCriticalSection cs(&m_csMutex);

	for (int i=nIndex; i<nIndex+nCount; i++)
	{
		char *str = (char*)GetAt(i);
		if (str) delete [] str;
	}

	CPtrArray::RemoveAt(nIndex, nCount);
}

void CStringArray::RemoveAll()
{
	CCriticalSection cs(&m_csMutex);

	for (int i=0; i<m_nSize; i++)
	{
		delete [] (char*)GetAt(i);
	}

	CPtrArray::RemoveAll();
}

LPCTSTR CStringArray::operator[](int nIndex)
{
	CCriticalSection cs(&m_csMutex);

	return (LPCTSTR)CPtrArray::GetAt(nIndex);
}

int CStringArray::Compare(const void *elem1, const void *elem2)
{
	LPCTSTR str1 = *(LPCTSTR*)elem1;
	LPCTSTR str2 = *(LPCTSTR*)elem2;

	return lstrcmp(str1, str2);
}

//////////////////////////////////////////////////////////////////
// CStringArrayW class

CStringArrayW::CStringArrayW() : CPtrArray()
{
	m_pCompare = Compare;
}

CStringArrayW::~CStringArrayW()
{
	RemoveAll();
}

int CStringArrayW::Add(LPCWSTR szItem)
{
	CCriticalSection cs(&m_csMutex);

	int len = lstrlenW(szItem);

	WCHAR *str = new WCHAR [len+1];
	lstrcpyW(str, szItem);

	return CPtrArray::Add(str);
}

void CStringArrayW::SetAt(int nIndex, LPCWSTR szItem)
{
	CCriticalSection cs(&m_csMutex);

	int len = lstrlenW(szItem);

	WCHAR *str = new WCHAR [len+1];
	lstrcpyW(str, szItem);

	CPtrArray::SetAt(nIndex, str);
}

void CStringArrayW::InsertAt(int nIndex, LPCWSTR szItem)
{
	CCriticalSection cs(&m_csMutex);

	int len = lstrlenW(szItem);

	WCHAR *str = new WCHAR [len+1];
	lstrcpyW(str, szItem);

	CPtrArray::InsertAt(nIndex, str);
}

void CStringArrayW::RemoveAt(int nIndex, int nCount /*=1*/)
{
	CCriticalSection cs(&m_csMutex);

	for (int i=nIndex; i<nIndex+nCount; i++)
	{
		WCHAR *str = (WCHAR*)GetAt(i);
		if (str) delete [] str;
	}

	CPtrArray::RemoveAt(nIndex, nCount);
}

void CStringArrayW::RemoveAll()
{
	CCriticalSection cs(&m_csMutex);

	for (int i=0; i<m_nSize; i++)
	{
		delete [] (WCHAR*)GetAt(i);
	}

	CPtrArray::RemoveAll();
}

LPCWSTR CStringArrayW::operator[](int nIndex)
{
	CCriticalSection cs(&m_csMutex);

	return (LPCWSTR)CPtrArray::GetAt(nIndex);
}

int CStringArrayW::Compare(const void *elem1, const void *elem2)
{
	LPCWSTR str1 = *(LPCWSTR*)elem1;
	LPCWSTR str2 = *(LPCWSTR*)elem2;

	return lstrcmpW(str1, str2);
}

//////////////////////////////////////////////////////////////////
// CTokens class

CTokens::CTokens(LPCTSTR str, LPCTSTR seps) : CPtrArray()
{
	lstrcpyn(string, str, MAX_PATH);

	// put the tokens into a pointer array
	char *token = (char *)_mbstok((unsigned char *)string, (const unsigned char *)seps);
	while (token != NULL)
	{
		Add(token);
		token = (char *)_mbstok(NULL, (const unsigned char *)seps);
	}
}

CTokens::~CTokens()
{
}

LPCTSTR CTokens::operator[](int nIndex)
{
	return (LPCTSTR)GetAt(nIndex);
}

void CTokens::Parse(LPCTSTR str, LPCTSTR seps)
{
	RemoveAll();

	lstrcpyn(string, str, MAX_PATH);

	// put the tokens into a pointer array
	char *token = (char *)_mbstok((unsigned char *)string, (const unsigned char *)seps);
	while (token != NULL)
	{
		Add(token);
		token = (char *)_mbstok(NULL, (const unsigned char *)seps);
	}
}

//////////////////////////////////////////////////////////////////
// CTokensW class

CTokensW::CTokensW(LPCWSTR str, LPCWSTR seps) : CPtrArray()
{
	lstrcpynW(string, str, MAX_PATH);

	// put the tokens into a pointer array
	WCHAR *token = wcstok(string, seps);
	while (token != NULL)
	{
		Add(token);
		token = wcstok(NULL, seps);
	}
}

CTokensW::~CTokensW()
{
}

LPCWSTR CTokensW::operator[](int nIndex)
{
	return (LPCWSTR)GetAt(nIndex);
}

void CTokensW::Parse(LPCWSTR str, LPCWSTR seps)
{
	RemoveAll();

	lstrcpynW(string, str, MAX_PATH);

	// put the tokens into a pointer array
	WCHAR *token = wcstok(string, seps);
	while (token != NULL)
	{
		Add(token);
		token = wcstok(NULL, seps);
	}
}

//////////////////////////////////////////////////////////////////
// CNamingTokens class

CNamingTokens::CNamingTokens(LPCTSTR str) : CPtrArray()
{
	lstrcpyn(string, str, MAX_PATH);

	// put the tokens into a pointer array
	char *ptr = string;
	do
	{
		Add(ptr);
		ptr = (char *)_mbsstr((const unsigned char *)ptr, (const unsigned char *)" - ");
		if (ptr)
		{
			*ptr = 0;
			ptr += 3;
		}
	}
	while (ptr);
}

CNamingTokens::~CNamingTokens()
{
}

LPCTSTR CNamingTokens::operator[](int nIndex)
{
	return (LPCTSTR)GetAt(nIndex);
}

void CNamingTokens::Parse(LPCTSTR str)
{
	RemoveAll();

	lstrcpyn(string, str, MAX_PATH);

	// put the tokens into a pointer array
	char *ptr = string;
	do
	{
		Add(ptr);
		ptr = (char *)_mbsstr((const unsigned char *)ptr, (const unsigned char *)" - ");
		if (ptr)
		{
			*ptr = 0;
			ptr += 3;
		}
	}
	while (ptr);
}

//////////////////////////////////////////////////////////////////
// CNamingTokensW class

CNamingTokensW::CNamingTokensW(LPCWSTR str) : CPtrArray()
{
	lstrcpynW(string, str, MAX_PATH);

	// put the tokens into a pointer array
	WCHAR *ptr = string;
	do
	{
		Add(ptr);
		ptr = wcsstr(ptr, L" - ");
		if (ptr)
		{
			*ptr = 0;
			ptr += 3;
		}
	}
	while (ptr);
}

CNamingTokensW::~CNamingTokensW()
{
}

LPCWSTR CNamingTokensW::operator[](int nIndex)
{
	return (LPCWSTR)GetAt(nIndex);
}

void CNamingTokensW::Parse(LPCWSTR str)
{
	RemoveAll();

	lstrcpynW(string, str, MAX_PATH);

	// put the tokens into a pointer array
	WCHAR *ptr = string;
	do
	{
		Add(ptr);
		ptr = wcsstr(ptr, L" - ");
		if (ptr)
		{
			*ptr = 0;
			ptr += 3;
		}
	}
	while (ptr);
}

//////////////////////////////////////////////////////////////////
// CTextFile class

CTextFile::CTextFile(LPCTSTR lpszFilename, UINT nOpenFlags)
{
	m_pStream = NULL;

	m_hFile = CreateFile(lpszFilename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	{
	}
}

CTextFile::~CTextFile()
{
}

void CTextFile::WriteString(LPCTSTR lpsz)
{
}

LPTSTR CTextFile::ReadString(LPTSTR lpsz, UINT nMax)
{
	return lpsz;
}

UINT CTextFile::Read(void* lpBuf, UINT nCount)
{
	return 0;
}

void CTextFile::Write(const void* lpBuf, UINT nCount)
{
}

void CTextFile::Flush()
{
}

void CTextFile::Close()
{
	if (m_pStream)
		fclose(m_pStream);

	m_pStream = NULL;
	m_hFile	= NULL;
}

//////////////////////////////////////////////////////////////////
// CTemplateFile class

CTemplateFile::CTemplateFile()
{
}

CTemplateFile::~CTemplateFile()
{
	m_ReplaceMap.RemoveAll();
}

void CTemplateFile::AddReplace(LPCTSTR lpsz1, LPCTSTR lpsz2)
{
	if (lpsz1 == NULL) return;
	if (lpsz2 == NULL) return;

	m_ReplaceMap.SetAt(lpsz1, lpsz2);
}

void CTemplateFile::AddReplace(LPCTSTR lpsz1, int index, LPCTSTR lpsz2)
{
	if (lpsz1 == NULL) return;
	if (lpsz2 == NULL) return;

	char str[MAX_PATH];
	wsprintf(str, "%s%ld", lpsz1, index);

	m_ReplaceMap.SetAt(str, lpsz2);
}

void CTemplateFile::AddReplace(LPCTSTR lpsz, int nNum)
{
	if (lpsz == NULL) return;

	char str[64] = "";
	itoa(nNum, str, 10);

	m_ReplaceMap.SetAt(lpsz, str);
}

void CTemplateFile::AddReplace(LPCTSTR lpsz, int index, int nNum)
{
	if (lpsz == NULL) return;

	char str[64] = "";
	itoa(nNum, str, 10);

	char str2[MAX_PATH];
	wsprintf(str2, "%s%ld", lpsz, index);

	m_ReplaceMap.SetAt(str2, str);
}

void CTemplateFile::AddReplace(LPCTSTR lpsz, COLORREF cr)
{
	if (lpsz == NULL) return;

	char str[MAX_PATH];
	wsprintf(str, "#%02x%02x%02x", GetRValue(cr), GetGValue(cr), GetBValue(cr));

	m_ReplaceMap.SetAt(lpsz, str);
}

BOOL CTemplateFile::Convert(LPCTSTR lpszOutput, LPCTSTR lpszInput)
{
	HANDLE hIn = NULL;
	HANDLE hOut = NULL;

	char tempstr[8] = "";

	char *buffer = NULL;
	char *matchstr = NULL;

	ENSURE
	{
		hIn = CreateFile(lpszInput, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hIn == INVALID_HANDLE_VALUE) FAIL;

		hOut = CreateFile(lpszOutput, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (hOut == INVALID_HANDLE_VALUE) FAIL;

		DWORD dwFileSize = GetFileSize(hIn, NULL);

		buffer = new char [dwFileSize];
		if (buffer == NULL) FAIL;

		matchstr = new char [256];
		if (matchstr == NULL) FAIL;

		DWORD nBytesRead = 0;
		DWORD nBytesWritten = 0;

		BOOL bResult = ReadFile(hIn, buffer, dwFileSize, &nBytesRead, NULL);
		if (!bResult) FAIL;

		int nLastWrote = 0;
		BOOL bInList = FALSE;
		int nListStartIndex = 0;
		int nLoopIndex = 0;
		BOOL bMatchFoundInList = FALSE;
		DWORD dwCurPos = 0;
		for (int i=0; i<(int)nBytesRead; i++)
		{
			// find the tag "<%"
			if ((buffer[i] == '<') &&
				(buffer[i+1] == '%'))
			{
				// write out whatever we have first
				if (i != nLastWrote)
				{
					WriteFile(hOut, &buffer[nLastWrote], i-nLastWrote, &nBytesWritten, NULL);
					nLastWrote = i;
				}

				// find the ending "%>
				for (int j=i+2; j<(int)nBytesRead; j++)
				{
					if ((buffer[j] == '%') &&
						(buffer[j+1] == '>'))
					{
						if (memcmp("liststart", &buffer[i+2], min(9, j-(i+2))) == 0)
						{
							i = j+2;
							nLastWrote = i;
							bInList = TRUE;
							nListStartIndex = i;
							nLoopIndex = 0;
							bMatchFoundInList = FALSE;
							dwCurPos = SetFilePointer(hOut, 0, NULL, FILE_CURRENT);
							break;
						}
						else if (memcmp("listend", &buffer[i+2], min(7, j-(i+2))) == 0)
						{
							if (bInList)
							{
								if (bMatchFoundInList)
								{
									i = nListStartIndex;
									nLastWrote = i;
									nLoopIndex++;
									bMatchFoundInList = FALSE;
									dwCurPos = SetFilePointer(hOut, 0, NULL, FILE_CURRENT);
								}
								else
								{
									i = j+2;
									nLastWrote = i;
									bInList = FALSE;
									SetFilePointer(hOut, dwCurPos, NULL, FILE_BEGIN);
									SetEndOfFile(hOut);
								}
								break;
							}
						}

						int matchstrlen = j-(i+2);
						lstrcpyn(matchstr, &buffer[i+2], matchstrlen+1);

						if (bInList)
						{
							lstrcat(matchstr, itoa(nLoopIndex, tempstr, 10));
							matchstrlen = lstrlen(matchstr);
						}

						// match the replacement string then
						char *replacement = NULL;
						if (m_ReplaceMap.Lookup(matchstr, replacement))
						{
							WriteFile(hOut, replacement, lstrlen(replacement), &nBytesWritten, NULL);

							// update i and stuff
							i = j+2;
							nLastWrote = i;
							bMatchFoundInList = TRUE;
						}
						else
						{
							i = j+2;
						}
						break;
					}
				}
			}
		}

		// write the remaining stuff
		if (nLastWrote != (int)nBytesRead)
		{
			WriteFile(hOut, &buffer[nLastWrote], nBytesRead-nLastWrote, &nBytesWritten, NULL);
		}

		// flush the buffer to disk first
		FlushFileBuffers(hOut);

		// close handles
		CloseHandle(hIn);
		CloseHandle(hOut);

		// clean up
		if (buffer) delete [] buffer;
		if (matchstr) delete [] matchstr;

		return TRUE;
	}
	END_ENSURE;

	if (hIn) CloseHandle(hIn);
	if (hOut) CloseHandle(hOut);
	if (buffer) delete [] buffer;
	if (matchstr) delete [] matchstr;

	return FALSE;
}

//////////////////////////////////////////////////////////////////
// CMapStringToString class

CMapStringToString::CMapStringToString()
{
	m_pHashTable	 = NULL;
	m_nHashTableSize = 64;
	m_nCount		 = 0;
}

CMapStringToString::~CMapStringToString()
{
	RemoveAll();
}

int	CMapStringToString::GetCount() const
{
	return m_nCount;
}

BOOL CMapStringToString::IsEmpty() const
{
	return m_nCount == 0;
}

void CMapStringToString::InitHashTable(UINT nHashSize, BOOL bAllocNow)
{
	if (m_pHashTable != NULL)
	{
		// free hash table
		delete[] m_pHashTable;
		m_pHashTable = NULL;
	}

	if (bAllocNow)
	{
		m_pHashTable = new CAssoc* [nHashSize];
		memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
	}
	m_nHashTableSize = nHashSize;
}

void CMapStringToString::SetAt(LPCTSTR key, LPCTSTR newValue)
{
	UINT nHash;
	CAssoc* pAssoc;
	if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
	{
		if (m_pHashTable == NULL)
			InitHashTable(m_nHashTableSize);

		// it doesn't exist, add a new Association
		pAssoc = new CAssoc(key, newValue);

		// put into hash table
		pAssoc->pNext = m_pHashTable[nHash];
		m_pHashTable[nHash] = pAssoc;

		m_nCount++;
	}
	else
	{
		if (pAssoc->value) delete [] pAssoc->value;
		pAssoc->value = new char [lstrlen(newValue) + 1];
		lstrcpy(pAssoc->value, newValue);
	}
}

BOOL CMapStringToString::Lookup(LPCTSTR key, LPTSTR &rValue) const
{
	UINT nHash;
	CAssoc* pAssoc = GetAssocAt(key, nHash);
	if (pAssoc == NULL)
		return FALSE;  // not in map

	rValue = pAssoc->value;
	return TRUE;
}

BOOL CMapStringToString::RemoveKey(LPCTSTR key)
{
	if (m_pHashTable == NULL)
		return FALSE;  // nothing in the table

	CAssoc** ppAssocPrev;
	ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

	CAssoc* pAssoc;
	for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
		{
			// remove it
			*ppAssocPrev = pAssoc->pNext;  // remove from list
			delete pAssoc;
			m_nCount--;
			return TRUE;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
	return FALSE;  // not found
}

void CMapStringToString::RemoveAll()
{
	if (m_pHashTable != NULL)
	{
		// destroy elements
		for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
		{
			CAssoc* pAssoc = m_pHashTable[nHash];
			while (pAssoc)
			{
				CAssoc*old = pAssoc;
				pAssoc = pAssoc->pNext;
				delete old;
			}
		}

		// free hash table
		delete [] m_pHashTable;
		m_pHashTable = NULL;
	}

	m_nCount = 0;
}

UINT CMapStringToString::HashKey(LPCTSTR key) const
{
	UINT nHash = 0;
	while (*key)
		nHash = (nHash<<5) + nHash + *key++;
	return nHash;
}

POSITION CMapStringToString::GetStartPosition() const
{
	return (m_nCount == 0) ? NULL : BEFORE_START_POSITION;
}

void CMapStringToString::GetNextAssoc(POSITION& rNextPosition, LPTSTR &rKey, LPTSTR &rValue) const
{
	CAssoc* pAssocRet = (CAssoc*)rNextPosition;

	if (pAssocRet == (CAssoc*) BEFORE_START_POSITION)
	{
		// find the first association
		for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
				break;
	}

	// find next association
	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		int nHashValue = HashKey(pAssocRet->key) % m_nHashTableSize;
		// go to next bucket
		for (UINT nBucket = nHashValue + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	rNextPosition = (POSITION) pAssocNext;

	// fill in return data
	rKey = pAssocRet->key;
	rValue = pAssocRet->value;
}

void CMapStringToString::Dump()
{
	DbgPrint("CMapStringToString::Dump()\n");

	POSITION pos = GetStartPosition();
	while (pos)
	{
		char *key, *value;
		GetNextAssoc(pos, key, value);

		DbgPrint("  %s <--> %s\n", key, value);
	}

	DbgPrint("--------------------------\n");
}

CMapStringToString::CAssoc::CAssoc(LPCTSTR k, LPCTSTR v)
{
	pNext = NULL;

	key = new char [lstrlen(k)+1];
	value = new char [lstrlen(v)+1];

	lstrcpy(key, k);
	lstrcpy(value, v);
}

CMapStringToString::CAssoc::~CAssoc()
{
	if (key) delete [] key;
	if (value) delete [] value;
}

CMapStringToString::CAssoc* CMapStringToString::GetAssocAt(LPCTSTR key, UINT &nHash) const
{
	nHash = HashKey(key) % m_nHashTableSize;

	if (m_pHashTable == NULL)
		return NULL;

	// see if it exists
	CAssoc* pAssoc;
	for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (lstrcmp(pAssoc->key, key) == 0)
			return pAssoc;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////
// CMapStringToStringW class

CMapStringToStringW::CMapStringToStringW()
{
	m_pHashTable	 = NULL;
	m_nHashTableSize = 64;
	m_nCount		 = 0;
}

CMapStringToStringW::~CMapStringToStringW()
{
	RemoveAll();
}

int	CMapStringToStringW::GetCount() const
{
	return m_nCount;
}

BOOL CMapStringToStringW::IsEmpty() const
{
	return m_nCount == 0;
}

void CMapStringToStringW::InitHashTable(UINT nHashSize, BOOL bAllocNow)
{
	if (m_pHashTable != NULL)
	{
		// free hash table
		delete[] m_pHashTable;
		m_pHashTable = NULL;
	}

	if (bAllocNow)
	{
		m_pHashTable = new CAssoc* [nHashSize];
		memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
	}
	m_nHashTableSize = nHashSize;
}

void CMapStringToStringW::SetAt(LPCWSTR key, LPCWSTR newValue)
{
	UINT nHash;
	CAssoc* pAssoc;
	if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
	{
		if (m_pHashTable == NULL)
			InitHashTable(m_nHashTableSize);

		// it doesn't exist, add a new Association
		pAssoc = new CAssoc(key, newValue);

		// put into hash table
		pAssoc->pNext = m_pHashTable[nHash];
		m_pHashTable[nHash] = pAssoc;

		m_nCount++;
	}
	else
	{
		if (pAssoc->value) delete [] pAssoc->value;
		pAssoc->value = new WCHAR [lstrlenW(newValue) + 1];
		lstrcpyW(pAssoc->value, newValue);
	}
}

BOOL CMapStringToStringW::Lookup(LPCWSTR key, LPWSTR &rValue) const
{
	UINT nHash;
	CAssoc* pAssoc = GetAssocAt(key, nHash);
	if (pAssoc == NULL)
		return FALSE;  // not in map

	rValue = pAssoc->value;
	return TRUE;
}

BOOL CMapStringToStringW::RemoveKey(LPCWSTR key)
{
	if (m_pHashTable == NULL)
		return FALSE;  // nothing in the table

	CAssoc** ppAssocPrev;
	ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

	CAssoc* pAssoc;
	for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
		{
			// remove it
			*ppAssocPrev = pAssoc->pNext;  // remove from list
			delete pAssoc;
			m_nCount--;
			return TRUE;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
	return FALSE;  // not found
}

void CMapStringToStringW::RemoveAll()
{
	if (m_pHashTable != NULL)
	{
		// destroy elements
		for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
		{
			CAssoc* pAssoc = m_pHashTable[nHash];
			while (pAssoc)
			{
				CAssoc*old = pAssoc;
				pAssoc = pAssoc->pNext;
				delete old;
			}
		}

		// free hash table
		delete [] m_pHashTable;
		m_pHashTable = NULL;
	}

	m_nCount = 0;
}

UINT CMapStringToStringW::HashKey(LPCWSTR key) const
{
	UINT nHash = 0;
	while (*key)
		nHash = (nHash<<5) + nHash + *key++;
	return nHash;
}

POSITION CMapStringToStringW::GetStartPosition() const
{
	return (m_nCount == 0) ? NULL : BEFORE_START_POSITION;
}

void CMapStringToStringW::GetNextAssoc(POSITION& rNextPosition, LPWSTR &rKey, LPWSTR &rValue) const
{
	CAssoc* pAssocRet = (CAssoc*)rNextPosition;

	if (pAssocRet == (CAssoc*) BEFORE_START_POSITION)
	{
		// find the first association
		for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
				break;
	}

	// find next association
	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		int nHashValue = HashKey(pAssocRet->key) % m_nHashTableSize;
		// go to next bucket
		for (UINT nBucket = nHashValue + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	rNextPosition = (POSITION) pAssocNext;

	// fill in return data
	rKey = pAssocRet->key;
	rValue = pAssocRet->value;
}

void CMapStringToStringW::Dump()
{
	DbgPrint("CMapStringToStringW::Dump()\n");

	POSITION pos = GetStartPosition();
	while (pos)
	{
		WCHAR *key, *value;
		GetNextAssoc(pos, key, value);

		DbgPrint("  %S <--> %S\n", key, value);
	}

	DbgPrint("--------------------------\n");
}

CMapStringToStringW::CAssoc::CAssoc(LPCWSTR k, LPCWSTR v)
{
	pNext = NULL;

	key = new WCHAR [lstrlenW(k)+1];
	value = new WCHAR [lstrlenW(v)+1];

	lstrcpyW(key, k);
	lstrcpyW(value, v);
}

CMapStringToStringW::CAssoc::~CAssoc()
{
	if (key) delete [] key;
	if (value) delete [] value;
}

CMapStringToStringW::CAssoc* CMapStringToStringW::GetAssocAt(LPCWSTR key, UINT &nHash) const
{
	nHash = HashKey(key) % m_nHashTableSize;

	if (m_pHashTable == NULL)
		return NULL;

	// see if it exists
	CAssoc* pAssoc;
	for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (lstrcmpW(pAssoc->key, key) == 0)
			return pAssoc;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////
// utility functions

void GetVersionInformation(HINSTANCE hInstance, int &major, int &minor, int &build)
{
	// Get version information
	char				szFullPath[260];
	DWORD				dwVerHnd=0;			// An 'ignored' parameter, always '0'
	VS_FIXEDFILEINFO*	lpVersion;			// Pointer to 'Fixed' file info
	UINT				uVersionLen;

	GetModuleFileName(hInstance, szFullPath, sizeof(szFullPath));
	DWORD dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	if (dwVerInfoSize)
	{
		LPSTR   lpstrVffInfo;
		HANDLE  hMem;

		// allocate memory for version block
		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo  = (LPSTR)GlobalLock(hMem);
		GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
		BOOL bRetCode = VerQueryValue(	lpstrVffInfo, "\\", (LPVOID *)&lpVersion, &uVersionLen);
		if (bRetCode)
		{
			// access version information
			major = HIWORD(lpVersion->dwProductVersionMS);
			minor = LOWORD(lpVersion->dwProductVersionMS);
			build = LOWORD(lpVersion->dwFileVersionLS);
		}

		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}
}

LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) {
        long datasize = MAX_PATH;
		TCHAR data[MAX_PATH];
		RegQueryValue(hkey, NULL, data, &datasize);
		lstrcpy(retdata,data);
		RegCloseKey(hkey);
    }

    return retval;
}

HINSTANCE GotoURL(LPCTSTR url, int showcmd)
{
    TCHAR key[MAX_PATH + MAX_PATH];	

    // First try ShellExecute()
    HINSTANCE result = ShellExecute(NULL, "open", url, NULL,NULL, showcmd);

    // If it failed, get the .htm regkey and lookup the program
    if ((UINT)result <= HINSTANCE_ERROR) {		
		
        if (GetRegKey(HKEY_CLASSES_ROOT, ".htm", key) == ERROR_SUCCESS) {
            lstrcat(key, "\\shell\\open\\command");

            if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
                TCHAR *pos;
				// 2005-01-23 Modified T-Matsuo
//              pos = strstr(key, "\"%1\"");
                pos = (char *)_mbsstr((const unsigned char *)key, (const unsigned char *)"\"%1\"");
                if (pos == NULL) {                     // No quotes found
					// 2005-01-23 Modified T-Matsuo
//                  pos = strstr(key, "%1");       // Check for %1, without quotes
                    pos = (char *)_mbsstr((const unsigned char *)key, (const unsigned char *)"%1");       // Check for %1, without quotes
                    if (pos == NULL)                   // No parameter at all...
                        pos = key+lstrlen(key)-1;
                    else
                        *pos = '\0';                   // Remove the parameter
                }
                else
                    *pos = '\0';                       // Remove the parameter

                lstrcat(pos, " ");
                lstrcat(pos, url);
                result = (HINSTANCE) WinExec(key,showcmd);
            }
        }
	}
	  
    return result;
}

BOOL Win98GetFileAttributesEx(LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
	static BOOL (WINAPI *pGetFileAttributesEx)(LPCTSTR, GET_FILEEX_INFO_LEVELS, LPVOID) = NULL;

	if (pGetFileAttributesEx == NULL)
	{
		HMODULE hModule = GetModuleHandle("KERNEL32.DLL");
		if (hModule)
		{
			pGetFileAttributesEx = (BOOL(WINAPI *)(LPCTSTR, GET_FILEEX_INFO_LEVELS, LPVOID)) GetProcAddress(hModule, "GetFileAttributesExA");
		}
	}

	return pGetFileAttributesEx ? pGetFileAttributesEx(lpFileName, fInfoLevelId, lpFileInformation) : FALSE;
}

BOOL Win98GetFileAttributesEx(LPCWSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
	static BOOL (WINAPI *pGetFileAttributesEx)(LPCWSTR, GET_FILEEX_INFO_LEVELS, LPVOID) = NULL;

	if (pGetFileAttributesEx == NULL)
	{
		HMODULE hModule = GetModuleHandle("KERNEL32.DLL");
		if (hModule)
		{
			pGetFileAttributesEx = (BOOL(WINAPI *)(LPCWSTR, GET_FILEEX_INFO_LEVELS, LPVOID)) GetProcAddress(hModule, "GetFileAttributesExW");
		}
	}

	return pGetFileAttributesEx ? pGetFileAttributesEx(lpFileName, fInfoLevelId, lpFileInformation) : FALSE;
}

HRESULT STDAPICALLTYPE WinXPEnableThemeDialogTexture(HWND hwnd, DWORD dwFlags)
{
	static HRESULT (STDAPICALLTYPE *pEnableThemeDialogTexture)(HWND, DWORD) = NULL;

	// get theme proc
	if (pEnableThemeDialogTexture == NULL)
	{
		HMODULE hModule = GetModuleHandle("UxTheme.dll");
		if (hModule)
		{
			pEnableThemeDialogTexture = (HRESULT (STDAPICALLTYPE *)(HWND, DWORD))GetProcAddress(hModule, "EnableThemeDialogTexture");
		}
	}

	return (pEnableThemeDialogTexture != NULL) ? pEnableThemeDialogTexture(hwnd, dwFlags) : E_FAIL;
}

LPCTSTR ALS(LPCTSTR str1, LPCTSTR str2 /*=NULL*/)
{
// crc function prototype
DWORD CRC32(LPCTSTR data_blk_ptr, int data_blk_size);

	// str2 is used for those shortcuts like "Refresh\tCtrl+R"

	static char szResult1[MAX_PATH*2] = "";
	static char szResult2[MAX_PATH*2] = "";

	char crc1[16], crc2[16];
	if (str1) wsprintf(crc1, "%08x", CRC32(str1, lstrlen(str1)));
	if (str2) wsprintf(crc2, "%08x", CRC32(str2, lstrlen(str2)));

#ifdef _DEBUG
	// a good way to get all the strings we need :)
	if (str1 && str1[0] != 0) WritePrivateProfileString("Strings", crc1, str1, "c:\\sample.al");
	if (str2 && str2[0] != 0) WritePrivateProfileString("Strings", crc2, str2, "c:\\sample.al");
#endif

	// no language or english (default) is selected?
	LPCTSTR file = wndWinampAL.GetSetting(settingTranslation);
	if (lstrlen(file) == 0)
	{
		if (str2 == NULL) return str1;
		
		lstrcpy(szResult1, str1);
		if (lstrlen(str2))
		{
			lstrcat(szResult1, "\t");
			lstrcat(szResult1, str2);
		}
		return szResult1;
	}

	// get the string from the language file (*.al)
	if (GetPrivateProfileString("Strings", crc1, "", szResult1, MAX_PATH, file) == 0)
	{
		GetPrivateProfileString("Strings", str1, str1, szResult1, MAX_PATH, file);
	}

	if (str2 && lstrlen(str2))
	{
		if (GetPrivateProfileString("Strings", crc2, "", szResult2, MAX_PATH, file) == 0)
		{
			GetPrivateProfileString("Strings", str2, str2, szResult2, MAX_PATH, file);
		}
		lstrcat(szResult1, "\t");
		lstrcat(szResult1, szResult2);
	}

	return szResult1;
}

LPCWSTR ALSW(LPCTSTR str1, LPCTSTR str2 /*= NULL*/)
{
	static WCHAR wResult[MAX_PATH];
	lstrcpynW(wResult, AutoWide(ALS(str1, str2), wndWinampAL.GetCodepage()), MAX_PATH);
	return wResult;
}

LPCTSTR FormatTime(LPTSTR str, int nTime)
{
	int hour = nTime/3600;
	int min = (nTime%3600)/60;
	int sec = nTime%60;
	if (hour)
		wsprintf(str, "%ld:%02ld:%02ld", hour, min, sec);
	else
		wsprintf(str, "%ld:%02ld", min, sec);

	return str;
}

void DbgPrint(LPSTR format, ...)
{
#ifdef _DEBUG
	va_list		args;
	va_start	(args, format);

	char str[1024];
	_vsnprintf(str, 1024, format, args);
	OutputDebugString(str);
#endif
}

//
// Usage: SetThreadName (-1, "MainThread");
//
typedef struct tagTHREADNAME_INFO
{
  DWORD dwType; // must be 0x1000
  LPCSTR szName; // pointer to name (in user addr space)
  DWORD dwThreadID; // thread ID (-1=caller thread)
  DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
{
#ifdef _DEBUG
  THREADNAME_INFO info;
  {
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
  }
  __try
  {
    RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
  }
  __except (EXCEPTION_CONTINUE_EXECUTION)
  {
  }
#endif
}

LPCTSTR lstrstri(LPCTSTR str, LPCTSTR substr)
{
	int len = lstrlen(str);
	int sublen = lstrlen(substr);

	if ((len == 0) || (sublen == 0))
		return NULL;

	int count = len - sublen;

	for (int i = 0; i <= count; i++)
	{
		if (_strnicmp(str+i, substr, sublen) == 0)
			return str+i;
	}

	return NULL;
}

////////////////////////////////////////////////////

HBITMAP LoadPictureFile(LPCTSTR szFile, SIZE size)
{
	return LoadPictureFile(AutoWide(szFile), size);
}

HBITMAP LoadPictureFile(LPCWSTR szFile, SIZE size)
{
	HANDLE		hFile		= NULL;
	HGLOBAL		hGlobal		= NULL;
	LPVOID		pvData		= NULL;
	LPSTREAM	pstm		= NULL;
	LPPICTURE	gpPicture	= NULL;

#ifdef _DEBUG
//	static DWORD dwTotal = 0;
//	static DWORD dwCount = 0;
//	DWORD dwTickCount = GetTickCount();
#endif

	ENSURE
	{
		// open file
		if ((hFile = CreateFileW(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
			FAIL;

		// get file size
		DWORD dwFileSize = GetFileSize(hFile, NULL);

		// alloc memory based on file size
		if ((hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize)) == NULL)
			FAIL;

		if ((pvData = GlobalLock(hGlobal)) == NULL)
			FAIL;

		DWORD dwBytesRead = 0;
		// read file and store in global memory
		BOOL bRead = ReadFile(hFile, pvData, dwFileSize, &dwBytesRead, NULL);

		GlobalUnlock(hGlobal);
		CloseHandle(hFile);		hFile = NULL;

		// create IStream* from global memory
		if (FAILED(CreateStreamOnHGlobal(hGlobal, TRUE, &pstm)))
			FAIL;

		// Create IPicture from image file

		if (FAILED(OleLoadPicture(pstm, dwFileSize, FALSE, IID_IPicture, (LPVOID *)&gpPicture)))
			FAIL;

		pstm->Release();
		pstm = NULL;

		OLE_HANDLE han;
		gpPicture->get_Handle(&han);

		BITMAP bm;
		GetObject((HBITMAP)han, sizeof(bm), &bm);

		// create offscreen surface
		HDC			hDC		= GetDC(GetDesktopWindow());
		HDC			hMem1	= CreateCompatibleDC(hDC);;
		HDC			hMem2	= CreateCompatibleDC(hDC);;
		HBITMAP		hBitmap	= CreateCompatibleBitmap(hDC, size.cx, size.cy);
		HBITMAP		oBitmap1= (HBITMAP)SelectObject(hMem1, hBitmap);
		HBITMAP		oBitmap2= (HBITMAP)SelectObject(hMem2, (HBITMAP)han);
		int			oBltMode= SetStretchBltMode(hMem1, bWinNT ? HALFTONE : COLORONCOLOR);

#if 0
		double ratioSrc = (double)bm.bmWidth / (double)bm.bmHeight;
		double ratioDst = (double)size.cx / (double)size.cy;

		int width = size.cx;
		int height = size.cy;

		if (ratioSrc > ratioDst)	height = size.cx / ratioSrc;
		else						width = size.cy * ratioSrc;

		int left = (size.cx - width) / 2;
		int top  = (size.cy - height) / 2;

		StretchBlt(hMem1, left, top, width, height, hMem2, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
#else
		StretchBlt(hMem1, 0, 0, size.cx, size.cy, hMem2, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
#endif

		// cleanup
		SetStretchBltMode(hMem1, oBltMode);
		SelectObject(hMem1, oBitmap1);
		SelectObject(hMem2, oBitmap2);
		DeleteDC(hMem1);
		DeleteDC(hMem2);
		ReleaseDC(GetDesktopWindow(), hDC);

		gpPicture->Release();
		gpPicture = NULL;

		GlobalFree(hGlobal);

#ifdef _DEBUG
//		dwTotal += GetTickCount() - dwTickCount;
//		dwCount ++;
//		DbgPrint("LoadPictureFile time = %ld\n", dwTotal/dwCount);
#endif

		return hBitmap;
	}
	END_ENSURE;

	if (hGlobal)	GlobalFree(hGlobal);
	if (hFile)		CloseHandle(hFile);
	if (pstm)		pstm->Release();
	if (gpPicture) 	gpPicture->Release();

	return NULL;
}

HBITMAP LoadPictureFile2(LPCTSTR szFile, SIZE size)
{
	return LoadPictureFile2(AutoWide(szFile), size);
}

HBITMAP LoadPictureFile2(LPCWSTR szFile, SIZE size)
{
	IImgCtx*	pImgCtx		= NULL;

#ifdef _DEBUG
	static DWORD dwTotal = 0;
	static DWORD dwCount = 0;
	DWORD dwTickCount = GetTickCount();
#endif

	ENSURE
	{
		struct _stat buf;
		if (_wstat(szFile, &buf) != 0)
			FAIL;

		// convert filename to url
		wchar_t szPath[MAX_PATH] = L"file://";
		wcscat(szPath, szFile);
		for (wchar_t *p = szPath; *p; p++)
		{
			if (*p == L'\\') *p = L'/';
		}

		if (FAILED(CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER, IID_IImgCtx, (LPVOID*)&pImgCtx)))
			FAIL;

		if (FAILED(pImgCtx->Load(szPath, 0)))
			FAIL;

		ULONG ulState;
		SIZE sizeImage;
		do
		{
			pImgCtx->GetStateInfo(&ulState, &sizeImage, FALSE);
 
			// This image should load/decode instantly, 
			// but everyone deserves some sleep
			Sleep(5);
		}
		while (ulState & IMGLOAD_LOADING);

		// create offscreen surface
		HDC			hDC		= GetDC(GetDesktopWindow());
		HDC			hMem1	= CreateCompatibleDC(hDC);;
		HBITMAP		hBitmap	= CreateCompatibleBitmap(hDC, size.cx, size.cy);
		HBITMAP		oBitmap1= (HBITMAP)SelectObject(hMem1, hBitmap);
		int			oBltMode= SetStretchBltMode(hMem1, bWinNT ? HALFTONE : COLORONCOLOR);

#if 0
		double ratioSrc = (double)bm.bmWidth / (double)bm.bmHeight;
		double ratioDst = (double)size.cx / (double)size.cy;

		int width = size.cx;
		int height = size.cy;

		if (ratioSrc > ratioDst)	height = size.cx / ratioSrc;
		else						width = size.cy * ratioSrc;

		int left = (size.cx - width) / 2;
		int top  = (size.cy - height) / 2;

	    pImgCtx->StretchBlt(hMem1, left, top, width, height, 0, 0, sizeImage.cx, sizeImage.cy, SRCCOPY);
#else
	    pImgCtx->StretchBlt(hMem1, 0, 0, size.cx, size.cy, 0, 0, sizeImage.cx, sizeImage.cy, SRCCOPY);
#endif

		// cleanup
		SetStretchBltMode(hMem1, oBltMode);
		SelectObject(hMem1, oBitmap1);
		DeleteDC(hMem1);
		ReleaseDC(GetDesktopWindow(), hDC);

		pImgCtx->Release();

#ifdef _DEBUG
		dwTotal += GetTickCount() - dwTickCount;
		dwCount ++;
		DbgPrint("LoadPictureFile2 time = %ld\n", dwTotal/dwCount);
#endif

		return hBitmap;
	}
	END_ENSURE;

	if (pImgCtx)	pImgCtx->Release();

	return NULL;
}

////////////////////////////////////////////////////

BOOL WriteTextToFile(HANDLE hFile, LPCTSTR str, BOOL bUTF8, int cp)
{
	DWORD nBytesWritten;

	int len = lstrlen(str);

	BOOL bRet;

	if (bUTF8)
	{
		wchar_t *w = new wchar_t [len*2];
		char	*u = new char [len*2];

		MultiByteToWideChar(cp, 0, str, len+1, w, len*2);
		WideCharToMultiByte(CP_UTF8, 0, w, wcslen(w)+1, u, len*2, NULL, NULL);
		bRet = WriteFile(hFile, u, strlen(u), &nBytesWritten, NULL);

		delete [] w;
		delete [] u;
	}
	else
	{
		bRet = WriteFile(hFile, str, len, &nBytesWritten, NULL);
	}

	return bRet;
}

BOOL WriteTextToFile(HANDLE hFile, LPCWSTR str, BOOL bUTF8, int cp)
{
	DWORD nBytesWritten;

	int len = wcslen(str);

	BOOL bRet;
	if (bUTF8)
	{
		char	*u = new char [len*2];
		WideCharToMultiByte(CP_UTF8, 0, str, len+1, u, len*2, NULL, NULL);
		bRet = WriteFile(hFile, u, strlen(u), &nBytesWritten, NULL);
		delete [] u;
	}
	else
	{
		char	*u = new char [len*2];
		WideCharToMultiByte(cp, 0, str, len+1, u, len*2, NULL, NULL);
		bRet = WriteFile(hFile, str, len, &nBytesWritten, NULL);
		delete [] u;
	}

	return bRet;
}

BOOL WritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCWSTR lpString, LPCTSTR lpFileName)
{
	AutoChar str(lpString, CP_UTF8);

	return WritePrivateProfileString(lpAppName, lpKeyName, str, lpFileName);
}

DWORD GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	char str[MAX_PATH*2];
	AutoChar def(lpDefault, CP_UTF8);

	DWORD dwRet = GetPrivateProfileString(lpAppName, lpKeyName, def, str, MAX_PATH*2, lpFileName);

	if (lpReturnedString) wcsncpy(lpReturnedString, AutoWide(str, CP_UTF8), nSize);

	return dwRet;
}

BOOL BrowseForFolder(HWND hWnd, LPCWSTR title, LPWSTR path, int nSize)
{
	BROWSEINFOW bi;
	memset(&bi, 0, sizeof(bi));

	wchar_t szBuffer[MAX_PATH] = L"";

	bi.hwndOwner = hWnd;
	bi.lpfn = NULL;
	bi.lParam = NULL;
	bi.lpszTitle = title;
	bi.pszDisplayName = szBuffer;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;

	BOOL bRet = FALSE;

	LPITEMIDLIST pidl;
	if ((pidl = ::SHBrowseForFolderW(&bi)) != NULL)
	{
		if (SUCCEEDED(::SHGetPathFromIDListW(pidl, szBuffer)))
		{
			bRet = TRUE;
			wcsncpy(path, szBuffer, nSize);
		}

		LPMALLOC pMalloc;
		//Retrieve a pointer to the shell's IMalloc interface
		if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			// free the PIDL that SHBrowseForFolder returned to us.
			pMalloc->Free(pidl);
			// release the shell's IMalloc interface
			(void)pMalloc->Release();
		}
	}

	return bRet;
}

int CompareStr(LPCWSTR str1, LPCWSTR str2)
{
	int nRet = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, str1, -1, str2, -1);

	if (nRet == CSTR_LESS_THAN) return -1;
	if (nRet == CSTR_EQUAL) return 0;
	if (nRet == CSTR_GREATER_THAN) return 1;

	return 0;
}
