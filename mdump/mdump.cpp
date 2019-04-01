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

size_t to_narrow(const wchar_t * src, char * dest, size_t dest_len) {
	size_t i;
	wchar_t code;

	i = 0;

	while (src[i] != '\0' && i < (dest_len - 1)) {
		code = src[i];
		if (code < 128)
			dest[i] = char(code);
		else {
			dest[i] = '?';
			if (code >= 0xD800 && code <= 0xD8FF)
				// lead surrogate, skip the next code unit, which is the trail
				i++;
		}
		i++;
	}

	dest[i] = '\0';

	return i - 1;

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

	// Set up vars for communication
	DWORD bytesIo = 0;
	DWORD max = MAX_LGMEMORY_REGIONS * sizeof(MEMORY_BASIC_INFORMATION);
	auto result = new BYTE[max];

	LGGETMEMORYREGION_REQ request = {
		pid
	};

	// Try to connect to the driver
	const auto hDev = CreateFile(LEGDWM_USERMODE_PATH, GENERIC_ALL, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM,
		nullptr);

	// Exit if we couldn't connect to the driver
	if (hDev == INVALID_HANDLE_VALUE)
	{
		wcout << L"Could not open driver" << std::endl;
		return 0;
	}


	CHAR sz[MAX_PATH];
	to_narrow(argv[1], sz, MAX_PATH);
	strcat_s(sz, MAX_PATH, ".bin");
	FILE* fp;
	fopen_s(&fp, sz, "wb+");
	

	// Talk to the driver
	if (DeviceIoControl(hDev, IOCTL_LGENUMMEMORYREGIONS, &request, sizeof(request), result,
		max, &bytesIo, nullptr))
	{
		BYTE* copy;
		bytesIo -= sizeof(request);
		wcout << L"OK " << "IO = " << bytesIo << endl;
		wcout << "mbi * " << bytesIo / sizeof(MEMORY_BASIC_INFORMATION) << endl;

		MEMORY_BASIC_INFORMATION mbi;
		int index = 0;
		while (bytesIo > 0)
		{
			memcpy(&mbi, result + index * sizeof(mbi), sizeof(mbi));
			index++;
			bytesIo -= sizeof(mbi);

			copy = new BYTE[mbi.RegionSize];


			LGCOPYMEMORY_REQ cpyreq = {
		FALSE,						// specifies that we are READING from the memory
		pid,						// target process ID
		mbi.BaseAddress,			// target memory address
		copy,						// copy bytes begining from this address
		mbi.RegionSize		// the count of the bytes we will write
			};

			if (DeviceIoControl(hDev, IOCTL_LGCOPYMEMORY, &cpyreq, sizeof(cpyreq), nullptr,
				0, &bytesIo, nullptr))
			{
				//wcout << L"Region written!" << endl;
				fwrite(copy, 1, mbi.RegionSize, fp);
			}

			delete[] copy;
		}

		//DeviceIoControl(hDev, IOCTL_LGCOPYMEMORY, )

		wcout << "Base address = " << mbi.BaseAddress << " RegionSize = " << mbi.RegionSize << endl;
	}
	else
	{
		DWORD error = GetLastError();
		wcout << L"Error code " << "0x" << hex << error << endl;
	}

	delete[] result;
	fclose(fp);
}
