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
#ifndef _JUMPTOALBUM_H_
#define _JUMPTOALBUM_H_

#include "Util.h"

//////////////////////////////////////////////////////////////////
// CSearchEdit class

class CSearchEdit : public CWnd
{
public:
	CSearchEdit();
	virtual ~CSearchEdit();

protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

//////////////////////////////////////////////////////////////////
// CJumpToAlbum class

class CAlbumList;
class CJumpToAlbum : public CDialog
{
public:
	CJumpToAlbum();
	virtual ~CJumpToAlbum();

	CAlbumList *m_pAlbumList;
	CSearchEdit	m_wndEdit;

protected:
	virtual BOOL OnInitDialog	();
	virtual BOOL OnDestroy		();
	virtual BOOL OnCommand		(WORD wNotifyCode, WORD wID, CWnd wndCtl);
	virtual void OnOK			();

	void OnChangeSearchText		();
	void OnDblClkResultList		();

	void OnPlay					();
	void OnEnqueue				();
};

#endif /* _JUMPTOALBUM_H_ */

