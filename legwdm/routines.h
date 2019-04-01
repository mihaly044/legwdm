#pragma once
#include "types.h"
NTSTATUS LgCopyMemory(
	IN PLGCOPYMEMORY_REQ pParam
);

NTSTATUS LgGetMemoryRegions(
	IN PLGGETMEMORYREGION_REQ pParam,
	PVOID buffer,
	PUINT32 count
);