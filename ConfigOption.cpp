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

#include "Gen_m3a.h"
#include "ConfigOption.h"

//////////////////////////////////////////////////////////////////
// CConfigOption class

CConfigOption::CConfigOption()
{
	m_nIDTemplate = IDD_CONFIGOPTION_v290;
	if (wndWinamp.IsWinamp5())
		m_nIDTemplate = IDD_CONFIGOPTION_v500;
}

CConfigOption::~CConfigOption()
{
}

BOOL CConfigOption::OnInitDialog()
{
	// auto advance
	int aa = wndWinampAL.GetSetting(settingAutoAdvance);
	CheckDlgButton(IDC_AUTOADVANCE, aa ? BST_CHECKED : BST_UNCHECKED);

	// random
	int ar = wndWinampAL.GetSetting(settingAdvanceRandom);
	CheckDlgButton(IDC_RANDOM, ar ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_RANDOM).EnableWindow(aa);

	// show cover
	int sc = wndWinampAL.GetSetting(settingShowCover);
	CheckDlgButton(IDC_SHOWCOVER, sc ? BST_CHECKED : BST_UNCHECKED);

	// loop
	int loop = wndWinampAL.GetSetting(settingLoop);
	CheckDlgButton(IDC_LOOPNEXTPREV, loop ? BST_CHECKED : BST_UNCHECKED);

	// keep song playing
	int ksp = wndWinampAL.GetSetting(settingKeepSongPlaying);
	CheckDlgButton(IDC_KEEPSONGPLAYING,	ksp ? BST_CHECKED : BST_UNCHECKED);

	// sort by filename
	int sbf = wndWinampAL.GetSetting(settingSortByFilename);
	CheckDlgButton(IDC_SORTFILENAME, sbf ? BST_CHECKED : BST_UNCHECKED);

	// resume playback
	int rp = wndWinampAL.GetSetting(settingResumePlayback);
	CheckDlgButton(IDC_RESUME, rp ? BST_CHECKED : BST_UNCHECKED);

	// enqueue default
	int ed = wndWinampAL.GetSetting(settingEnqueueDefault);
	CheckDlgButton(IDC_ENQUEUE, ed ? BST_CHECKED : BST_UNCHECKED);

	// lightning bolt
	int lb = wndWinampAL.GetSetting(settingLightningBolt);
	CheckDlgButton(IDC_LIGHTNING_BOLT, lb ? BST_CHECKED : BST_UNCHECKED);

	// display warning
	int paw = wndWinampAL.GetSetting(settingPlayAllWarning);
	CheckDlgButton(IDC_PLAYALL_WARNING, paw ? BST_CHECKED : BST_UNCHECKED);

	char limit [17];
	wsprintf(limit, "%d", wndWinampAL.GetSetting(settingPlayAllLimit));
	GetDlgItem(IDC_PLAYALL_LIMIT).SetWindowText(limit);

	GetDlgItem(IDC_PLAYALL_GT).EnableWindow(paw);
	GetDlgItem(IDC_PLAYALL_LIMIT).EnableWindow(paw);
	GetDlgItem(IDC_PLAYALL_ALBUM).EnableWindow(paw);

	return TRUE;
}

BOOL CConfigOption::OnDestroy()
{
	wndWinampAL.SetSetting(settingAutoAdvance, IsDlgButtonChecked(IDC_AUTOADVANCE));
	wndWinampAL.SetSetting(settingAdvanceRandom, IsDlgButtonChecked(IDC_RANDOM));
	wndWinampAL.SetSetting(settingShowCover, IsDlgButtonChecked(IDC_SHOWCOVER));
	wndWinampAL.SetSetting(settingLoop, IsDlgButtonChecked(IDC_LOOPNEXTPREV));
	wndWinampAL.SetSetting(settingEnqueueDefault, IsDlgButtonChecked(IDC_ENQUEUE));
	wndWinampAL.SetSetting(settingSortByFilename, IsDlgButtonChecked(IDC_SORTFILENAME));
	wndWinampAL.SetSetting(settingKeepSongPlaying, IsDlgButtonChecked(IDC_KEEPSONGPLAYING));
	wndWinampAL.SetSetting(settingResumePlayback, IsDlgButtonChecked(IDC_RESUME));
	wndWinampAL.SetSetting(settingLightningBolt, IsDlgButtonChecked(IDC_LIGHTNING_BOLT));
	wndWinampAL.SetSetting(settingPlayAllWarning, IsDlgButtonChecked(IDC_PLAYALL_WARNING));

	char limit [32];
	GetDlgItem(IDC_PLAYALL_LIMIT).GetWindowText(limit, 32);
	int value = min(max(10, atoi(limit)), 9999);
	wndWinampAL.SetSetting(settingPlayAllLimit, value);

	return TRUE;
}

BOOL CConfigOption::OnCommand(WORD wNotifyCode, WORD wID, CWnd wndCtl)
{
	switch (wNotifyCode)
	{
	case BN_CLICKED:
		switch (wID)
		{
		case IDC_AUTOADVANCE:
			OnAutoAdvance();
			return TRUE;

		case IDC_PLAYALL_WARNING:
			OnPlayAllWarning();
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void CConfigOption::OnAutoAdvance()
{
	GetDlgItem(IDC_RANDOM).EnableWindow(IsDlgButtonChecked(IDC_AUTOADVANCE));
}

void CConfigOption::OnPlayAllWarning()
{
	int paw = IsDlgButtonChecked(IDC_PLAYALL_WARNING);

	GetDlgItem(IDC_PLAYALL_GT).EnableWindow(paw);
	GetDlgItem(IDC_PLAYALL_LIMIT).EnableWindow(paw);
	GetDlgItem(IDC_PLAYALL_ALBUM).EnableWindow(paw);
}
