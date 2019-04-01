#pragma once
#include "types.h"
#include "imports.h"

#define PAGE_SIZE 0x1000
#define MM_POOL_TAG 'wdM'
#define MM_POOL_TAG1 'mndW'

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