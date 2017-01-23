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
#include <stdlib.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "HTMLite.h"

#define AL_HTMLITE_CLASS		"AL HTMLite Class"

//////////////////////////////////////////////////////////////////
// CAnchor class

CAnchor::CAnchor()
{
}

CAnchor::CAnchor(RECT rect, LPCTSTR str)
{
	rc = rect;
	lstrcpyn(url, str, MAX_PATH);
}

CAnchor::~CAnchor()
{
}

//////////////////////////////////////////////////////////////////
// CHTMLite class

CHTMLite::CHTMLite()
{
}

CHTMLite::~CHTMLite()
{
	int size = m_AnchorList.GetSize();
	for (int i=0; i<size; i++)
		delete (CAnchor *)m_AnchorList.GetAt(i);
	m_AnchorList.RemoveAll();
}

BOOL CHTMLite::Create(int x, int y, int cx, int cy, CWnd wndParent)
{
	WNDCLASSEX wndclassex;
	wndclassex.cbSize = sizeof(wndclassex);
	if (!GetClassInfoEx(hDllInstance, AL_HTMLITE_CLASS, &wndclassex))
	{
		wndclassex.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
		wndclassex.lpfnWndProc = WindowProc;
		wndclassex.lpszClassName = AL_HTMLITE_CLASS;
		wndclassex.cbClsExtra = 0;
		wndclassex.cbWndExtra = 0;
		wndclassex.hInstance = hDllInstance;
		wndclassex.hIcon = NULL;
		wndclassex.hbrBackground = NULL;//(HBRUSH)COLOR_WINDOW;
		wndclassex.lpszMenuName = NULL;
		wndclassex.hIconSm = NULL;
		wndclassex.hCursor = NULL;

		if (!RegisterClassEx(&wndclassex))
			return FALSE;
	}

    // Create the main window. 
    m_hWnd = CreateWindowEx(WS_EX_TRANSPARENT, AL_HTMLITE_CLASS, "a", WS_CHILD, x, y, cx, cy, wndParent, NULL, hDllInstance, (LPVOID) this);

	if (!m_hWnd) 
		return FALSE;

	return TRUE;
}

int CHTMLite::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return 0;
}

BOOL CHTMLite::OnDestroy()
{
	return FALSE;
}

