#ifndef __CSTRING_H__
#define __CSTRING_H__

#include <stdio.h>

class CString
{
public:
// Constructors

	// constructs empty CString
	CString()
	{
		lstrcpy(m_pchData, "");
	}
	// copy constructor
	CString(const CString& stringSrc)
	{
		lstrcpy(m_pchData, stringSrc.m_pchData);
	}
	// from an ANSI string (converts to TCHAR)
	CString(LPCSTR lpsz)
	{
		lstrcpy(m_pchData, lpsz);
	}

// Attributes & Operations

	// get data length
	int GetLength() const
	{
		return lstrlen(m_pchData);
	}
	// TRUE if zero length
	BOOL IsEmpty() const
	{
		return (lstrlen(m_pchData) == 0);
	}
	// clear contents to empty
	void Empty()
	{
		lstrcpy(m_pchData, "");
	}

	// return pointer to const string
	operator LPCTSTR() const
	{
		return m_pchData;
	}

	void Format( LPCTSTR lpszFormat, ... )
	{
		va_list		args;
		va_start	(args, lpszFormat);

		_vsnprintf(m_pchData, MAX_PATH, lpszFormat, args);
	}

	LPTSTR GetBuffer(int nMinBufLength)
	{
		return m_pchData;
	}

	void ReleaseBuffer(int nNewLength = -1)
	{
	}

	// overloaded assignment

	// ref-counted copy from another CString
	const CString& operator=(const CString& stringSrc)
	{
		lstrcpy(m_pchData, stringSrc.m_pchData);
		return *this;
	}

	const CString& operator+=(const CString& stringSrc)
	{
		lstrcat(m_pchData, stringSrc.m_pchData);
		return *this;
	}

	// copy string content from ANSI string (converts to TCHAR)
	const CString& operator=(LPCSTR lpsz)
	{
		lstrcpy(m_pchData, lpsz);
		return *this;
	}

	// load from string resource
	BOOL LoadString(UINT nID)
	{
//		::LoadString(plugin.hDllInstance, nID, m_pchData, MAX_PATH);
		return FALSE;
	}

protected:
	TCHAR m_pchData[MAX_PATH];   // pointer to ref counted string data
};

#endif /* __CSTRING_H__ */
