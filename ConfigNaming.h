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
#ifndef _CONFIGNAMING_H_
#define _CONFIGNAMING_H_

#include "Util.h"

//////////////////////////////////////////////////////////////////
// CConfigNaming class

class CConfigNaming : public CDialog
{
public:
	CConfigNaming();
	virtual ~CConfigNaming();

	int  m_nDirStyle;
    BOOL m_bFixTitles;
	BOOL m_bMultiDiscFix;
	BOOL m_bUseID3;
	wchar_t m_szMultiDiscNames[MAX_PATH];
	BOOL m_bIgnoreTHE;

	BOOL m_bNeedRefresh;

protected:
	virtual BOOL OnInitDialog	();
	virtual BOOL OnDestroy		();

private:
	void RecalcDropWidth		();
};

#endif /* _CONFIGNAMING_H_ */

