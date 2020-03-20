#include "routines.h"
#include "imports.h"
#include "internal.h"

NTSTATUS LgCopyMemory(IN PLGCOPYMEMORY_REQ pParam)
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pProcess = NULL, pSourceProc = NULL, pTargetProc = NULL;
	PVOID pSource = NULL, pTarget = NULL;

	// Deny access to non-user space
	if (pParam->pAddr >= (ULONGLONG)MM_HIGHEST_USER_ADDRESS || (ULONGLONG)(pParam->pAddr) + pParam->dwSize > (ULONGLONG)MM_HIGHEST_USER_ADDRESS)
	{
		return STATUS_INVALID_ADDRESS;
	}

	status = PsLookupProcessByProcessId((HANDLE)pParam->dwPid, &pProcess);
	if (NT_SUCCESS(status))
	{
		SIZE_T bytes = 0;

		// Write
		if (pParam->bWrite != FALSE)
		{
			pSourceProc = PsGetCurrentProcess();
			pTargetProc = pProcess;
			pSource = (PVOID)pParam->pData;
			pTarget = (PVOID)pParam->pAddr;
		}
		// Read
		else
		{
			pSourceProc = pProcess;
			pTargetProc = PsGetCurrentProcess();
			pSource = (PVOID)pParam->pAddr;
			pTarget = (PVOID)pParam->pData;
		}

		status = MmCopyVirtualMemory(pSourceProc, pSource, pTargetProc, pTarget, pParam->dwSize, KernelMode, &bytes);
	}
	else
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s %s: PsLookupProcessByProcessId failed with status 0x%X\n", __FILE__, __FUNCTION__, status);

	if (pProcess)
		ObDereferenceObject(pProcess);

	return status;
}

NTSTATUS LgGetMemoryRegions(IN PLGGETMEMORYREGION_REQ pParam)
{
	NTSTATUS status;				// NT status of the routine
	ULONG_PTR base;					// user-space memory address to query for
	DWORD count;					// count of results
	SIZE_T cbIo;					// number of bytes copied via MmCopyVirtualMemory
	PEPROCESS pProcess;				// target process
	PEPROCESS pCalleeProcess;		// callee user process
	HANDLE hProcess;				// apc state of pProcess
	KAPC_STATE apc;					// apc of pProcess
	MEMORY_BASIC_INFORMATION mbi;	// temporary var to copy to user space
	PCHAR buf;						// buffer for temp data

	// Init varialbes
	base = (ULONG_PTR)MM_LOWEST_USER_ADDRESS;
	count = 0;

	// Allocate memory for temp buffer
	buf = ExAllocatePool(PagedPool, MAX_LGMEMORY_REGIONS * sizeof(MEMORY_BASIC_INFORMATION));

	// Return if memory allocation has failed
	if (buf == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// Return if the callee process id is invalid
	status = PsLookupProcessByProcessId((HANDLE)pParam->dwCpId, &pCalleeProcess);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	// Return if the target process id is invalid
	status = PsLookupProcessByProcessId((HANDLE)pParam->dwPid, &pProcess);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	// Attach process to the stack, set current process variable
	KeStackAttachProcess(pProcess, &apc);
	hProcess = ZwCurrentProcess();

	// Loop through memregion MM_LOWEST_USER_ADDRESS to MM_HIGHEST_USER_ADDRESS
	while (base < (ULONG_PTR)MM_HIGHEST_USER_ADDRESS - PAGE_SIZE - 1)
	{
		// Attempt to retrieve information about the curreent block at base address
		status = ZwQueryVirtualMemory(hProcess, (PULONG_PTR)base, MemoryBasicInformation, &mbi, sizeof(MEMORY_BASIC_INFORMATION), NULL);
		if (NT_SUCCESS(status))
		{
			// Skip re-allocated memory
			if ((ULONG_PTR)mbi.AllocationBase != base)
			{
				// Advance base pointers
				if (mbi.RegionSize > 0)
					LgAdjustMemoryPointerByOffset(&base, mbi.RegionSize);
				else
					LgAdjustMemoryPointerByOffset(&base, PAGE_SIZE);

				continue;
			}

			// We have ran out of our result buffer size. Truncate the output
			if (count > MAX_LGMEMORY_REGIONS)
			{
				count--;
				goto Detach;
			}

			// Copy current MBI to temp buffer
			RtlCopyMemory((PVOID)((ULONG_PTR)buf + (ULONG_PTR)count * sizeof(MEMORY_BASIC_INFORMATION)), &mbi, sizeof(mbi));
			count++;

			// Advance the base pointer so that it will point to the next
			// memory region
			if (mbi.RegionSize > 0)
				LgAdjustMemoryPointerByOffset(&base, mbi.RegionSize);
			else
				LgAdjustMemoryPointerByOffset(&base, PAGE_SIZE);
		}
		else
		{
			goto Detach;
		}
	}

	// Copy results to user space
	MmCopyVirtualMemory(pProcess, buf, pCalleeProcess, pParam->pMbi, sizeof(MEMORY_BASIC_INFORMATION) * count, KernelMode, &cbIo);
	MmCopyVirtualMemory(pProcess, &count, pCalleeProcess, pParam->pcbMbi, sizeof(DWORD), KernelMode, &cbIo);

	// Cleanup
	Detach:
	KeUnstackDetachProcess(&apc);
	if (pProcess)
		ObDereferenceObject(pProcess);
	
	if (pCalleeProcess)
		ObDereferenceObject(pCalleeProcess);

	ExFreePool(buf);
	return status;
}

NTSTATUS LgQueryMemImageName(IN PLGQUERYMEMIMAGENAME_REQ pParam, PVOID buffer, PSIZE_T count)
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pProcess;
	HANDLE hProcess;
	KAPC_STATE apc;

	status = PsLookupProcessByProcessId((HANDLE)pParam->pid, &pProcess);
	if (!NT_SUCCESS(status))
	{
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s %s: PsLookupProcessByProcessId failed with status 0x%X\n", __FILE__, __FUNCTION__, status);
	}
	else
	{
		KeStackAttachProcess(pProcess, &apc);
		hProcess = ZwCurrentProcess();

		MEMORY_SECTION_NAME msn = { 0 };
		status = ZwQueryVirtualMemory(hProcess, (PULONG_PTR)pParam->base, 2, &msn, sizeof(MEMORY_SECTION_NAME), NULL);
		if (NT_SUCCESS(status))
		{
			NtPathToDosPathW(msn.Buffer, buffer);
			*count = wcsnlen_s(buffer, MAX_PATH);
		}
		else
		{
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s %s: ZwQueryVirtualMemory failed with status 0x%X\n", __FILE__, __FUNCTION__, status);
		}

		KeUnstackDetachProcess(&apc);
	}

	if (pProcess)
		ObDereferenceObject(pProcess);

	return status;
}