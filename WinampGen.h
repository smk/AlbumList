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
#ifndef __WINAMP_GEN_H__
#define __WINAMP_GEN_H__

#include "Util.h"

//////////////////////////////////////////////////////////////////
// CWinampGen class

class CUserInterface;
class CWinampGen : public CWnd
{
public:
	CWinampGen();
	virtual ~CWinampGen();

	CUserInterface *m_pObj;

protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

#endif /* __WINAMP_GEN_H__ */