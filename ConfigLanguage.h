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
#ifndef _CONFIGLANGUAGE_H_
#define _CONFIGLANGUAGE_H_

#include "Util.h"

// translation structure
typedef struct _TAGTRANSLATION
{
	char pathname[MAX_PATH];	// full path of the language file
	char language[MAX_PATH];	// language name
	char name	 [MAX_PATH];	// name
	int  codepage;				// codepage

}	TRANSLATION, *LPTRANSLATION;

//////////////////////////////////////////////////////////////////
// CLanguageArray class

class CLanguageArray : public CPtrArray
{
public:
	CLanguageArray();

	static int Compare(const void *elem1, const void *elem2);
};

//////////////////////////////////////////////////////////////////
// CConfigLanguage class

class CConfigLanguage : public CDialog
{
public:
	CConfigLanguage();
	virtual ~CConfigLanguage();

protected:
	virtual BOOL OnInitDialog	();
	virtual BOOL OnDestroy		();
	virtual BOOL OnCommand		(WORD wNotifyCode, WORD wID, CWnd wndCtl);

	void OnTranslationChange	();
	void OnEdit					();

private:
	BOOL AddLanguage			(CComboBox wnd, ALLanguage lang);
	void GetTranslations		();

	CLanguageArray m_Translations;
};

#endif /* _CONFIGLANGUAGE_H_ */

