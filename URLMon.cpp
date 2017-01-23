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
/*
 * URLMon.CPP
 *
 * Interface glue code to the URLMon
 */
#include "stdafx.h"
#include <urlmon.h>

static HINSTANCE hDllURL = 0L;
static DWORD	 nRef = 0L;

static HRESULT (WINAPI *pURLDownloadToFile)(LPUNKNOWN,LPCSTR,LPCSTR,DWORD,LPBINDSTATUSCALLBACK);

void URLMonFree();
BOOL URLMonLoad()
{
	// check for multiple instances and increment reference counts.
	if (nRef != 0)
	{
		nRef++;
		return TRUE;
	}

	hDllURL = LoadLibrary("URLMON.DLL");
	if (!hDllURL)
	{
		URLMonFree();
		return FALSE;
	}

	pURLDownloadToFile = (HRESULT (WINAPI *)(LPUNKNOWN,LPCSTR,LPCSTR,DWORD,LPBINDSTATUSCALLBACK)) GetProcAddress(hDllURL, "URLDownloadToFileA");

	// increment reference counts
	nRef++;

	return TRUE;
}

void URLMonFree()
{
	// check reference count first
	if (nRef == 0) return;
	if (--nRef != 0) return;

	pURLDownloadToFile	= NULL;

	FreeLibrary(hDllURL);
	hDllURL = NULL;
}

STDAPI URLDownloadToFileA(LPUNKNOWN pCaller, LPCSTR szURL, LPCSTR szFileName, DWORD dwResv, LPBINDSTATUSCALLBACK lpfnCB)
{
	return pURLDownloadToFile ? pURLDownloadToFile(pCaller, szURL, szFileName, dwResv, lpfnCB) : E_FAIL;
}
 