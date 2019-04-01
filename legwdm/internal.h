#pragma once
#include "types.h"

#define PAGE_SIZE 0x1000
#define MM_POOL_TAG 'enoB'

VOID LgAdjustMemoryPointerByOffset(
	OUT PULONG_PTR ptr, 
	IN ULONG_PTR offset
);

NTSTATUS LgWait(
	IN LONG usec
);

