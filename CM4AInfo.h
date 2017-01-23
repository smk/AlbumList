#ifndef _CM4AINFO_H_
#define _CM4AINFO_H_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <objbase.h>
#include "CWAExtendedInfo.h"

class CM4AInfo : public CWAExtendedInfo
{
	virtual bool getCoverImage(LPBYTE *input, DWORD &dwSize);
};

bool CM4AInfo::getCoverImage(LPBYTE *input, DWORD &dwSize)
{
	if (input == NULL) return false;

	HANDLE fp = NULL;

	// initialize variables
	*input = NULL;
	dwSize = 0;

	ENSURE
	{
		DWORD nBytesRead;

		if ((fp = CreateFileW(m_FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE) FAIL;

		DWORD dwCurrentFilePosition = 0;
		DWORD dwEnd = GetFileSize (fp, NULL);

		// try maximum 100 atoms
		int i = 100;

		bool b_moov = false;
		bool b_udta = false;
		bool b_meta = false;
		bool b_ilst = false;
		bool b_covr = false;

		BYTE buf[16];

		do
		{
			// get atom size first
			if (!ReadFile(fp, buf, 4, &nBytesRead, NULL)) FAIL;

			DWORD dwAtomSize = ((DWORD)buf[0] << 24) + ((DWORD)buf[1] << 16) + ((DWORD)buf[2] << 8) + (DWORD)buf[3];

			// get atom type
			buf[4] = 0;
			if (!ReadFile(fp, buf, 4, &nBytesRead, NULL)) FAIL;

			DbgPrint("Atom - %08x (%04x) - %s\n", dwCurrentFilePosition, dwAtomSize, buf);

			if (memcmp(buf, "moov", 4) == 0)
			{
				dwEnd = dwCurrentFilePosition + dwAtomSize - 8;

				b_moov = true;
			}
			else if (b_moov && (memcmp(buf, "udta", 4) == 0))
			{
				b_udta = true;
			}
			else if (b_udta && (memcmp(buf, "meta", 4) == 0))
			{
				// skip four bytes (flags?)
				SetFilePointer(fp, 4, NULL, FILE_CURRENT);

				b_meta = true;
			}
			else if (b_meta && (memcmp(buf, "ilst", 4) == 0))
			{
				b_ilst = true;
			}
			else if (b_ilst && (memcmp(buf, "covr", 4) == 0))
			{
				b_covr = true;
			}
			else if (b_covr && (memcmp(buf, "data", 4) == 0))
			{
				// skip eight bytes (flags?)
				SetFilePointer(fp, 8, NULL, FILE_CURRENT);

				dwSize = dwAtomSize - 8 /*size + type*/ - 8 /*flags*/;
				*input = new BYTE [dwSize];
				if (!ReadFile(fp, *input, dwSize, &nBytesRead, NULL))
				{
					delete[] *input;
					*input = NULL;
				}
				break;
			}
			else
			{
				SetFilePointer(fp, dwAtomSize-8, NULL, FILE_CURRENT);
			}

			// update current position
			dwCurrentFilePosition = SetFilePointer(fp, 0, NULL, FILE_CURRENT);
		}
		while ((dwCurrentFilePosition < dwEnd) && i--);

		if (*input == NULL) FAIL;

		CloseHandle(fp);

		return true;
	}
	END_ENSURE;

	if (fp) CloseHandle(fp);
	
	return false;
}

#endif /* _CM4AINFO_H_ */
