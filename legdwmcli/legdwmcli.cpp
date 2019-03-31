// legdwmcli.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <Shlwapi.h>

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

	// Get the ID of the target process
	const auto pid = GetCurrentProcessId(); //GetProcessId(argv[1]);

	// Exit if PID was 0
	if (pid < 1)
	{
		wcout << L"Invalid proceess ID " << pid << " . Exiting.";
		return 0;
	}

	// Try to connect to the driver
	const auto h_dev = CreateFile(LEGDWM_USERMODE_PATH, GENERIC_ALL, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM,
		nullptr);

	// Exit if we couldn't connect to the driver
	if (h_dev == INVALID_HANDLE_VALUE)
	{
		std::wcout << L"Could not open driver" << std::endl;
		return 0;
	}

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
	if (DeviceIoControl(h_dev, IOCTL_LGCOPYMEMORY, &request, sizeof(request), nullptr,
		0, &bytesIo, nullptr))
	{
		wcout << "The value is " << value << endl;
	}
	else
	{
		wcout << "Failed talking to the driver!" << endl;
	}

	CloseHandle(h_dev);
}
