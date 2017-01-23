// PlayRandomAlbum.cpp : Defines the entry point for the application.
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
		PostMessage(hWinampAL, WM_AL_IPC, IPC_PLAYRANDOMALBUM_CUR, 0);
	}

	return 0;
}



