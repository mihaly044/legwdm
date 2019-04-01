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