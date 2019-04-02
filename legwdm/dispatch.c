#include "common.h"
#include "dispatch.h"
#include "routines.h"
#include "internal.h"

NTSTATUS DispatchMajorFunction(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	switch (IoStackLocation->MajorFunction)
	{
	case IRP_MJ_CREATE:
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_CREATE called\r\n");
		break;
	case IRP_MJ_CLOSE:
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_CLOSE called\r\n");
		break;
	case IRP_MJ_READ:
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_READ called\r\n");
		break;
	case IRP_MJ_WRITE:
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_WRITE called\r\n");
		break;
	default:
		status = STATUS_INVALID_PARAMETER;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;
	ULONG processedIo = 0;

	switch (IoStackLocation->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_LGCOPYMEMORY:
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s:%d IOCTL_LGCOPYMEMORY was called\r\n", __FILE__, __LINE__);
		
		PLGCOPYMEMORY_REQ pParam = (PLGCOPYMEMORY_REQ)Irp->AssociatedIrp.SystemBuffer;
		if (!pParam || pParam->pid == 0)
		{
			status = STATUS_INVALID_PARAMETER;
		}
		else
		{
			status = LgCopyMemory(pParam);
		}
		
		processedIo = sizeof(PLGCOPYMEMORY_REQ);
		break;

	case IOCTL_LGENUMMEMORYREGIONS:
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s:%d IOCTL_LGENUMMEMORYREGIONS was called\r\n", __FILE__, __LINE__);
		PLGGETMEMORYREGION_REQ pParam1 = (PLGGETMEMORYREGION_REQ)Irp->AssociatedIrp.SystemBuffer;

		if (!pParam1 || pParam1->pid == 0)
		{
			status = STATUS_INVALID_PARAMETER;
			processedIo = sizeof(LGGETMEMORYREGION_REQ);
		}
		else
		{
			if (IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength != MAX_LGMEMORY_REGIONS * sizeof(MEMORY_BASIC_INFORMATION))
			{
				status = STATUS_NO_MEMORY;
				processedIo = sizeof(LGGETMEMORYREGION_REQ);
			}
			else
			{
				SIZE_T count = 0;
				PVOID buf = ExAllocatePoolWithTag(PagedPool, MAX_LGMEMORY_REGIONS * (sizeof(MEMORY_BASIC_INFORMATION)), MM_POOL_TAG);

				if (buf == NULL )
				{
					status = STATUS_INSUFFICIENT_RESOURCES;
					processedIo = sizeof(LGGETMEMORYREGION_REQ);
				}
				else
				{
					status = LgGetMemoryRegions(pParam1, buf, &count);

					if (count > 0)
					{
						RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, buf, sizeof(MEMORY_BASIC_INFORMATION) * count);
						processedIo = sizeof(LGGETMEMORYREGION_REQ) + sizeof(MEMORY_BASIC_INFORMATION) * count;
					}
					else
					{
						processedIo = sizeof(LGGETMEMORYREGION_REQ);
					}

					ExFreePoolWithTag(buf, MM_POOL_TAG);
				}
			}
		}
		break;

	case IOCTL_LGQUERYMEMIMAGENAME:
		////DbgPrintEx causes PAGE_FAULT if called heavily. Have no idea why. Probably it eats up the stack?
		////DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s:%d IOCTL_LGQUERYMEMIMAGENAME was called\r\n", __FILE__, __LINE__);
		Irp->AssociatedIrp.SystemBuffer;
		PLGQUERYMEMIMAGENAME_REQ pParam2 = (PLGQUERYMEMIMAGENAME_REQ)Irp->AssociatedIrp.SystemBuffer;

		if (!pParam2 || pParam2->pid == 0)
		{
			status = STATUS_INVALID_PARAMETER;
			processedIo = sizeof(LGQUERYMEMIMAGENAME_REQ);
		}
		else
		{
			SIZE_T count1 = 0;
			PVOID buf1 = ExAllocatePoolWithTag(PagedPool, 1024 * (sizeof(WCHAR)), MM_POOL_TAG1);

			if (buf1 == NULL)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				processedIo = sizeof(LGQUERYMEMIMAGENAME_REQ);
			}
			else
			{
				status = LgQueryMemImageName(pParam2, buf1, &count1);

				if (count1 > 0)
				{
					RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, buf1, sizeof(WCHAR) * count1);
					processedIo = sizeof(LGQUERYMEMIMAGENAME_REQ) + sizeof(WCHAR) * (ULONG)count1 + 1;
				}
				else
				{
					processedIo = sizeof(LGQUERYMEMIMAGENAME_REQ);
				}
			}

			ExFreePoolWithTag(buf1, MM_POOL_TAG1);
		}
		break;
	default:
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s:%d Invalid control code: 0x%08x\r\n", __FILE__, __LINE__, 
		//	IoStackLocation->Parameters.DeviceIoControl.IoControlCode);
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = processedIo;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}