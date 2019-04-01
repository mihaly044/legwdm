#include "routines.h"
#include "imports.h"
#include "internal.h"

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

NTSTATUS LgGetMemoryRegions(IN PLGGETMEMORYREGION_REQ pParam, PVOID buffer, PUINT32 count)
{
	NTSTATUS status = STATUS_SUCCESS;

	ULONG_PTR base = 0;

	MEMORY_BASIC_INFORMATION mbi;
	PEPROCESS pProcess;
	HANDLE hProcess;
	KAPC_STATE apc;
	base = (ULONG_PTR)MM_LOWEST_USER_ADDRESS;

	status = PsLookupProcessByProcessId((HANDLE)pParam->pid, &pProcess);
	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s %s: PsLookupProcessByProcessId failed with status 0x%X\n", __FILE__, __FUNCTION__, status);
	}
	else
	{
		KeStackAttachProcess(pProcess, &apc);
		hProcess = ZwCurrentProcess();

		while (base < (ULONG_PTR)MM_HIGHEST_USER_ADDRESS - PAGE_SIZE - 1)
		{
			status = ZwQueryVirtualMemory(hProcess, (PULONG_PTR)base, MemoryBasicInformation, &mbi, sizeof(mbi), NULL);

			if (NT_SUCCESS(status))
			{
				if (/*mbi.Type != 0x1000000 ||*/ (ULONG_PTR)mbi.AllocationBase != base)
				{
					if (mbi.RegionSize > 0)
						LgAdjustMemoryPointerByOffset(&base, mbi.RegionSize);
					else
						LgAdjustMemoryPointerByOffset(&base, PAGE_SIZE);

					continue;
				}
			}
			else
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s %s: ZwQueryVirtualMemory failed with status 0x%X\n", __FILE__, __FUNCTION__, status);
				goto Detach;
			}

			if (NT_SUCCESS(status))
			{
				if (*count > MAX_LGMEMORY_REGIONS)
				{
					*count = *count - 1;
					goto Detach;
				}

				RtlCopyMemory((PVOID)( (ULONG_PTR)buffer + *count * sizeof(MEMORY_BASIC_INFORMATION)) , &mbi, sizeof(mbi));
				*count = *count + 1;

				if (mbi.RegionSize > 0)
					LgAdjustMemoryPointerByOffset(&base, mbi.RegionSize);
				else
					LgAdjustMemoryPointerByOffset(&base, PAGE_SIZE);
			}
			else
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s %s: ZwQueryVirtualMemory failed with status 0x%X\n", __FILE__, __FUNCTION__, status);
				goto Detach;
			}
		}

		Detach:
		KeUnstackDetachProcess(&apc);
	}

	if (pProcess)
		ObDereferenceObject(pProcess);

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
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s %s: PsLookupProcessByProcessId failed with status 0x%X\n", __FILE__, __FUNCTION__, status);
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
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s %s: ZwQueryVirtualMemory failed with status 0x%X\n", __FILE__, __FUNCTION__, status);
		}

		KeUnstackDetachProcess(&apc);
	}

	if (pProcess)
		ObDereferenceObject(pProcess);

	return status;
}