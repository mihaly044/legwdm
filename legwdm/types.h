#pragma once
#if !defined(_CONSOLE) && !defined USERMODE
#include <ntdef.h>
#include <ntifs.h>
#define MAX_PATH 260
#endif

#define MAX_LGMEMORY_REGIONS 512

typedef struct _class_LGCOPYMEMORY_REQ
{
	BOOLEAN write;
	unsigned long pid;
	PVOID addr;
	PVOID data;
	unsigned long size;
} LGCOPYMEMORY_REQ, *PLGCOPYMEMORY_REQ;

typedef struct _class_LGGETMEMORYREGION_REQ
{
	unsigned long pid;
} LGGETMEMORYREGION_REQ, *PLGGETMEMORYREGION_REQ;

typedef struct _class_LGQUERYMEMIMAGENAME_REQ
{
	unsigned long pid;
	PVOID base;
} LGQUERYMEMIMAGENAME_REQ, *PLGQUERYMEMIMAGENAME_REQ;

typedef struct _class_MEMORY_SECTION_NAME {
	UNICODE_STRING Name;
	WCHAR     Buffer[MAX_PATH];
} MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;