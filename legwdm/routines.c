#include "routines.h"
#include "imports.h"

NTSTATUS LgCopyMemory(IN PLGCOPYMEMORY_REQ pParam)
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pProcess = NULL, pSourceProc = NULL, pTargetProc = NULL;
	PVOID pSource = NULL, pTarget = NULL;

	status = PsLookupProcessByProcessId((HANDLE)pParam->pid, &pProcess);

	if (NT_SUCCESS(status))
	{
		SIZE_T bytes = 0;

		// Write
		if (pParam->write != FALSE)
		{
			pSourceProc = PsGetCurrentProcess();
			pTargetProc = pProcess;
			pSource = (PVOID)pParam->data;
			pTarget = (PVOID)pParam->addr;
		}
		// Read
		else
		{
			pSourceProc = pProcess;
			pTargetProc = PsGetCurrentProcess();
			pSource = (PVOID)pParam->addr;
			pTarget = (PVOID)pParam->data;
		}

		status = MmCopyVirtualMemory(pSourceProc, pSource, pTargetProc, pTarget, pParam->size, KernelMode, &bytes);
	}
	else
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s %s: PsLookupProcessByProcessId failed with status 0x%X\n", __FILE__, __FUNCTION__, status);

	if (pProcess)
		ObDereferenceObject(pProcess);

	return status;
}