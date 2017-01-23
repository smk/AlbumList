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
#ifndef _CONFIGCOVER_H_
#define _CONFIGCOVER_H_

#include "Util.h"

//////////////////////////////////////////////////////////////////
// CConfigCover class

class CConfigCover : public CDialog
{
public:
	CConfigCover();
	virtual ~CConfigCover();

	int			m_nCoverWidth;
	int			m_nCoverHeight;
	int			m_nBorderWidth;
	COLORREF	m_clrCoverBorder;
	COLORREF	m_clrCoverText;
	BOOL		m_bCoverShadow;
	BOOL		m_bSearchFolder;
	BOOL		m_bSearchAltFolder;
	BOOL		m_bOverrideDefCover;
	BOOL		m_bCustomBorderColor;

	BOOL		m_bSearchMP3;
	BOOL		m_bDrawTitle;
	BOOL		m_bCacheCovers;
	wchar_t		m_szDefaultCover[MAX_PATH];
	wchar_t		m_szCoverSearchExt[MAX_PATH];
	wchar_t		m_szAltFolder[MAX_PATH];

protected:
	virtual BOOL OnInitDialog	();
	virtual BOOL OnDestroy		();
	virtual BOOL OnCommand		(WORD wNotifyCode, WORD wID, CWnd wndCtl);
	virtual BOOL OnNotify		(UINT nID, NMHDR *pnmh, LRESULT *pResult);

private:
	void OnColorChooser			(int nColorId, COLORREF clrDef, COLORREF &color);
	void OnBrowse				();
	void OnBrowseFolder			();
	void OnToggleUseAltFolder	();
	void OnToggleInsideMP3		();
	void OnToggleDefCover		();
	void OnToggleDrawTitle		();
	void OnToggleUseAlbumFolder	();
	void OnToggleBorderColor	();
	BOOL OnCustomDrawColorBtn	(NMHDR* pnmh, LRESULT* pResult);
	void UpdateColor			(int nID, COLORREF color);
};

#endif /* _CONFIGCOVER_H_ */

