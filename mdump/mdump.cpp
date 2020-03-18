// mdump.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
#include <SubAuth.h>

/*
Driver specific includes
These must be included ONLY after the basic types have been definied.
types.h includes winnt.h for type definitions and that is not availabe
in console applications hence basic type keywords must be defined
*/
#include "../legwdm/common.h"
#include "../legwdm/types.h"

using namespace std;

DWORD GetProcessId(LPCWSTR ProcessName)
{
	PROCESSENTRY32 pt;
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hsnap, &pt)) {
		do {
			if (!lstrcmpi(pt.szExeFile, ProcessName)) {
				CloseHandle(hsnap);
				return pt.th32ProcessID;
			}
		} while (Process32Next(hsnap, &pt));
	}
	CloseHandle(hsnap);
	return 0;
}

int wmain(int argc, wchar_t** argv)
{
	// Get PID
	const auto pid = GetProcessId(argv[1]);
	if (pid < 1)
	{
		wcout << L"Couldn't get process ID for " << argv[1] << endl;
		return 0;
	}

	const auto hDev = CreateFile(LEGDWM_USERMODE_PATH, GENERIC_ALL, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM,
		nullptr);

	// Exit if we couldn't connect to the driver
	if (hDev == INVALID_HANDLE_VALUE)
	{
		std::wcout << L"Could not open driver" << std::endl;
		return 0;
	}


	FILE* fp = NULL;
	DWORD bytesIo = 0;
	DWORD bytesIo2 = 0;
	DWORD cbMbi = 0;
	BYTE* copy = NULL;
	MEMORY_BASIC_INFORMATION mbi[MAX_LGMEMORY_REGIONS];
	LGGETMEMORYREGION_REQ request;

	request.dwCpId = GetCurrentProcessId();
	request.dwPid = pid;
	request.pcbMbi = &cbMbi;
	request.pMbi = mbi;

	// Talk to the driver
	if (DeviceIoControl(hDev, IOCTL_LGENUMMEMORYREGIONS, &request, sizeof(request), nullptr,
		0, &bytesIo, nullptr))
	{
		CHAR dumpPath[MAX_PATH];
		sprintf_s(dumpPath, MAX_PATH, "%ls", argv[1]);
		strcat_s(dumpPath, MAX_PATH, ".bin");
		fopen_s(&fp, dumpPath, "wb+");

		for (DWORD i = 0; i < cbMbi; i++)
		{
			if (mbi[i].Type == MEM_IMAGE)
				continue;

			copy = new BYTE[mbi[i].RegionSize];
			LGCOPYMEMORY_REQ cpyreq
			{
				FALSE,
				pid,
				mbi[i].BaseAddress,
				copy,
				mbi[i].RegionSize
			};

			if (DeviceIoControl(hDev, IOCTL_LGCOPYMEMORY, &cpyreq, sizeof(cpyreq), nullptr,
				0, &bytesIo2, nullptr))
			{
				wcout << L"Copied " << mbi[i].RegionSize << L" bytes from " << mbi[i].BaseAddress << endl;
				fwrite(copy, 1, mbi[i].RegionSize, fp);
			}
			else
			{
				DWORD error = GetLastError();
				wcout << L"Error code " << "0x" << hex << error << endl;
			}

			delete[] copy;
		}

		fclose(fp);
	}
	else
	{
		DWORD error = GetLastError();
		wcout << L"Error code " << "0x" << hex << error << endl;
	}
}
