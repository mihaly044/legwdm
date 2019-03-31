#ifndef _CONSOLE
	#include <ntdef.h>
#endif

typedef struct _class_LGCOPYMEMORY_REQ
{
	BOOLEAN write;
	unsigned long pid;
	PVOID addr;
	PVOID data;
	unsigned long size;
} LGCOPYMEMORY_REQ, *PLGCOPYMEMORY_REQ;