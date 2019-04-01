#pragma once
#if !defined(_CONSOLE) && !defined USERMODE
#include <ntdef.h>
#include <ntifs.h>
#endif

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

typedef struct _class_MEMORY_SECTION_NAME {
	UNICODE_STRING Name;
	WCHAR     Buffer[2048];
} MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;

typedef struct _class_LGMEMORY_REGION {
	MEMORY_SECTION_NAME section;
	MEMORY_BASIC_INFORMATION mbi;
} LGMEMORY_REGION, *PLGMEMORY_REGION;