#include "internal.h"

VOID LgAdjustMemoryPointerByOffset(OUT PULONG_PTR ptr, IN ULONG_PTR offset)
{
	*ptr += offset;
}

NTSTATUS LgWait(IN LONG usec)
{
	ULONG ulReturn = 0;
	if (usec < 50000)
	{
		/* KeDelayExecutionThread yields the process */
		/* Waiting times lower than 50ms are not accurate */
		/* Because of this we need to busy wait here */
		KeStallExecutionProcessor(usec);
	}
	else
	{
		LARGE_INTEGER waittime;
		/* specifies the wait time in steps of 100ns */
		/* Negative values specify a relative time offset */
		waittime.QuadPart = -1 * usec * 10;
		if (STATUS_SUCCESS == KeDelayExecutionThread(KernelMode, TRUE, &waittime))
			ulReturn = 0;
		else
			ulReturn = 1;
	}
	return ulReturn;
}

ULONG NtQueryDosDevice(WCHAR* wzDosDevice, WCHAR* wzNtDevice, ULONG ucchMax)
{
	NTSTATUS Status;
	POBJECT_DIRECTORY_INFORMATION ObjectDirectoryInfor;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING uniString;
	HANDLE hDirectory;
	HANDLE hDevice;
	ULONG  ulReturnLength;
	ULONG  ulNameLength;
	ULONG  ulLength;
	ULONG       Context;
	BOOLEAN     bRestartScan;
	WCHAR*      Ptr = NULL;
	UCHAR       szBuffer[512] = { 0 };
	RtlInitUnicodeString(&uniString, L"\\??");
	InitializeObjectAttributes(&oa,
		&uniString,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);
	Status = ZwOpenDirectoryObject(&hDirectory, DIRECTORY_QUERY, &oa);
	if (!NT_SUCCESS(Status))
	{
		return 0;
	}
	ulLength = 0;
	if (wzDosDevice != NULL)
	{
		RtlInitUnicodeString(&uniString, (PWSTR)wzDosDevice);
		InitializeObjectAttributes(&oa, &uniString, OBJ_CASE_INSENSITIVE, hDirectory, NULL);
		Status = ZwOpenSymbolicLinkObject(&hDevice, GENERIC_READ, &oa);
		if (!NT_SUCCESS(Status))
		{
			ZwClose(hDirectory);
			return 0;
		}
		uniString.Length = 0;
		uniString.MaximumLength = (USHORT)ucchMax * sizeof(WCHAR);
		uniString.Buffer = wzNtDevice;
		ulReturnLength = 0;
		Status = ZwQuerySymbolicLinkObject(hDevice, &uniString, &ulReturnLength);
		ZwClose(hDevice);
		ZwClose(hDirectory);
		if (!NT_SUCCESS(Status))
		{
			return 0;
		}
		ulLength = uniString.Length / sizeof(WCHAR);
		if (ulLength < ucchMax)
		{
			wzNtDevice[ulLength] = UNICODE_NULL;
			ulLength++;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		bRestartScan = TRUE;
		Context = 0;
		Ptr = wzNtDevice;
		ObjectDirectoryInfor = (POBJECT_DIRECTORY_INFORMATION)szBuffer;
		while (TRUE)
		{
			Status = ZwQueryDirectoryObject(hDirectory, szBuffer, sizeof(szBuffer), TRUE, bRestartScan, &Context, &ulReturnLength);
			if (!NT_SUCCESS(Status))
			{
				if (Status == STATUS_NO_MORE_ENTRIES)
				{
					*Ptr = UNICODE_NULL;
					ulLength++;
					Status = STATUS_SUCCESS;
				}
				else
				{
					ulLength = 0;
				}
				break;
			}
			if (!wcscmp(ObjectDirectoryInfor->TypeName.Buffer, L"SymbolicLink"))
			{
				ulNameLength = ObjectDirectoryInfor->Name.Length / sizeof(WCHAR);
				if (ulLength + ulNameLength + 1 >= ucchMax)
				{
					ulLength = 0;
					break;
				}
				memcpy(Ptr, ObjectDirectoryInfor->Name.Buffer, ObjectDirectoryInfor->Name.Length);
				Ptr += ulNameLength;
				ulLength += ulNameLength;
				*Ptr = UNICODE_NULL;
				Ptr++;
				ulLength++;
			}
			bRestartScan = FALSE;
		}
		ZwClose(hDirectory);
	}
	return ulLength;
}

BOOLEAN NtPathToDosPathW(IN WCHAR* wzFullNtPath, OUT WCHAR* wzFullDosPath)
{
	WCHAR wzDosDevice[4] = { 0 };
	WCHAR wzNtDevice[64] = { 0 };
	WCHAR *RetStr = NULL;
	size_t NtDeviceLen = 0;
	short i = 0;
	if (!wzFullNtPath || !wzFullDosPath)
	{
		return FALSE;
	}
	for (i = 65; i < 26 + 65; i++)
	{
		wzDosDevice[0] = i;
		wzDosDevice[1] = L':';
		if (NtQueryDosDevice(wzDosDevice, wzNtDevice, 64))
		{
			if (wzNtDevice)
			{
				NtDeviceLen = wcslen(wzNtDevice);
				if (!_wcsnicmp(wzNtDevice, wzFullNtPath, NtDeviceLen))
				{
					wcscpy(wzFullDosPath, wzDosDevice);
					wcscat(wzFullDosPath, wzFullNtPath + NtDeviceLen);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}