BOOL CHTMLite::OnPaint(HDC hdc)
{
	int size = m_AnchorList.GetSize();
	for (int i=0; i<size; i++)
		delete (CAnchor *)m_AnchorList.GetAt(i);
	m_AnchorList.RemoveAll();

	// get window text first
	int n = GetWindowTextLength(m_hWnd);
	char *strText = new char [n+1];
	GetWindowText(strText, n+1);
	strText[n] = 0;
	char str1[MAX_PATH];

	// begin draw
	PAINTSTRUCT ps;
	BeginPaint(m_hWnd, &ps);

	// use the default if none is specified
	if (hdc == NULL) 
		hdc = ps.hdc;

	RECT rc;
	GetClientRect(&rc);

	rc.left += 5;		rc.right -= 5;
	rc.top += 5;		rc.bottom -= 5;

	SetBkMode(hdc, TRANSPARENT);

	LOGFONT lf;
	GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
	LOGFONT prevLogFont = lf;

	HFONT font = NULL;

	int		nBold = 0, nItalic = 0, nUnderline = 0, nStrikeThru = 0;
	BOOL	bInAnchor = FALSE, bInFont = FALSE;
	char   *p = strText;
	char	strAnchorText[MAX_PATH] = "";
	int		nInitialXOffset = rc.left;

	COLORREF textColor = RGB(0,0,0);
	COLORREF prevColor = RGB(0,0,0);

	while (n > 0)
	{
		///////////////////////////////////////////////////////////////////////
		if (_strnicmp(p, ("<B>"), 3) == 0)			// check for <b> or <B>
		{
			n -= 3;
			p += 3;
			nBold++;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("</B>"), 4) == 0)	// check for </b> or </B>
		{
			n -= 4;
			p += 4;
			nBold--;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("<I>"), 3) == 0)			// check for <i> or <I>
		{
			n -= 3;
			p += 3;
			nItalic++;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("</I>"), 4) == 0)	// check for </i> or </I>
		{
			n -= 4;
			p += 4;
			nItalic--;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("<U>"), 3) == 0)			// check for <u> or <U>
		{
			n -= 3;
			p += 3;
			nUnderline++;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("</U>"), 4) == 0)	// check for </u> or </U>
		{
			n -= 4;
			p += 4;
			nUnderline--;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("<STRIKE>"), 8) == 0)		// check for <strike> or <STRIKE>
		{
			n -= 8;
			p += 8;
			nStrikeThru++;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("</STRIKE>"), 9) == 0)// check for </strike> or </STRIKE>
		{
			n -= 9;
			p += 9;
			nStrikeThru--;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("<BR>"), 4) == 0)	// check for <br>
		{
			n -= 4;
			p += 4;

			SIZE size;
			GetTextExtentPoint32(hdc, "test", 4, &size);
			rc.top += size.cy;
			nInitialXOffset = rc.left;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		// <a href=www.xyz.com>XYZ Web Site</a>
		else if (_strnicmp(p, ("<A HREF="), 8) == 0)	// check for <A HREF=
		{
			n -= 8;
			p += 8;

			char *p2 = strchr(p, '>');
			if (p2)
			{
				int l = p2 - p;				// length
				lstrcpyn(strAnchorText, p, l+1);
				n -= l+1;
				p += l+1;
				bInAnchor = TRUE;
				continue;
			}
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("</A>"), 4) == 0)	// check for </A>
		{
			n -= 4;
			p += 4;
			bInAnchor = FALSE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("<FONT"), 5) == 0)			// check for <font>
		{
			n -= 5;
			p += 5;

			char *p2 = strchr(p, '>');
			if (p2)
			{
				char strAttribute[MAX_PATH];

				int l = p2 - p;				// length
				lstrcpyn(strAttribute, p, l+1);
				n -= l+1;
				p += l+1;
				bInFont = TRUE;

				char *a = strAttribute;
				while (l > 0)
				{
					// trim left whitespace
					if (*a == ' ')
					{
						l -= 1;
						a += 1;
						continue;
					}

					///////////////////////////////////////////////////////////
					if (_strnicmp(a, ("COLOR"), 5) == 0)
					{
						l -= 5;
						a += 5;

						// find first quote
						if ((p2 = strchr(a, '"')) != NULL)
						{
							p2++;
							// find second quote
							char *p3;
							if ((p3 = strchr(p2, '"')) != NULL)
							{
								char strColor[64];
								lstrcpyn(strColor, p2, p3-p2+1);
								textColor = GetColorFromString(strColor);
								prevColor = SetTextColor(hdc, textColor);

								l -= p3-p2+3;
								a += p3-p2+3;
								continue;
							}
						}
					}
					///////////////////////////////////////////////////////////
					else if (_strnicmp(a, ("SIZE"), 4) == 0)
					{
						l -= 4;
						a += 4;

						// find first quote
						if ((p2 = strchr(a, '"')) != NULL)
						{
							p2++;
							// find second quote
							char *p3;
							if ((p3 = strchr(p2, '"')) != NULL)
							{
								char strSize[64];
								lstrcpyn(strSize, p2, p3-p2+1);
								lf.lfHeight -= atoi(strSize);

								l -= p3-p2+3;
								a += p3-p2+3;
								continue;
							}
						}
					}
					///////////////////////////////////////////////////////////
					else if (_strnicmp(a, ("FACE"), 4) == 0)
					{
						l -= 4;
						a += 4;

						// find first quote
						if ((p2 = strchr(a, '"')) != NULL)
						{
							p2++;
							// find second quote
							char *p3;
							if ((p3 = strchr(p2, '"')) != NULL)
							{
								lstrcpyn(lf.lfFaceName, p2, p3-p2+1);

								l -= p3-p2+3;
								a += p3-p2+3;
								continue;
							}
						}
					}
				}

				continue;
			}
		}
		///////////////////////////////////////////////////////////////////////
		else if (_strnicmp(p, ("</FONT>"), 7) == 0)			// check for </font>
		{
			n -= 7;
			p += 7;
			bInFont = FALSE;

			// restore font attributes
			lf = prevLogFont;
			textColor = prevColor;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else
		{
			char *p2 = strchr(p, '<');
			if (p2)
			{
				int l = p2 - p;				// length
				lstrcpyn(str1, p, l+1);
				n -= l;
				p += l;
			}
			else
			{
				lstrcpy(str1, p);
				n = 0;
			}
		}

		lf.lfWeight    = (nBold > 0) ? FW_BOLD : FW_NORMAL;
		lf.lfUnderline = (BYTE) (nUnderline > 0);
		lf.lfItalic    = (BYTE) (nItalic > 0);
		lf.lfStrikeOut = (BYTE) (nStrikeThru > 0);

		if (font) DeleteObject(font);
		font = CreateFontIndirect(&lf);
		SelectObject(hdc, font);
		SetTextColor(hdc, textColor);

		nInitialXOffset = FormatText(hdc, str1, &rc, nInitialXOffset);

		if (bInAnchor)
		{
			SIZE size;
			GetTextExtentPoint32(hdc, str1, lstrlen(str1), &size);

			RECT rect = rc;
			rect.left = nInitialXOffset - size.cx;
			rect.right = nInitialXOffset;
			rect.bottom = rect.top + size.cy;

			m_AnchorList.Add(new CAnchor(rect, strAnchorText));

			DbgPrint("Added Anchor: %s\n", strAnchorText);
		}
	}

	// cleanup
	EndPaint(m_hWnd, &ps);

	delete [] strText;

	return TRUE;
}

