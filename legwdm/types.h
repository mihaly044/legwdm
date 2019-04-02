#pragma once
#if !defined(_CONSOLE) && !defined USERMODE
#include <ntdef.h>
#include <ntifs.h>
#include <minwindef.h>
#define MAX_PATH 260
#endif

#define MAX_LGMEMORY_REGIONS 2048

typedef struct _class_LGCOPYMEMORY_REQ
{
	BOOLEAN bWrite;
	DWORD dwPid;
	PVOID pAddr;
	PVOID pData;
	DWORD dwSize;
} LGCOPYMEMORY_REQ, *PLGCOPYMEMORY_REQ;

typedef struct _class_LGGETMEMORYREGION_REQ
{
	DWORD dwCpId;
	DWORD dwPid;
	PVOID pMbi;
	PVOID pcbMbi;
} LGGETMEMORYREGION_REQ, *PLGGETMEMORYREGION_REQ;

typedef struct _class_LGQUERYMEMIMAGENAME_REQ
{
	DWORD pid;
	PVOID base;
} LGQUERYMEMIMAGENAME_REQ, *PLGQUERYMEMIMAGENAME_REQ;

typedef struct _class_MEMORY_SECTION_NAME {
	UNICODE_STRING Name;
	WCHAR     Buffer[MAX_PATH];
} MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;