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
#include <commdlg.h>
#include <dlgs.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "ConfigCover.h"
#include "VisualStylesXP.h"
#include "AutoWide.h"

#define COLOR_DIM		128

//////////////////////////////////////////////////////////////////
// CConfigCover class

CConfigCover::CConfigCover()
{
	m_nIDTemplate = IDD_CONFIGCOVER;

	m_nCoverWidth		= 100;
	m_nCoverHeight		= 100;
	m_nBorderWidth		= 3;
	m_clrCoverBorder	= RGB(0, 255, 0);
	m_clrCoverText		= RGB(255, 255, 255);
	m_bCustomBorderColor= TRUE;
	m_bCoverShadow		= FALSE;
	m_bSearchAltFolder	= FALSE;
	m_bSearchFolder		= TRUE;
	m_bOverrideDefCover	= FALSE;
	m_bSearchMP3		= TRUE;
	m_bDrawTitle		= TRUE;
	m_bCacheCovers		= FALSE;
	memset(m_szDefaultCover, 0, sizeof(m_szDefaultCover));
	memset(m_szAltFolder, 0, sizeof(m_szAltFolder));
	memset(m_szCoverSearchExt, 0, sizeof(m_szCoverSearchExt));
}

CConfigCover::~CConfigCover()
{
}

BOOL CConfigCover::OnInitDialog()
{
	// set range for buddy controls
	SendDlgItemMessage(IDC_COVER_WIDTH_SPIN,  UDM_SETRANGE32, 16, 400);
	SendDlgItemMessage(IDC_COVER_HEIGHT_SPIN, UDM_SETRANGE32, 16, 400);
	SendDlgItemMessage(IDC_BORDER_WIDTH_SPIN, UDM_SETRANGE32, 3, max(3, min(m_nCoverWidth, m_nCoverHeight) / 10));

	// set current values
	SendDlgItemMessage(IDC_COVER_WIDTH_SPIN,  UDM_SETPOS, 0, MAKELPARAM(m_nCoverWidth, 0));
	SendDlgItemMessage(IDC_COVER_HEIGHT_SPIN, UDM_SETPOS, 0, MAKELPARAM(m_nCoverHeight, 0));
	SendDlgItemMessage(IDC_BORDER_WIDTH_SPIN, UDM_SETPOS, 0, MAKELPARAM(m_nBorderWidth, 0));

	// update border color
	UpdateColor(IDC_COLORCHOOSER, m_clrCoverBorder);

	// search mp3
	CheckDlgButton(IDC_INSIDE_MP3, m_bSearchMP3 ? BST_CHECKED : BST_UNCHECKED);

	// override default cover
	CheckDlgButton(IDC_OVERRIDE_DEF_COVER, m_bOverrideDefCover ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_DEFAULT_COVER).SetWindowText(m_szDefaultCover);
	UpdateColor(IDC_TEXTCOLORCHOOSER, m_clrCoverText);
	CheckDlgButton(IDC_SHADOW, m_bCoverShadow ? BST_CHECKED : BST_UNCHECKED);
	OnToggleDefCover();

	// draw title over cover
	CheckDlgButton(IDC_DRAWTITLE, m_bDrawTitle ? BST_CHECKED : BST_UNCHECKED);
	
	// set use alternate folder
	CheckDlgButton(IDC_USE_ALT_FOLDER, m_bSearchAltFolder ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_ALTERNATE_FOLDER).SetWindowText(m_szAltFolder);
	OnToggleUseAltFolder();

	// search album folder
	CheckDlgButton(IDC_SEARCH_ALBUM_DIR, m_bSearchFolder ? BST_CHECKED : BST_UNCHECKED);
	OnToggleUseAlbumFolder();

	// custom border color
	CheckDlgButton(IDC_BORDER_COLOR, m_bCustomBorderColor ? BST_CHECKED : BST_UNCHECKED);
	OnToggleBorderColor();

	// search order
	GetDlgItem(IDC_SEARCH_ORDER).SetWindowText(m_szCoverSearchExt);

	// cache cover
	CheckDlgButton(IDC_CACHE_COVERS, m_bCacheCovers ? BST_CHECKED : BST_UNCHECKED);

	return TRUE;
}

