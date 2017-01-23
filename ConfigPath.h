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
#ifndef _CONFIGPATH_H_
#define _CONFIGPATH_H_

#include "Util.h"

//////////////////////////////////////////////////////////////////
// CConfigPath class

class CConfigPath : public CDialog
{
public:
	CConfigPath();
	virtual ~CConfigPath();

	// dialog data
	CStringArrayW		m_SearchPaths;
	wchar_t				m_szExtList[MAX_PATH];
	BOOL				m_bStartupScan;
	BOOL				m_bScanSubDir;
	BOOL				m_bScanCDROM;
	BOOL				m_bIgnorePL;
	BOOL				m_bIgnoreRootPL;
	BOOL				m_bPLOnly;

	BOOL				m_bNeedRefresh;

protected:
	virtual BOOL OnInitDialog	();
	virtual BOOL OnDestroy		();
	virtual BOOL OnCommand		(WORD wNotifyCode, WORD wID, CWnd wndCtl);

	void OnAdd					();
	void OnRemove				();
	void OnRemoveAll			();
	void OnIgnoreM3U			();
	void OnM3UOnly				();

private:
	void AdjustScrollWidth		();

};

#endif /* _CONFIGPATH_H_ */

