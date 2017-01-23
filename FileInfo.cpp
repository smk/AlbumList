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
#include <Shlwapi.h>

#include "Gen_m3a.h"
#include "Util.h"

#include "FileInfo.h"
#include "CMP3Info.h"
#include "CMPCInfo.h"
#include "CSPCInfo.h"
#include "CPSFInfo.h"
#include "CWMAInfo.h"
#include "CAPEInfo.h"
#include "CWAVInfo.h"
#include "CM4AInfo.h"
#include "CWAExtendedInfo.h"

static CPtrArray g_FileReaders;				// array of file readers
static UINT nRef = 0;

typedef struct _TAGFILEREADER
{
	HMODULE		hModule;			// instance handle
	int			nIndex;
	char		exts[MAX_PATH];		// extensions (separated by "space")
	char	   *(WINAPI *pGetExt)(int);	// create function
	CFileInfo  *(WINAPI *pCreate)(int);	// create function

}	FILEREADER, *LPFILEREADER;

void LoadExternalFileReader()
{
	if (nRef++ > 0) return;

	char path[MAX_PATH] = "";
	lstrcpy(path, dirPlugin);
	PathAppend(path, "Album List\\Plugins\\*.afp");

	HANDLE hFile;
	WIN32_FIND_DATA FindFileData;
	if ((hFile = FindFirstFile(path, &FindFileData)) != INVALID_HANDLE_VALUE)
	{
		char reader[MAX_PATH] = "";

		do
		{
			lstrcpy(reader, dirPlugin);
			PathAppend(reader, "Album List\\Plugins");
			PathAppend(reader, FindFileData.cFileName);

			AddFileReader(reader);
		}
		while (FindNextFile(hFile, &FindFileData) != 0);

		FindClose(hFile);
	}
}

void UnloadExternalFileReader()
{
	if (--nRef > 0) return;

	int size = g_FileReaders.GetSize();
	for (int i=0; i<size; i++)
	{
		LPFILEREADER pFileReader = (LPFILEREADER)g_FileReaders[i];

		if (pFileReader->nIndex == 0)
		{
			FreeLibrary(pFileReader->hModule);
		}

		delete pFileReader;
	}
	g_FileReaders.RemoveAll();
}

void AddFileReader(LPCTSTR reader)
{
	HMODULE hModule = NULL;
	char	   *(WINAPI *pGetExt)(int);
	CFileInfo  *(WINAPI *pCreate)(int);

	if (hModule = LoadLibrary(reader))
	{
		pGetExt = (char*     (WINAPI *)(int))GetProcAddress(hModule, "GetExtension");
		pCreate = (CFileInfo*(WINAPI *)(int))GetProcAddress(hModule, "CreateFileInfo");

		if (pGetExt && pCreate)
		{
			char *ptr = NULL;
			int i=0;

			while (pGetExt(i))
			{
				LPFILEREADER pFileReader = new FILEREADER;
				memset(pFileReader, 0, sizeof(FILEREADER));

				lstrcpyn(pFileReader->exts, pGetExt(i), MAX_PATH);
				pFileReader->nIndex = i;
				pFileReader->hModule = hModule;
				pFileReader->pCreate = pCreate;
				pFileReader->pGetExt = pGetExt;
				g_FileReaders.Add(pFileReader);

				i++;
			}
			if (i == 0) FreeLibrary(hModule);
		}
		else
		{
			FreeLibrary(hModule);
		}
	}
}

CFileInfo *CreateFileInfo(LPCTSTR szFile)
{
	return CreateFileInfo(AutoWide(szFile));
}

CFileInfo *CreateFileInfo(LPCWSTR szFile)
{
	int len = wcslen(szFile);
	if (len < 5) return NULL;

	if (wndWinampAL.GetSetting(settingLogFile))
	{
		FILE *fp = fopen("c:\\gen_m3a.log", "a+t");
		if (fp)
		{
			fputs(AutoChar(szFile), fp);
			fputs("\n", fp);
			fclose(fp);
		}
	}

	if (wndWinampAL.GetSetting(settingBuiltInFileInfo))
	{
		// try external file info readers first
		// so others can override the built-in readers
		int size = g_FileReaders.GetSize();
		for (int i=0; i<size; i++)
		{
			LPFILEREADER pFileReader = (LPFILEREADER)g_FileReaders[i];

			CTokens extensions(pFileReader->exts, " ,.;");
			int size = extensions.GetSize();
			for (int i=0; i<size; i++)
			{
				int extlen = lstrlen(extensions[i]);

				if (_wcsicmp(szFile + len - extlen, AutoWide(extensions[i])) == 0)
					return pFileReader->pCreate(pFileReader->nIndex);
			}
		}
		
		// try the built-in MP3 reader
		if (_wcsicmp(szFile + len - 3, L"MP3") == 0)
			return new CMP3Info;

		// try the built-in MPC reader
		if ((_wcsicmp(szFile + len - 3, L"MPC") == 0) ||
			(_wcsicmp(szFile + len - 3, L"MP+") == 0) ||
			(_wcsicmp(szFile + len - 3, L"MPP") == 0))
			return new CMPCInfo;

		// try the built-in SPC reader
		if (_wcsicmp(szFile + len - 3, L"SPC") == 0)
			return new CSPCInfo;

		// try the built-in WMA reader
		if (_wcsicmp(szFile + len - 3, L"WMA") == 0)
			return new CWMAInfo;

		// try the built-in APE reader
		if ((_wcsicmp(szFile + len - 3, L"APE") == 0) ||
			(_wcsicmp(szFile + len - 3, L"MAC") == 0))
			return new CAPEInfo;

		// try the built-in WAV reader
		if (_wcsicmp(szFile + len - 3, L"WAV") == 0)
			return new CWAVInfo;

		// try the built-in PSF reader
		if ((_wcsicmp(szFile + len - 3, L"PSF") == 0) ||
			(_wcsicmp(szFile + len - 7, L"MINIPSF") == 0))
			return new CPSFInfo;

		// try the built-in M4A reader
		if (_wcsicmp(szFile + len - 3, L"M4A") == 0)
			return new CM4AInfo;
	}

	if (wndWinampAL.GetSetting(settingWinampFileInfo))
	{
		return new CWAExtendedInfo;
	}

	return NULL;
}

CFileInfo *CreateWAExtendedInfo()
{
	return new CWAExtendedInfo;
}