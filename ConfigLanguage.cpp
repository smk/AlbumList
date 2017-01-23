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
#include <stdlib.h>
#include "AutoWide.h"
#include <Shlwapi.h>
#include <Shellapi.h>
#include "resource.h"

#include "Gen_m3a.h"
#include "ConfigLanguage.h"

//////////////////////////////////////////////////////////////////
// CConfigLanguage class

CConfigLanguage::CConfigLanguage()
{
	m_nIDTemplate = IDD_CONFIGLANGUAGE_v290;
	if (wndWinamp.IsWinamp5())
		m_nIDTemplate = IDD_CONFIGLANGUAGE_v500;

	GetTranslations();
}

CConfigLanguage::~CConfigLanguage()
{
	int size = m_Translations.GetSize();
	for (int i=0; i<size; ++i)
	{
		delete (LPTRANSLATION)m_Translations.GetAt(i);
	}
	m_Translations.RemoveAll();
}

BOOL CConfigLanguage::OnInitDialog()
{
	// translations
	CComboBox wndTranslation = GetDlgItem(IDC_LANGUAGE);
	CWnd wndTranslatedBy = GetDlgItem(IDC_TRANSLATED_BY);

	int index = wndTranslation.AddString("English (Default)");
	if (m_Translations.GetSize())
	{
		LPCTSTR file = wndWinampAL.GetSetting(settingTranslation);

		int size = m_Translations.GetSize();
		int nSel = 0;
		for (int i=0; i<size; ++i)
		{
			LPTRANSLATION pTranslation = (LPTRANSLATION)m_Translations.GetAt(i);

			if (lstrcmpi(file, pTranslation->pathname) == 0)
				nSel = i+1;

			int index = wndTranslation.AddString(pTranslation->language, pTranslation->codepage);
		}
		wndTranslation.SetCurSel(nSel);
	}
	else
	{
		wndTranslation.SetCurSel(0);
		wndTranslation.EnableWindow(FALSE);
	}
	OnTranslationChange();

	// setup encoding language for AL
	CComboBox wndALLang = GetDlgItem(IDC_AL_LANG);
	AddLanguage(wndALLang, (ALLanguage)wndWinampAL.GetSetting(settingEncodingLangAL));
	if (wndALLang.GetCount() < 2)
	{
		wndALLang.EnableWindow(FALSE);
		GetDlgItem(IDC_ALBUMLIST).EnableWindow(FALSE);
	}

	// setup encoding language for PE
	CComboBox wndPELang = GetDlgItem(IDC_PE_LANG);
	AddLanguage(wndPELang, (ALLanguage)wndWinampAL.GetSetting(settingEncodingLangPE));
	if (wndPELang.GetCount() < 2)
	{
		wndPELang.EnableWindow(FALSE);
		GetDlgItem(IDC_PLAYLISTEDITOR).EnableWindow(FALSE);
	}

	if (wndWinamp.IsWinamp525())
	{
		wndPELang.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_PE).ShowWindow(SW_HIDE);
	}

	return FALSE;
}

BOOL CConfigLanguage::OnDestroy()
{
	// translations
	CComboBox wndTranslation = GetDlgItem(IDC_LANGUAGE);
	int index = wndTranslation.GetCurSel();
	if (index < 1)
	{
		wndWinampAL.SetSetting(settingTranslation, "");
	}
	else
	{
		LPTRANSLATION pTranslation = (LPTRANSLATION)m_Translations.GetAt(index-1);
		if (pTranslation)
		{
			wndWinampAL.SetSetting(settingTranslation, pTranslation->pathname);
		}
		else
		{
			wndWinampAL.SetSetting(settingTranslation, "");
		}
	}

	// get encoding language for AL
	CComboBox wndALLang = GetDlgItem(IDC_AL_LANG);
	index = wndALLang.GetCurSel();
	int lang  = wndALLang.GetItemData(index);
	wndWinampAL.SetSetting(settingEncodingLangAL, lang);

	// get encoding language for PE
	CComboBox wndPELang = GetDlgItem(IDC_PE_LANG);
	index = wndPELang.GetCurSel();
	lang  = wndPELang.GetItemData(index);
	wndWinampAL.SetSetting(settingEncodingLangPE, lang);

	return TRUE;
}

BOOL CConfigLanguage::OnCommand(WORD wNotifyCode, WORD wID, CWnd wndCtl)
{
	if (wNotifyCode == CBN_SELCHANGE)
	{
		switch (wID)
		{
		case IDC_LANGUAGE:
			OnTranslationChange();
			return TRUE;
		}
	}

	else if (wNotifyCode == BN_CLICKED)
	{
		switch (wID)
		{
		case IDC_EDIT:
			OnEdit();
			return TRUE;
		}
	}

	return FALSE;
}

void CConfigLanguage::OnTranslationChange()
{
	CComboBox wndTranslation = GetDlgItem(IDC_LANGUAGE);
	CWnd wndTranslatedBy = GetDlgItem(IDC_TRANSLATED_BY);
	CWnd wndEdit = GetDlgItem(IDC_EDIT);

	int index = wndTranslation.GetCurSel();
	if (index < 1)
	{
		wndTranslatedBy.ShowWindow(SW_HIDE);
		wndEdit.EnableWindow(FALSE);
	}
	else
	{
		LPTRANSLATION pTranslation = (LPTRANSLATION)m_Translations.GetAt(index-1);
		if (pTranslation)
		{
			char str[512];
			wsprintf(str, ALS("Translated by: %s"), pTranslation->name);
			wndTranslatedBy.SetWindowText(str);
			wndTranslatedBy.ShowWindow(SW_SHOW);
		}
		else
		{
			wndTranslatedBy.ShowWindow(SW_HIDE);
		}
		wndEdit.EnableWindow(TRUE);
	}
}

