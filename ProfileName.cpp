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
#include "ProfileName.h"

//////////////////////////////////////////////////////////////////
// CProfileName class

CProfileName::CProfileName()
{
	m_nIDTemplate = IDD_PROFILENAME;

	memset(m_szProfileName, 0, sizeof(m_szProfileName));
}

CProfileName::~CProfileName()
{
}

BOOL CProfileName::OnInitDialog()
{
	CWnd wndEdit = GetDlgItem(IDC_PROFILE_EDIT);

	wndEdit.SetWindowText(m_szProfileName);

	CenterWindow();

	return TRUE;
}

void CProfileName::OnOK()
{
	CWnd wndEdit = GetDlgItem(IDC_PROFILE_EDIT);

	wndEdit.GetWindowText(m_szProfileName, sizeof(m_szProfileName));

	CDialog::OnOK();
}

LPCTSTR	CProfileName::GetName()
{
	return m_szProfileName;
}

void CProfileName::SetName(LPCTSTR szName)
{
	lstrcpy(m_szProfileName, szName);
}
