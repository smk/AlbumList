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
#ifndef _CONFIGOPTION_H_
#define _CONFIGOPTION_H_

#include "Util.h"

//////////////////////////////////////////////////////////////////
// CConfigOption class

class CConfigOption : public CDialog
{
public:
	CConfigOption();
	virtual ~CConfigOption();

protected:
	virtual BOOL OnInitDialog	();
	virtual BOOL OnDestroy		();
	virtual BOOL OnCommand		(WORD wNotifyCode, WORD wID, CWnd wndCtl);

	void OnAutoAdvance			();
	void OnPlayAllWarning		();
};

#endif /* _CONFIGOPTION_H_ */

