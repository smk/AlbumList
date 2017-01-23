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
#include "ConfigAbout.h"


//////////////////////////////////////////////////////////////////
// CConfigAbout class

CConfigAbout::CConfigAbout()
{
	m_nIDTemplate = IDD_CONFIGABOUT_v290;
	if (wndWinamp.IsWinamp5())
		m_nIDTemplate = IDD_CONFIGABOUT_v500;

	lstrcpy(m_clrAL, "#ff0000");
	lstrcpy(m_clrNorm, "#000000");
	lstrcpy(m_clrLink, "#0000ff");
}

CConfigAbout::~CConfigAbout()
{
}

BOOL CConfigAbout::OnInitDialog()
{
	int major = 2, minor = 01, build = 1;
	GetVersionInformation(hDllInstance, major, minor, build);

	char about[1024];

	char *fmt = "<font color=\"%s\" size=\"+6\" face=\"Arial\">Album List v%ld.%02ld</font><br>"
				"<font color=\"%s\">Copyright (c) 1999-2007, Safai Ma, All Rights Reserved.<br>"
				"<br>"
				"Build %ld/%s (%s %s)<br>"
				"<br><br>"
				"<u>Contact Information:</u><br>"
				"<b>email:</b></font>   <font color=\"%s\"><a href=mailto:safai.ma@gmail.com>safai.ma@gmail.com</a></font><br>"
				"<font color=\"%s\"><b>website:</b></font>   <font color=\"%s\"><a href=http://albumlist.sourceforge.net>http://albumlist.sourceforge.net</a></font>";

#ifdef _DEBUG
	wsprintf(about, fmt, m_clrAL, major, minor, m_clrNorm, build, "debug", __DATE__, __TIME__, m_clrLink, m_clrNorm, m_clrLink);
#else
	wsprintf(about, fmt, m_clrAL, major, minor, m_clrNorm, build, "release", __DATE__, __TIME__, m_clrLink, m_clrNorm, m_clrLink);
#endif

	// version check
	int aa = wndWinampAL.GetSetting(settingVersionCheck);
	CheckDlgButton(IDC_CHECK_UPDATE, aa ? BST_CHECKED : BST_UNCHECKED);

	RECT rc;
	GetDlgItem(IDC_VERSION_STATIC).GetWindowRect(&rc);
	GetDlgItem(IDC_VERSION_STATIC).ShowWindow(SW_HIDE);
	ScreenToClient(&rc);

	m_HTMLAbout.Create(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, GetSafeHwnd());
	m_HTMLAbout.SetWindowText(about);
	m_HTMLAbout.ShowWindow(SW_SHOW);

	return FALSE;
}

BOOL CConfigAbout::OnDestroy()
{
	wndWinampAL.SetSetting(settingVersionCheck, IsDlgButtonChecked(IDC_CHECK_UPDATE));

	return TRUE;
}
