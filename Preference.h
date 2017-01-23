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
#ifndef _PREFERENCE_H_
#define _PREFERENCE_H_

#include "Util.h"

//////////////////////////////////////////////////////////////////
// CPreference class

class CPreference : public CDialog
{
public:
	CPreference();
	virtual ~CPreference();

	int				m_nLastConfig;

	void AddPage(LPCTSTR string, CDialog *pDialog);

protected:
	virtual BOOL OnInitDialog	();
	virtual BOOL OnDestroy		();
	virtual BOOL OnNotify		(UINT nID, NMHDR *pnmh, LRESULT *pResult);

	BOOL OnNotifyTCNSelChanging	();
	BOOL OnNotifyTCNSelChanged	();

private:
	CStringArray	m_strConfigs;
	CPtrArray		m_wndConfigs;
};

#endif /* _PREFERENCE_H_ */

