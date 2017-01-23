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
#include <commctrl.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "ConfigHistory.h"

//////////////////////////////////////////////////////////////////
// CConfigHistory class

CConfigHistory::CConfigHistory()
{
	m_nIDTemplate = IDD_CONFIGHISTORY_v290;
	if (wndWinamp.IsWinamp5())
		m_nIDTemplate = IDD_CONFIGHISTORY_v500;
}

CConfigHistory::~CConfigHistory()
{
}

BOOL CConfigHistory::OnInitDialog()
{
	CWnd wndEdit = GetDlgItem(IDC_EDIT);

	char filename[512];
	lstrcpy(filename, dirPlugin);
	PathAppend(filename, "Album List\\history.txt");

	BOOL bTextLoaded = FALSE;

	HANDLE fp = NULL;
	if (INVALID_HANDLE_VALUE != (fp = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)))
	{
		DWORD dwSize = GetFileSize (fp, NULL);

		char *buffer = new char [dwSize+1];

		DWORD nBytesRead;
		BOOL bResult = ReadFile(fp, buffer, dwSize, &nBytesRead, NULL);
		if (bResult)
		{
			buffer[dwSize] = 0;
			wndEdit.SetWindowText(buffer);

			bTextLoaded = TRUE;
		}

		delete [] buffer;

		CloseHandle(fp);
	}

	if (!bTextLoaded)
	{
		wndEdit.SetWindowText(ALS("history.txt not found"));
	}

	return TRUE;
}