int CHTMLite::FormatText(HDC hdc, LPCTSTR lpszText, RECT *pRect, int nInitialXOffset)
{
	SIZE size;
	GetTextExtentPoint32(hdc, lpszText, lstrlen(lpszText), &size);

	RECT rc = *pRect;
	rc.left = nInitialXOffset;

	DrawText(hdc, lpszText, -1, &rc, 0);

	return nInitialXOffset + size.cx;
}

COLORREF CHTMLite::GetColorFromString(LPCTSTR lpszColor)
{
	COLORREF rgb = RGB(0,0,0);		// initialize to black
	BYTE r = 0;
	BYTE g = 0;
	BYTE b = 0;
	const TCHAR *cp;

	if ((cp = strchr(lpszColor, ',')) != NULL)
	{
		// "255,0,0"
		r = (BYTE) atoi(lpszColor);
		cp++;
		g = (BYTE) atoi(cp);
		cp = strchr(cp, ',');
		if (cp)
		{
			cp++;
			b = (BYTE) atoi(cp);
		}
		rgb = RGB(r,g,b);
	}
	else if ((cp = strchr(lpszColor, '#')) != NULL)
	{
		// "#0000FF"
		if (lstrlen(lpszColor) == 7)
		{
			TCHAR s[3];
			cp++;
			s[0] = *cp++;
			s[1] = *cp++;
			s[2] = '\0';
			r = (BYTE)strtoul(s, NULL, 16);
			s[0] = *cp++;
			s[1] = *cp++;
			g = (BYTE)strtoul(s, NULL, 16);
			s[0] = *cp++;
			s[1] = *cp++;
			b = (BYTE)strtoul(s, NULL, 16);
			rgb = RGB(r,g,b);
		}
	}

	return rgb;
}

BOOL CHTMLite::OnMouseMove(UINT nFlags, POINT point)
{
	int size = m_AnchorList.GetSize();
	for (int i=0; i<size; i++)
	{
		CAnchor *a = (CAnchor *)m_AnchorList.GetAt(i);
		if (a)
		{
			if (PtInRect(&a->rc, point))
			{
				SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(32649)));
				return TRUE;
			}
		}
	}

	SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
	return FALSE;
}

BOOL CHTMLite::OnLButtonDown(UINT nFlags, POINT point)
{
	int size = m_AnchorList.GetSize();
	for (int i=0; i<size; i++)
	{
		CAnchor *a = (CAnchor *)m_AnchorList.GetAt(i);
		if (a)
		{
			if (PtInRect(&a->rc, point))
			{
				DbgPrint("Clicked on %s\n", a->url);
				GotoURL(a->url, SW_SHOWNORMAL);
				return TRUE;
			}
		}
	}

	return FALSE;
}

LRESULT CHTMLite::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_SETCURSOR)
	{
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		return TRUE;
	}

	return CWnd::DefWindowProc(message, wParam, lParam);
}
