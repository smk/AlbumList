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
#include <Shlwapi.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "AlbumInfo.h"

//////////////////////////////////////////////////////////////////
// CAlbumInfo class

CAlbumInfo::CAlbumInfo(CAlbum *pAlbum)
{
	m_pAlbum = pAlbum;
	m_nIDTemplate = IDD_ALBUMINFO;

	// lock the album for use
	if (m_pAlbum) m_pAlbum->AddRef();
}

CAlbumInfo::~CAlbumInfo()
{
	// release the album for deletion
	if (m_pAlbum) m_pAlbum->Release();
}

BOOL CAlbumInfo::OnInitDialog()
{
	RECT rcWnd, rcParent;
	GetParent().GetWindowRect(&rcParent);
	GetWindowRect(&rcWnd);

	int nHeight = rcWnd.bottom - rcWnd.top;

	if (GetSystemMetrics(SM_CYSCREEN) >= (rcParent.bottom + nHeight))
	{
		SetWindowPos(NULL, rcWnd.left, rcParent.bottom, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	}
	else if (0 <= (rcParent.top - nHeight))
	{
		SetWindowPos(NULL, rcWnd.left, rcParent.top - nHeight, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	}

	if (m_pAlbum)
	{
		int s  = m_pAlbum->GetTotalTime();
		int ss = s%60;
		int m  = (s-ss)/60;
		int mm = m%60;
		int h = (m-mm)/60;

		char szLength[16]; // temporary string

		// make a "hh:mm:ss" if there is any hours, otherwise
		// make it   "mm:ss"
		if (h>0)
		{
			wsprintf(szLength,"%02ld:%02ld:%02ld", h, mm, ss);
		}
		else
		{
			wsprintf(szLength,        "%ld:%02ld",    mm, ss);
		}

		SetDlgItemText(IDC_LENGTH, szLength); 
		
		if (m_pAlbum->IsPlaylistBased())
			PathSetDlgItemPathW(GetSafeHwnd(), IDC_PATH, m_pAlbum->GetM3U());
		else
			PathSetDlgItemPathW(GetSafeHwnd(), IDC_PATH, m_pAlbum->GetPath());

		SetDlgItemText(IDC_ARTIST,	m_pAlbum->GetArtist(), wndWinampAL.GetEncodingCodepage());
		SetDlgItemText(IDC_ALBUM,	m_pAlbum->GetAlbum(), wndWinampAL.GetEncodingCodepage());
		SetDlgItemText(IDC_LISTNAME,m_pAlbum->GetTitle(), wndWinampAL.GetEncodingCodepage());
		SetDlgItemText(IDC_GENRE,	m_pAlbum->GetGenre(), wndWinampAL.GetEncodingCodepage());

		SetDlgItemInt(IDC_FILECOUNT,m_pAlbum->GetTrackCount());
		SetDlgItemInt(IDC_YEAR,		m_pAlbum->GetYear());
		SetDlgItemInt(IDC_MULTIDISC,m_pAlbum->GetNumberOfDiscs());
	}

	return TRUE;
}

void CAlbumInfo::OnOK()
{
	if (m_pAlbum)
	{
		wchar_t str[MAX_PATH];

		GetDlgItemText(IDC_ARTIST,	str, MAX_PATH, wndWinampAL.GetEncodingCodepage());
		m_pAlbum->SetArtist(str);

		GetDlgItemText(IDC_ALBUM,	str, MAX_PATH, wndWinampAL.GetEncodingCodepage());
		m_pAlbum->SetAlbum(str);

		GetDlgItemText(IDC_LISTNAME,str, MAX_PATH, wndWinampAL.GetEncodingCodepage());
		m_pAlbum->SetTitle(str);

		GetDlgItemText(IDC_GENRE,	str, MAX_PATH, wndWinampAL.GetEncodingCodepage());
		m_pAlbum->SetGenre(str);

		m_pAlbum->SetYear(GetDlgItemInt(IDC_YEAR, NULL, TRUE));
	}

	CDialog::OnOK();
}
