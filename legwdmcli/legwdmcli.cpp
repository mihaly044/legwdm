// legwdmcli.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

void test_memory_write(HANDLE hDev, DWORD pid)
{
	wcout << __FUNCTIONW__ << "():" << endl;

	// This variable holds the value which we will tamper with from inside our driver
	DWORD value = 123;

	// Print the initial value of our target variable
	wcout << "The initial value was " << value << endl;

	// We are going to replace the value of the previous variable with this
	DWORD replace = 666;

	// This replace represents how many bytes were written to/read from our driver
	DWORD bytesIo = 0;

	// This is the request we are going to pass to our driver
	LGCOPYMEMORY_REQ request = {
		TRUE,						// specifies that we are WRITING to the memory
		pid,						// target process ID
		&value,						// target memory address
		&replace,					// copy bytes begining from this address
		sizeof(value)				// the count of the bytes we will write
	};

	// Talk to the driver
	if (DeviceIoControl(hDev, IOCTL_LGCOPYMEMORY, &request, sizeof(request), nullptr,
		0, &bytesIo, nullptr))
	{
		wcout << "The value is " << value << endl << endl;
	}
	else
	{
		wcout << "Failed talking to the driver!" << endl << endl;
	}
}

void test_memory_read(HANDLE hDev, DWORD pid)
{
	wcout << __FUNCTIONW__ << "():" << endl;

	// This variable holds the value which we will tamper with from inside our driver
	DWORD value = 123;

	// We will copy the previous variable's value into this
	DWORD read = 0;

	// This replace represents how many bytes were written to/read from our driver
	DWORD bytesIo = 0;

	// This is the request we are going to pass to our driver
	LGCOPYMEMORY_REQ request = {
		FALSE,						// specifies that we are READING from the memory
		pid,						// target process ID
		&value,						// target memory address
		&read,						// copy bytes begining from this address
		sizeof(value)				// the count of the bytes we will write
	};

	// Talk to the driver
	if (DeviceIoControl(hDev, IOCTL_LGCOPYMEMORY, &request, sizeof(request), nullptr,
		0, &bytesIo, nullptr))
	{
		wcout << "The read value is " << read << endl << endl;
	}
	else
	{
		wcout << "Failed talking to the driver!" << endl << endl;
	}
}

void test_invalid_ctl(HANDLE hDev)
{
	wcout << __FUNCTIONW__ << "():" << endl;

	DWORD bytesIo;

	// In this simple test we will send an invlaid control code to our driver
	// and check the results in WinDbg
	DWORD error;
	if (!DeviceIoControl(hDev, IOCTL_INVALID, nullptr, 0, nullptr,
		0, &bytesIo, nullptr))
	{
		error = GetLastError();
		if (error = ERROR_INVALID_PARAMETER)
			wcout << L"Excepted error code was ERROR_INVALID_PARAMETER " << hex << error << endl << endl;
		else
			wcout << L"DeviceIoControl failed with error code " << hex << error << endl << endl;
	}
	else
	{
		wcout << L"Request shouldn't have succeeded!" << endl << endl;
	}
}

void test_enum_regions(HANDLE hDev, DWORD pid)
{
	wcout << __FUNCTIONW__ << "():" << endl;

	DWORD bytesIo = 0;

	DWORD max =  MAX_LGMEMORY_REGIONS * sizeof(MEMORY_BASIC_INFORMATION);

	auto result = new BYTE[max];

	// This is the request we are going to pass to our driver
	LGGETMEMORYREGION_REQ request = {
		pid
	};

	// Talk to the driver
	if (DeviceIoControl(hDev, IOCTL_LGENUMMEMORYREGIONS, &request, sizeof(request), result,
		max, &bytesIo, nullptr))
	{
		bytesIo -= sizeof(request);
		wcout << L"OK " << "IO = " << bytesIo << endl;
		wcout << "mbi * " << bytesIo / sizeof(MEMORY_BASIC_INFORMATION) << endl;

		MEMORY_BASIC_INFORMATION mbi;
		int index = 0;
		while (bytesIo > 0)
		{
			memcpy(&mbi, result + index * sizeof(mbi), sizeof(mbi));
			index++;

			wcout << "Base address = " << mbi.BaseAddress << " RegionSize = " << mbi.RegionSize << endl;
			bytesIo -= sizeof(mbi);
		}
	}
	else
	{
		DWORD error = GetLastError();
		wcout << L"Error code " << "0x" << hex << error << endl;
	}

	delete[] result;
}

int wmain(int argc, wchar_t** argv)
{
	// Get the ID of the target process
	const auto pid = GetCurrentProcessId(); //GetProcessId(argv[1]);

	// Exit if PID was 0
	if (pid < 1)
	{
		wcout << L"Invalid proceess ID " << pid << " . Exiting.";
		return 0;
	}

	// Try to connect to the driver
	const auto hDev = CreateFile(LEGDWM_USERMODE_PATH, GENERIC_ALL, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM,
		nullptr);

	// Exit if we couldn't connect to the driver
	if (hDev == INVALID_HANDLE_VALUE)
	{
		std::wcout << L"Could not open driver" << std::endl;
		return 0;
	}

	//test_memory_write(hDev, pid);
	//test_memory_read(hDev, pid);
	//test_invalid_ctl(hDev);

	test_enum_regions(hDev, GetProcessId(argv[1]));

	// Close the device handler
	CloseHandle(hDev);
}
