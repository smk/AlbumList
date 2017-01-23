// OGG.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <mbstring.h>
#include "OGG.h"
#include "OggInfo.h"

HANDLE g_hModule = NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
			g_hModule = hModule;
			break;

		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			g_hModule = NULL;
			break;
    }
    return TRUE;
}

// This is an example of an exported function.
char * WINAPI GetExtension(int idx)
{
	if (idx == 0)
		return "OGG";

	return NULL;
}

CFileInfo* WINAPI CreateFileInfo(int idx)
{
	if (idx == 0)
		return new COGGInfo;

	return NULL;
}

void Install()
{
	char ini_file[MAX_PATH], *p;
	GetModuleFileName((HINSTANCE)g_hModule, ini_file, sizeof(ini_file));
	if (p = (char *)_mbsrchr((unsigned char *)ini_file,'\\'))
	{
		*p = 0;
		if (p = (char *)_mbsrchr((unsigned char *)ini_file,'\\'))
		{
			*p = 0;
			if (p = (char *)_mbsrchr((unsigned char *)ini_file,'\\'))
			{
				*p = 0;
			}
			else
				p = ini_file;
		}
		else
			p = ini_file;
	}
	else
		p = ini_file;

	lstrcat(ini_file, "\\plugin.ini");

	char al_extlist[MAX_PATH] = "MP3, MPC, MP+, MPP, WMA, APE, MAC";

	GetPrivateProfileString("Album List", "al_extlist", al_extlist, al_extlist, MAX_PATH, ini_file);

	char extlist[MAX_PATH] = "";
	lstrcpyn(extlist, al_extlist, MAX_PATH);
	_strlwr(extlist);

	if (strstr(extlist, "ogg") == NULL)
	{
		lstrcat(al_extlist, ", OGG");
	
		WritePrivateProfileString("Album List", "al_extlist", al_extlist, ini_file);
	}
}
