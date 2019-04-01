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

	DWORD bytesIo = 0;
	DWORD dw = 0;

	// This the maximum amount of results we can store
	// if it's anything else than MAX_LGMEMORY_REGIONS * sizeof(MEMORY_BASIC_INFORMATION);
	// the driver will complain about possibly not having enough memory to
	// copy the results
	DWORD max = MAX_LGMEMORY_REGIONS * sizeof(LGMEMORY_REGION);
	auto result = new BYTE[max];

	// This is the request we are going to pass to our driver
	LGGETMEMORYREGION_REQ request = {
		pid
	};

	// Talk to the driver
	if (DeviceIoControl(hDev, IOCTL_LGENUMMEMORYREGIONS, &request, sizeof(request), result,
		max, &bytesIo, nullptr))
	{
		CHAR dumpPath[MAX_PATH];
		sprintf_s(dumpPath, MAX_PATH, "%ls", argv[1]);
		strcat_s(dumpPath, MAX_PATH, ".bin");

		FILE* fp;
		fopen_s(&fp, dumpPath, "wb+");

		BYTE* copy;
		bytesIo -= sizeof(request);
		LGMEMORY_REGION reg;
		int index = 0;
		while (bytesIo > 0)
		{
			memcpy(&reg, result + index * sizeof(reg), sizeof(reg));
			index++;
			bytesIo -= sizeof(reg);

			if (reg.mbi.Type == MEM_IMAGE)
				continue;

			copy = new BYTE[reg.mbi.RegionSize];

			LGCOPYMEMORY_REQ cpyreq
			{
				FALSE,
				pid,
				reg.mbi.BaseAddress,
				copy,
				reg.mbi.RegionSize
			};

			if (DeviceIoControl(hDev, IOCTL_LGCOPYMEMORY, &cpyreq, sizeof(cpyreq), nullptr,
				0, &dw, nullptr))
			{
				wcout << L"Copied " << reg.mbi.RegionSize << L" bytes from " << reg.mbi.BaseAddress << endl;
				fwrite(copy, 1, reg.mbi.RegionSize, fp);
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

	delete[] result;
}