void CConfigLanguage::OnEdit()
{
	CComboBox wndTranslation = GetDlgItem(IDC_LANGUAGE);

	int index = wndTranslation.GetCurSel();
	if (index > 0)
	{
		LPTRANSLATION pTranslation = (LPTRANSLATION)m_Translations.GetAt(index-1);
		if (pTranslation)
		{
			ShellExecute(NULL, NULL, "notepad.exe", pTranslation->pathname, NULL, SW_SHOW);
		}
	}
}

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType,  LPARAM lParam)
{
	BOOL *bAvail = (BOOL*)lParam;

	*bAvail = TRUE;

	return FALSE;
}
 
BOOL CConfigLanguage::AddLanguage(CComboBox wnd, ALLanguage lang)
{
	HDC hdc = GetDC(wnd);

	LOGFONT lf;

	BOOL bChineseBig5 = FALSE;
	BOOL bGB2312 = FALSE;
	BOOL bShiftJIS = FALSE;
	BOOL bHangul = FALSE;

	lf.lfFaceName[0] = 0;
	lf.lfPitchAndFamily = 0;

	char szLangID[4] = "\0\0\0";
	LCID lcid = GetUserDefaultLCID();
	GetLocaleInfo (lcid, LOCALE_SABBREVLANGNAME, szLangID, 4);

	// Traditional Chinese
	if (lstrcmpi(szLangID, "cht"))
	{
		lf.lfCharSet = CHINESEBIG5_CHARSET;
		EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)&bChineseBig5, 0);
	}

	// Simplified Chinese
	if (lstrcmpi(szLangID, "chs"))
	{
		lf.lfCharSet = GB2312_CHARSET;		
		EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)&bGB2312, 0);
	}

	// Japanese
	if (lstrcmpi(szLangID, "jpn"))
	{
		lf.lfCharSet = SHIFTJIS_CHARSET;	
		EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)&bShiftJIS, 0);
	}

	// Korean
	if (lstrcmpi(szLangID, "kor"))
	{
		lf.lfCharSet = HANGUL_CHARSET;		
		EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)&bHangul, 0);
	}

	ReleaseDC(wnd, hdc);

	int index = wnd.AddString(ALS("System Default"));
	wnd.SetItemData(index, langSystemDefault);
	if (lang == langSystemDefault) wnd.SetCurSel(index);

	// add the available languages
	if (bChineseBig5)
	{
		int index = wnd.AddString(ALS("Traditional Chinese"));
		wnd.SetItemData(index, langTraditionalChinese);
		if (lang == langTraditionalChinese) wnd.SetCurSel(index);
	}
	if (bGB2312)
	{
		int index = wnd.AddString(ALS("Simplified Chinese"));
		wnd.SetItemData(index, langSimplifiedChinese);
		if (lang == langSimplifiedChinese) wnd.SetCurSel(index);
	}
	if (bShiftJIS)
	{
		int index = wnd.AddString(ALS("Japanese"));
		wnd.SetItemData(index, langJapanese);
		if (lang == langJapanese) wnd.SetCurSel(index);
	}
	if (bHangul)
	{
		int index = wnd.AddString(ALS("Korean"));
		wnd.SetItemData(index, langKorean);
		if (lang == langKorean) wnd.SetCurSel(index);
	}

	return TRUE;
}

void CConfigLanguage::GetTranslations()
{
	char alpath[MAX_PATH] = "";
	lstrcpy(alpath, dirPlugin);
	PathAppend(alpath, "Album List\\Language\\*.al");

	HANDLE hFile;
	WIN32_FIND_DATA FindFileData;
	if ((hFile = FindFirstFile(alpath, &FindFileData)) != INVALID_HANDLE_VALUE)
	{
		char lang_ini[MAX_PATH] = "";

		do
		{
			lstrcpy(lang_ini, dirPlugin);
			PathAppend(lang_ini, "Album List\\Language");
			PathAppend(lang_ini, FindFileData.cFileName);
			GetLongPathName(lang_ini, lang_ini, MAX_PATH);

			char lang[MAX_PATH] = "";
			char name[MAX_PATH] = "";
			int codepage = CP_ACP;
			GetPrivateProfileString("Info", "Language", "", lang, MAX_PATH, lang_ini);
			GetPrivateProfileString("Info", "Name",		"", name, MAX_PATH, lang_ini);
			codepage = GetPrivateProfileInt("Info", "Codepage", CP_ACP, lang_ini);
			if (lang[0] != 0)
			{
				LPTRANSLATION pTranslation = new TRANSLATION;
				
				lstrcpy(pTranslation->pathname, lang_ini);
				lstrcpy(pTranslation->language, lang);
				lstrcpy(pTranslation->name, name);
				pTranslation->codepage = codepage;

				m_Translations.Add(pTranslation);
			}
		}
		while (FindNextFile(hFile, &FindFileData) != 0);

		FindClose(hFile);
	}

	m_Translations.Sort();
}

//////////////////////////////////////////////////////////////////
// CLanguageArray class

CLanguageArray::CLanguageArray() : CPtrArray()
{
	m_pCompare = Compare;
}

int CLanguageArray::Compare(const void *elem1, const void *elem2)
{
	LPTRANSLATION pLang1 = *(LPTRANSLATION*)elem1;
	LPTRANSLATION pLang2 = *(LPTRANSLATION*)elem2;

	return lstrcmp(pLang1->language, pLang2->language);
}