BOOL CConfigCover::OnDestroy()
{
	// update dimensions
	m_nCoverWidth	= SendDlgItemMessage(IDC_COVER_WIDTH_SPIN, UDM_GETPOS);
	m_nCoverHeight	= SendDlgItemMessage(IDC_COVER_HEIGHT_SPIN, UDM_GETPOS);
	m_nBorderWidth	= SendDlgItemMessage(IDC_BORDER_WIDTH_SPIN, UDM_GETPOS);

	// update default cover
	CWnd wndDefaultCover = GetDlgItem(IDC_DEFAULT_COVER);
	wndDefaultCover.GetWindowText(m_szDefaultCover, MAX_PATH);

	m_bCoverShadow		= IsDlgButtonChecked(IDC_SHADOW);
	m_bSearchFolder		= IsDlgButtonChecked(IDC_SEARCH_ALBUM_DIR);
	m_bSearchAltFolder	= IsDlgButtonChecked(IDC_USE_ALT_FOLDER);
	m_bOverrideDefCover	= IsDlgButtonChecked(IDC_OVERRIDE_DEF_COVER);
	m_bSearchMP3		= IsDlgButtonChecked(IDC_INSIDE_MP3);
	GetDlgItem(IDC_ALTERNATE_FOLDER).GetWindowText(m_szAltFolder, MAX_PATH);
	GetDlgItem(IDC_SEARCH_ORDER).GetWindowText(m_szCoverSearchExt, MAX_PATH);

	m_bCustomBorderColor= IsDlgButtonChecked(IDC_BORDER_COLOR);

	m_bDrawTitle		= IsDlgButtonChecked(IDC_DRAWTITLE);
	m_bCacheCovers		= IsDlgButtonChecked(IDC_CACHE_COVERS);

	return TRUE;
}

UINT CALLBACK OFNHookProc(
  HWND hdlg,      // handle to child dialog window
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
)
{
	if (uiMsg == WM_INITDIALOG)
	{
		PostMessage(hdlg, WM_USER+1000,0,0);
	}
	else if (uiMsg == (WM_USER+1000))
	{
		HWND hShellDef = GetDlgItem(GetParent(hdlg), lst2);
		if (hShellDef)
		{
			// thumbnail mode
			SendMessage(hShellDef, WM_COMMAND, 0x702d, 0);

			return TRUE;
		}	
	}
	else if (uiMsg == WM_DESTROY)
	{
	}
	return 0;
}

