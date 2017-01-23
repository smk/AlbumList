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
#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <commctrl.h>

#ifndef ENSURE
#define ENSURE			do
#define FAIL			break
#define END_ENSURE		while (FALSE)
#endif

extern BOOL bWin9x;
extern BOOL bWinNT;

//////////////////////////////////////////////////////////////////
// CRect class

class CRect : public RECT
{
public:
	int Width()		{ return right - left; }
	int Height()	{ return bottom - top; }

	void operator=(const RECT& srcRect) { ::CopyRect(this, &srcRect); }
};

//////////////////////////////////////////////////////////////////
// CWnd class

class CWnd
{
public:
	CWnd();
	CWnd(HWND hwnd);
	virtual ~CWnd();

	HWND m_hWnd;

public:
	operator HWND() const
	{
		return ::IsWindow(m_hWnd) ? m_hWnd : NULL;
	}

	HWND GetSafeHwnd	();
	BOOL IsWindow		();
	CWnd GetParent		();
	BOOL ModifyStyle	(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
	BOOL ModifyStyleEx	(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);

	// dialog related functions
	CWnd GetDlgItem		(int nIDDlgItem);
	int  CheckDlgButton	(int nIDButton, UINT nCheck);
	UINT IsDlgButtonChecked	(int nIDButton);
	void SetDlgItemText	(int nID, LPCTSTR lpszString, int codepage = -1);
	void SetDlgItemText	(int nID, LPCWSTR lpszString, int codepage = -1);
	int  GetDlgItemText	(int nID, LPTSTR lpStr, int nMaxCount, int codepage = -1);
	int  GetDlgItemText	(int nID, LPWSTR lpStr, int nMaxCount, int codepage = -1);
	int  SetDlgItemInt	(int nID, UINT nValue, BOOL bSigned = TRUE);
	int  GetDlgItemInt	(int nID, BOOL *lpTrans = NULL, BOOL bSigned = TRUE);
	LRESULT SendDlgItemMessage(int nID, UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	BOOL EndDialog		(int nResult);

	// window related functions
	int  EnableWindow	(BOOL bEnable = TRUE);
	BOOL ShowWindow		(int nCmdShow);
	void SetWindowText	(LPCTSTR lpszString);
	void SetWindowText	(LPCWSTR lpszString);
	void GetWindowText	(LPTSTR lpszStringBuf, int nMaxCount);
	void GetWindowText	(LPWSTR lpszStringBuf, int nMaxCount);
	BOOL DestroyWindow	();

	// position related functions
	void ScreenToClient	(LPPOINT lpPoint);
	void ScreenToClient	(LPRECT lpRect);
	void GetClientRect	(LPRECT lpRect);
	void GetWindowRect	(LPRECT lpRect);
	BOOL SetWindowPos	(CWnd wndInsertAfter, int x, int y, int cx, int cy, UINT nFlags);
	HDWP DeferWindowPos	(HDWP hWinPosInfo, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags);
	void CenterWindow	(CWnd *pAlternateOwner = NULL);

	BOOL UpdateWindow	();

	LRESULT SendMessage	(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT PostMessage	(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

	BOOL SubclassWindow	(CWnd wnd);
	HWND UnsubclassWindow();

protected:
	virtual BOOL OnInitDialog	();
	virtual int  OnCreate		(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnPaint		(HDC hdc);
	virtual BOOL OnEraseBkgnd	(HDC hdc);
	virtual BOOL OnSize			(UINT nType, int cx, int cy);
	virtual BOOL OnCommand		(WORD wNotifyCode, WORD wID, CWnd wndCtl);
	virtual void OnOK			();
	virtual void OnCancel		();
	virtual BOOL OnSysCommand	(UINT uCmdType, short xPos, short yPos);
	virtual BOOL OnNotify		(UINT nID, NMHDR *pnmh, LRESULT *pResult);
	virtual BOOL OnDisplayChange(int cBitsPerPixel, int cx, int cy);
	virtual BOOL OnDestroy		();
	virtual BOOL OnEndSession	(BOOL bEnding);
	virtual BOOL OnQueryEndSession();
	virtual BOOL OnDeviceChange	(UINT nEventType, DWORD dwData);
	virtual BOOL OnClose		();
	virtual BOOL OnShowWindow	(BOOL bShow, UINT nStatus);
	virtual BOOL OnMouseMove	(UINT nFlags, POINT point);
	virtual BOOL OnLButtonDown	(UINT nFlags, POINT point);
	virtual BOOL OnNcMouseMove	(UINT nHitTest, POINT point);
	virtual BOOL OnKeyDown		(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL OnChar			(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK SubclassWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void TranslateStrings	();
	
protected:
	LRESULT (CALLBACK *pWindowProc)(HWND, UINT, WPARAM, LPARAM);

	BOOL ModifyStyle(int nStyleOffset, DWORD dwRemove, DWORD dwAdd, UINT nFlags);
};

//////////////////////////////////////////////////////////////////
// CDialog class

class CDialog : public CWnd
{
public:
	CDialog();
	CDialog(UINT nIDTemplate);

	BOOL Create					(CWnd* pParentWnd = NULL, BOOL bUnicode = TRUE);
	BOOL Create					(UINT nIDTemplate, CWnd* pParentWnd = NULL, BOOL bUnicode = TRUE);
	BOOL DoModal				(CWnd* pParentWnd = NULL, BOOL bUnicode = TRUE);
	BOOL DoModal				(UINT nIDTemplate, CWnd* pParentWnd = NULL, BOOL bUnicode = TRUE);

protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	UINT m_nIDTemplate;
};

//////////////////////////////////////////////////////////////////
// CComboBox class

class CComboBox : public CWnd
{
public:
	CComboBox();
	CComboBox(HWND hwnd);

	int   AddString				(LPCTSTR lpszString, int codepage = -1);
	int   GetCount				();
	int   SetCurSel				(int nSelect);
	int   GetCurSel				();
	int   SetItemData			(int nIndex, DWORD dwItemData);
	DWORD GetItemData			(int nIndex);
	int   GetLBText				(int nIndex, LPTSTR lpszText);
	int   GetLBText				(int nIndex, LPWSTR lpszText);
};

//////////////////////////////////////////////////////////////////
// CTabCtrl class

class CTabCtrl : public CWnd
{
public:
	CTabCtrl();
	CTabCtrl(HWND hwnd);

	BOOL InsertItem				(int nItem, TCITEM* pTabCtrlItem);
	int  SetCurSel				(int nItem);
	int  GetCurSel				();
	int  GetItemCount			();
	BOOL GetItem				(int nItem, TCITEM* pTabCtrlItem);
	void AdjustRect				(BOOL bLarger, LPRECT lpRect);
};

//////////////////////////////////////////////////////////////////
// CListBox class

class CListBox : public CWnd
{
public:
	CListBox();
	CListBox(HWND hwnd);

	int   AddString				(LPCTSTR lpszString, int codepage = -1);
	int   AddString				(LPCWSTR lpszString, int codepage = -1);
	int   GetCount				();
	int   SetCurSel				(int nSelect);
	int   GetCurSel				();
	int   SetItemData			(int nIndex, DWORD dwItemData);
	DWORD GetItemData			(int nIndex);
	void  ResetContent			();
	int   GetText				(int nIndex, LPTSTR lpszBuffer);
	int   GetText				(int nIndex, LPWSTR lpszBuffer);
	int   GetTextLen			(int nIndex);
};

//////////////////////////////////////////////////////////////////
// CListCtrl class

class CListCtrl : public CWnd
{
public:
	CListCtrl();
	CListCtrl(HWND hwnd);

	int InsertItem				(const LVITEM* pItem);
};

//////////////////////////////////////////////////////////////////
// CMenu class

class CMenu
{
public:
	CMenu();
	CMenu(HMENU hMenu);
	virtual ~CMenu();

	HMENU m_hMenu;

private:
	BOOL  m_bUnicode;

public:
	operator HMENU() const
	{
		return m_hMenu;
	}
	BOOL CreateMenu			();
	BOOL CreatePopupMenu	();
	BOOL DestroyMenu		();
	BOOL DeleteMenu			(UINT nPosition, UINT nFlags);
	BOOL TrackPopupMenu		(UINT nFlags, int x, int y, CWnd* pWnd, LPCRECT lpRect = NULL);
	BOOL AppendMenu			(UINT nFlags, UINT nIDNewItem = 0, LPCTSTR lpszNewItem = NULL);
	BOOL AppendMenu			(UINT nFlags, UINT nIDNewItem = 0, LPCWSTR lpszNewItem = NULL);
	UINT CheckMenuItem		(UINT nIDCheckItem, UINT nCheck);
	UINT EnableMenuItem		(UINT nIDEnableItem, UINT nEnable);
	BOOL CheckMenuRadioItem	(UINT nIDFirst, UINT nIDLast, UINT nIDItem, UINT nFlags);
	BOOL SetDefaultItem		(UINT uItem, UINT fByPos = FALSE);
	BOOL IsUnicode			() { return m_bUnicode; }
};

//////////////////////////////////////////////////////////////////
// CCriticalSection class

class CCriticalSection
{
public:
	CCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
	{
		EnterCriticalSection(m_lpCriticalSection = lpCriticalSection);
	}

	~CCriticalSection()
	{
		LeaveCriticalSection(m_lpCriticalSection);
	}

private:
	LPCRITICAL_SECTION m_lpCriticalSection;
};

//////////////////////////////////////////////////////////////////
// CPtrArray class

class CPtrArray
{
public:
	CPtrArray();
	virtual ~CPtrArray();

public:
	virtual int GetSize();
	virtual void SetSize(int nNewSize, int nGrowBy = -1);

	virtual int Add(LPVOID pItem);
	virtual void SetAt(int nIndex, LPVOID pItem);
	virtual LPVOID GetAt(int nIndex);
	virtual void Swap(int from, int to);

	virtual void InsertAt(int nIndex, LPVOID pItem);
	virtual void RemoveAt(int nIndex, int nCount = 1);

	virtual void RemoveAll();

	virtual LPVOID *GetData();

	virtual void Sort();

	LPVOID operator[](int nIndex);

protected:
	int m_nAllocated;
	int m_nGrowBy;
	LPVOID *m_pData;

	int m_nSize;
	CRITICAL_SECTION m_csMutex;

	int (*m_pCompare)(const void *elem1, const void *elem2);
};

//////////////////////////////////////////////////////////////////
// CDWordArray class

class CDWordArray : public CPtrArray
{
public:
	CDWordArray() : CPtrArray()	{}
	virtual ~CDWordArray()		{}

public:
	int Add(DWORD dwItem)					{ return CPtrArray::Add((LPVOID)dwItem);	}
	void SetAt(int nIndex, DWORD dwItem)	{ CPtrArray::SetAt(nIndex, (LPVOID)dwItem);	}

	void InsertAt(int nIndex, DWORD dwItem)	{ CPtrArray::InsertAt(nIndex, (LPVOID)dwItem);	}

	DWORD operator[](int nIndex)			{ return (DWORD)CPtrArray::GetAt(nIndex);	}
};

//////////////////////////////////////////////////////////////////
// CStringArray class

class CStringArray : public CPtrArray
{
public:
	CStringArray();
	virtual ~CStringArray();

public:
	int Add(LPCTSTR szItem);
	void SetAt(int nIndex, LPCTSTR szItem);

	void InsertAt(int nIndex, LPCTSTR szItem);
	void RemoveAt(int nIndex, int nCount = 1);

	void RemoveAll();

	LPCTSTR operator[](int nIndex);

private:
	static int CStringArray::Compare(const void *elem1, const void *elem2);
};

//////////////////////////////////////////////////////////////////
// CStringArrayW class

class CStringArrayW : public CPtrArray
{
public:
	CStringArrayW();
	virtual ~CStringArrayW();

public:
	int Add(LPCWSTR szItem);
	void SetAt(int nIndex, LPCWSTR szItem);

	void InsertAt(int nIndex, LPCWSTR szItem);
	void RemoveAt(int nIndex, int nCount = 1);

	void RemoveAll();

	LPCWSTR operator[](int nIndex);

private:
	static int CStringArrayW::Compare(const void *elem1, const void *elem2);
};

//////////////////////////////////////////////////////////////////
// CTokens class

class CTokens : public CPtrArray
{
public:
	CTokens(LPCTSTR str, LPCTSTR seps);
	virtual ~CTokens();

public:
	LPCTSTR operator[](int nIndex);

	void Parse(LPCTSTR str, LPCTSTR seps);

private:
	char string[MAX_PATH];
};

//////////////////////////////////////////////////////////////////
// CTokensW class

class CTokensW : public CPtrArray
{
public:
	CTokensW(LPCWSTR str, LPCWSTR seps);
	virtual ~CTokensW();

public:
	LPCWSTR operator[](int nIndex);

	void Parse(LPCWSTR str, LPCWSTR seps);

private:
	WCHAR string[MAX_PATH];
};

//////////////////////////////////////////////////////////////////
// CNamingTokens class

class CNamingTokens : public CPtrArray
{
public:
	CNamingTokens(LPCTSTR str);
	virtual ~CNamingTokens();

public:
	LPCTSTR operator[](int nIndex);

	void Parse(LPCTSTR str);

private:
	char string[MAX_PATH];
};

//////////////////////////////////////////////////////////////////
// CNamingTokensW class

class CNamingTokensW : public CPtrArray
{
public:
	CNamingTokensW(LPCWSTR str);
	virtual ~CNamingTokensW();

public:
	LPCWSTR operator[](int nIndex);

	void Parse(LPCWSTR str);

private:
	WCHAR string[MAX_PATH];
};

//////////////////////////////////////////////////////////////////
// CTextFile class

class CTextFile
{
public:
	CTextFile(LPCTSTR lpszFilename, UINT nOpenFlags);
	virtual ~CTextFile();

	void WriteString(LPCTSTR lpsz);
	LPTSTR ReadString(LPTSTR lpsz, UINT nMax);

	UINT Read(void* lpBuf, UINT nCount);
	void Write(const void* lpBuf, UINT nCount);
	void Flush();
	void Close();

private:
	HANDLE	m_hFile;
	FILE   *m_pStream;
};

//////////////////////////////////////////////////////////////////
// CMapStringToString class

#ifndef POSITION
struct __POSITION { };
typedef __POSITION* POSITION;
#endif

#define BEFORE_START_POSITION ((POSITION)-1L)

class CMapStringToString
{
public:
	CMapStringToString();
	virtual ~CMapStringToString();

	int	GetCount() const;
	BOOL IsEmpty() const;
	void SetAt(LPCTSTR key, LPCTSTR newValue);
	BOOL Lookup(LPCTSTR key, LPTSTR &rValue) const;
	BOOL RemoveKey(LPCTSTR key);
	void RemoveAll();

	UINT HashKey(LPCTSTR key) const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, LPTSTR &rKey, LPTSTR &rValue) const;

	void Dump();

protected:
	// Association
	class CAssoc
	{
	public:
		CAssoc(LPCTSTR k, LPCTSTR v);
		virtual ~CAssoc();

		CAssoc* pNext;
		char *key;
		char *value;
	};

protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;

	CAssoc* GetAssocAt(LPCTSTR key, UINT &nHash) const;
};

//////////////////////////////////////////////////////////////////
// CMapStringToStringW class

class CMapStringToStringW
{
public:
	CMapStringToStringW();
	virtual ~CMapStringToStringW();

	int	GetCount() const;
	BOOL IsEmpty() const;
	void SetAt(LPCWSTR key, LPCWSTR newValue);
	BOOL Lookup(LPCWSTR key, LPWSTR &rValue) const;
	BOOL RemoveKey(LPCWSTR key);
	void RemoveAll();

	UINT HashKey(LPCWSTR key) const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, LPWSTR &rKey, LPWSTR &rValue) const;

	void Dump();

protected:
	// Association
	class CAssoc
	{
	public:
		CAssoc(LPCWSTR k, LPCWSTR v);
		virtual ~CAssoc();

		CAssoc* pNext;
		WCHAR *key;
		WCHAR *value;
	};

protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;

	CAssoc* GetAssocAt(LPCWSTR key, UINT &nHash) const;
};

//////////////////////////////////////////////////////////////////
// CTemplateFile class

class CTemplateFile
{
public:
	CTemplateFile();
	virtual ~CTemplateFile();

	void AddReplace(LPCTSTR lpsz1, LPCTSTR lpsz2);
	void AddReplace(LPCTSTR lpsz1, int index, LPCTSTR lpsz2);
	void AddReplace(LPCTSTR lpsz, int nNum);
	void AddReplace(LPCTSTR lpsz, int index, int nNum);
	void AddReplace(LPCTSTR lpsz, COLORREF cr);

	BOOL Convert(LPCTSTR lpszOutput, LPCTSTR lpszInput);

private:
	CMapStringToString m_ReplaceMap;
};

//////////////////////////////////////////////////////////////////
// CChangeDir class

class CChangeDir
{
public:
	CChangeDir(LPCTSTR dir)
	{
//		GetCurrentDirectoryW(MAX_PATH, oldPath);
//		SetCurrentDirectory(dir);
	}

	CChangeDir(LPCWSTR dir)
	{
//		GetCurrentDirectoryW(MAX_PATH, oldPath);
//		SetCurrentDirectoryW(dir);
	}

	~CChangeDir()
	{
//		SetCurrentDirectoryW(oldPath);
	}

private:
	wchar_t oldPath[MAX_PATH];
};

//////////////////////////////////////////////////////////////////
// utility functions
void GetVersionInformation(HINSTANCE hInstance, int &major, int &minor, int &build);
HINSTANCE GotoURL(LPCTSTR url, int showcmd);
BOOL Win98GetFileAttributesEx(LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation);
BOOL Win98GetFileAttributesEx(LPCWSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation);
HRESULT STDAPICALLTYPE WinXPEnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);
LPCTSTR FormatTime(LPTSTR str, int nTime);
void DbgPrint(LPSTR format, ...);
void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName);
LPCTSTR lstrstri(LPCTSTR str, LPCTSTR substr);

LPCTSTR ALS(LPCTSTR str1, LPCTSTR str2 = NULL);
LPCWSTR ALSW(LPCTSTR str1, LPCTSTR str2 = NULL);
void UpdateDialogStrings(CWnd wnd);

HBITMAP LoadPictureFile(LPCTSTR szFile, SIZE size);
HBITMAP LoadPictureFile(LPCWSTR szFile, SIZE size);
HBITMAP LoadPictureFile2(LPCTSTR szFile, SIZE size);
HBITMAP LoadPictureFile2(LPCWSTR szFile, SIZE size);

BOOL WriteTextToFile(HANDLE hFile, LPCTSTR str, BOOL bUTF8, int cp);
BOOL WriteTextToFile(HANDLE hFile, LPCWSTR str, BOOL bUTF8, int cp);

BOOL WritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCWSTR lpString, LPCTSTR lpFileName);
DWORD GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName);

BOOL BrowseForFolder(HWND hWnd, LPCWSTR title, LPWSTR path, int nSize);

int CompareStr(LPCWSTR str1, LPCWSTR str2);

#endif /* __UTIL_H__ */
