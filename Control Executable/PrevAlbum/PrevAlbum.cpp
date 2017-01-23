// PrevAlbum.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <stdlib.h>
#include "../../alfront.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	HWND hWinampAL = FindWindow("Winamp AL",NULL);
	if (hWinampAL)
	{
		int inc = atol(lpCmdLine);
		if (inc != 0)
		{
			int cur = SendMessage(hWinampAL, WM_AL_IPC, IPC_GETALBUMINDEX_CUR, 0);

			PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYALBUM_CUR, cur - inc);
		}
		else
		{
			PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYPREVALBUM_CUR, 0);
		}
	}

	return 0;
}



