#pragma once
#include "types.h"
#include "imports.h"

//#define PAGE_SIZE 0x1000

// Pool tags should be not more than 4 chars
// @seealso https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/content/wdm/nf-wdm-exallocatepoolwithtag
#define MM_POOL_TAG '1baT'
#define MM_POOL_TAG1 '2gaT'

typedef struct _OBJECT_DIRECTORY_INFORMATION
{
	UNICODE_STRING Name;
	UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

VOID LgAdjustMemoryPointerByOffset(
	OUT PULONG_PTR ptr, 
	IN ULONG_PTR offset
);

NTSTATUS LgWait(
	IN LONG usec
);

ULONG NtQueryDosDevice(
	WCHAR* wzDosDevice,
	WCHAR* wzNtDevice,
	ULONG ucchMax
);

BOOLEAN NtPathToDosPathW(
	IN WCHAR* wzFullNtPath,
	OUT WCHAR* wzFullDosPath
);