// PlayAlbumName.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <stdlib.h>
#include "../../alfront.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	if (lstrlen(lpCmdLine) == 0) return 0;

	HWND hWinampAL = FindWindow("Winamp AL",NULL);
	if (hWinampAL)
	{
		int size = lstrlen(lpCmdLine)+1;

		// Get Winamp process ID
		DWORD dwProcessID = 0;
		GetWindowThreadProcessId(hWinampAL, &dwProcessID);

		// Open Winamp for write access
		HANDLE hWinamp = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, dwProcessID);

		// Allocated memory in Winamp process
		LPVOID pMem = VirtualAllocEx(hWinamp, NULL, size, MEM_COMMIT, PAGE_READWRITE);

		// Write to allocated memory
		WriteProcessMemory(hWinamp, pMem, lpCmdLine, size, NULL);

		SendMessage(hWinampAL, WM_AL_IPC, IPC_PLAYALBUMNAME_CUR, (LPARAM)pMem);

		// Free allocated memory in ATIMMC Process
		VirtualFreeEx(hWinamp, pMem, size, MEM_DECOMMIT);

		// Close handle to ATIMMC Process
		CloseHandle(hWinamp);
	}

	return 0;
}