BOOL CConfigCover::OnCommand(WORD wNotifyCode, WORD wID, CWnd wndCtl)
{
	if (wNotifyCode == BN_CLICKED)
	{
		switch (wID)
		{
		case IDC_COLORCHOOSER:
			OnColorChooser(IDC_COLORCHOOSER, RGB(0,255,0), m_clrCoverBorder);
			return TRUE;
		
		case IDC_TEXTCOLORCHOOSER:
			OnColorChooser(IDC_TEXTCOLORCHOOSER, RGB(255,255,255), m_clrCoverText);
			return TRUE;

		case IDC_BROWSE:
			OnBrowse();
			return TRUE;

		case IDC_BROWSE2:
			OnBrowseFolder();
			return TRUE;

		case IDC_USE_ALT_FOLDER:
			OnToggleUseAltFolder();
			return TRUE;

		case IDC_INSIDE_MP3:
			OnToggleInsideMP3();
			return TRUE;

		case IDC_OVERRIDE_DEF_COVER:
			OnToggleDefCover();
			return TRUE;

		case IDC_DRAWTITLE:
			OnToggleDrawTitle();
			return TRUE;

		case IDC_SEARCH_ALBUM_DIR:
			OnToggleUseAlbumFolder();
			return TRUE;

		case IDC_BORDER_COLOR:
			OnToggleBorderColor();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CConfigCover::OnNotify(UINT nID, NMHDR *pnmh, LRESULT *pResult)
{
	switch (pnmh->idFrom)
	{
	case IDC_COLORCHOOSER:
	case IDC_TEXTCOLORCHOOSER:
		if (pnmh->code == NM_CUSTOMDRAW)
		{
			return OnCustomDrawColorBtn(pnmh, pResult);
		}
		break;
	}

	return FALSE;
}

void CConfigCover::OnToggleBorderColor()
{
	int cbc = IsDlgButtonChecked(IDC_BORDER_COLOR);

	GetDlgItem(IDC_COLORCHOOSER).EnableWindow(cbc);
}

void CConfigCover::OnColorChooser(int nColorId, COLORREF clrDef, COLORREF &color)
{
	static COLORREF acrCustClr[16]; // array of custom colors 

	// Initialize CHOOSECOLOR 
	CHOOSECOLOR cc;                 // common dialog box structure 
	ZeroMemory(&cc, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = m_hWnd;
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = color;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
 
	if (ChooseColor(&cc)==TRUE)
	{
		color = cc.rgbResult; 

		UpdateColor(nColorId, color);
	}
	
}

void CConfigCover::OnBrowse()
{
	OPENFILENAMEW ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEW));
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = L"Cover Art\0*.jpg;*.jpeg;*.gif;*.bmp;*.png\0\0";
	ofn.lpstrFile = m_szDefaultCover;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_ENABLEHOOK|OFN_EXPLORER|OFN_ENABLESIZING;
	ofn.lpfnHook = OFNHookProc;

	if (GetOpenFileNameW(&ofn))
	{
		GetDlgItem(IDC_DEFAULT_COVER).SetWindowText(m_szDefaultCover);
	}
	else
	{
		GetDlgItem(IDC_DEFAULT_COVER).GetWindowText(m_szDefaultCover, MAX_PATH);
	}
}

void CConfigCover::OnBrowseFolder()
{
	AutoWide title(ALS("Select the folder."), wndWinampAL.GetCodepage());
	wchar_t szBuffer[MAX_PATH] = L"";

	if (BrowseForFolder(m_hWnd, title, szBuffer, MAX_PATH))
	{
		GetDlgItem(IDC_ALTERNATE_FOLDER).SetWindowText(szBuffer);
	}
}

void CConfigCover::OnToggleUseAltFolder()
{
	int uaf = IsDlgButtonChecked(IDC_USE_ALT_FOLDER);

	GetDlgItem(IDC_ALTERNATE_FOLDER).EnableWindow(uaf);
	GetDlgItem(IDC_BROWSE2).EnableWindow(uaf);
}

void CConfigCover::OnToggleUseAlbumFolder()
{
	int sad = IsDlgButtonChecked(IDC_SEARCH_ALBUM_DIR);

	GetDlgItem(IDC_SEARCH_ORDER).EnableWindow(sad);
}

void CConfigCover::OnToggleInsideMP3()
{
}

void CConfigCover::OnToggleDefCover()
{
	int odc = IsDlgButtonChecked(IDC_OVERRIDE_DEF_COVER);

	GetDlgItem(IDC_DEFAULT_COVER).EnableWindow(odc);
	GetDlgItem(IDC_BROWSE).EnableWindow(odc);
}

void CConfigCover::OnToggleDrawTitle()
{
	int dt = IsDlgButtonChecked(IDC_DRAWTITLE);

	GetDlgItem(IDC_TEXT_COLOR_STATIC).EnableWindow(dt);
	GetDlgItem(IDC_TEXTCOLORCHOOSER).EnableWindow(dt);
	GetDlgItem(IDC_SHADOW).EnableWindow(dt);
}

void CConfigCover::UpdateColor(int nID, COLORREF color)
{
	RECT rc = { 0, 0, COLOR_DIM, COLOR_DIM };

	HDC		hDC		= GetDC(m_hWnd);
	HDC		hMem	= CreateCompatibleDC(hDC);
	HBITMAP	hBitmap	= CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
	HBRUSH	hBrush	= CreateSolidBrush(color);

	// fill solid rect
	HBITMAP oBitmap = (HBITMAP)SelectObject(hMem, hBitmap);
	FillRect(hMem, &rc, hBrush); 
	SelectObject(hMem, oBitmap);

	hBitmap = (HBITMAP)SendDlgItemMessage(nID, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	DeleteObject(hBitmap);
	DeleteObject(hBrush);
	DeleteDC(hMem);
	ReleaseDC(m_hWnd, hDC);
}

BOOL CConfigCover::OnCustomDrawColorBtn(NMHDR* pnmh, LRESULT* pResult)
{
	*pResult = CDRF_DODEFAULT;

	LPNMCUSTOMDRAW pCD = (LPNMCUSTOMDRAW) pnmh;

	if (!g_xpStyle.IsAppThemed() || !g_xpStyle.IsThemeActive())
	{
		// themes not active - draw normally
		return FALSE;
	}
	
	if (pCD->dwDrawStage == CDDS_PREERASE)
	{
		// get theme handle
		HTHEME hTheme = g_xpStyle.OpenThemeData (m_hWnd, L"BUTTON");
		if (hTheme == NULL) return FALSE;
		
		HDC			hDC		= CreateCompatibleDC(pCD->hdc);;
		HBITMAP		hBitmap	= CreateCompatibleBitmap(pCD->hdc, pCD->rc.right, pCD->rc.bottom);
		HBITMAP		oBitmap	= (HBITMAP)SelectObject(hDC, hBitmap);

		DWORD style = GetWindowLong(pnmh->hwndFrom, GWL_STYLE);

		// determine state for DrawThemeBackground()
		// note: order of these tests is significant
		int state_id = PBS_NORMAL;
		if		(style & WS_DISABLED)				state_id = PBS_DISABLED;
		else if (pCD->uItemState & CDIS_SELECTED)	state_id = PBS_PRESSED;
		else if (pCD->uItemState & CDIS_HOT)		state_id = PBS_HOT;
		else if (style & BS_DEFPUSHBUTTON)			state_id = PBS_DEFAULTED;
		
		// erase background (according to parent window's themed background
		g_xpStyle.DrawThemeParentBackground (m_hWnd, hDC, &pCD->rc);

		// draw themed button background appropriate to button state
		g_xpStyle.DrawThemeBackground(hTheme, hDC, BP_PUSHBUTTON, state_id, &pCD->rc, NULL);
		
		// get content rectangle (space inside button for image)
		RECT content_rect (pCD->rc); 
		g_xpStyle.GetThemeBackgroundContentRect(hTheme, hDC, BP_PUSHBUTTON, state_id, &pCD->rc, &content_rect);
		
		// we're done with the theme
		g_xpStyle.CloseThemeData(hTheme);
		
		// draw the image
		HDC		hMem	 = CreateCompatibleDC(hDC);
		HBITMAP hBitmap2 = (HBITMAP)SendDlgItemMessage(pnmh->idFrom, BM_GETIMAGE, IMAGE_BITMAP, 0);
		HBITMAP oBitmap2 = (HBITMAP)SelectObject(hMem, hBitmap2);
			
		RECT rc = content_rect;
		InflateRect(&rc, -3, -2);

		StretchBlt(hDC, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, hMem, 0, 0, COLOR_DIM, COLOR_DIM, SRCCOPY);
			
		SelectObject(hMem, oBitmap2);
		DeleteDC(hMem);
		
		HPEN hPen = CreatePen(PS_SOLID, 0, RGB(0,0,0));
		HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);

		// draw the selection rectangle
		MoveToEx(hDC, rc.left, rc.top, NULL);
		LineTo(hDC, rc.left, rc.bottom-1);
		LineTo(hDC, rc.right-1, rc.bottom-1);
		LineTo(hDC, rc.right-1, rc.top);
		LineTo(hDC, rc.left, rc.top);

		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);

		// copy the offscreen drawing to onscreen
		BitBlt(pCD->hdc, pCD->rc.left, pCD->rc.top, pCD->rc.right-pCD->rc.left, pCD->rc.bottom-pCD->rc.top, hDC, pCD->rc.left, pCD->rc.top, SRCCOPY);

		// cleanup
		SelectObject(hDC, oBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hDC);

		*pResult = CDRF_SKIPDEFAULT;
	}

	return TRUE;
}
