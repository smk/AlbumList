// NextAlbum.cpp : Defines the entry point for the application.
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
		int index = atol(lpCmdLine);
		if (index != 0)
		{
			PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYALBUM1_CUR, index);
		}
	}

	return 0;
}



