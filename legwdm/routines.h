#pragma once
#include "types.h"
NTSTATUS LgCopyMemory(
	IN PLGCOPYMEMORY_REQ pParam
);

NTSTATUS LgGetMemoryRegions(
	IN PLGGETMEMORYREGION_REQ pParam
);

NTSTATUS LgQueryMemImageName(
	IN PLGQUERYMEMIMAGENAME_REQ pParam,
	PVOID buffer,
	PSIZE_T count
);