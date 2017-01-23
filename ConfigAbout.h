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
#ifndef _CONFIGABOUT_H_
#define _CONFIGABOUT_H_

#include "Util.h"
#include "HTMLite.h"

//////////////////////////////////////////////////////////////////
// CConfigAbout class

class CConfigAbout : public CDialog
{
public:
	CConfigAbout();
	virtual ~CConfigAbout();

protected:
	virtual BOOL OnInitDialog	();
	virtual BOOL OnDestroy		();

	CHTMLite	m_HTMLAbout;

	char		m_clrAL[8];
	char		m_clrNorm[8];
	char		m_clrLink[8];
};

#endif /* _CONFIGABOUT_H_ */

