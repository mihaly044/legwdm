#include "common.h"
#include "dispatch.h"
#include "routines.h"

NTSTATUS DispatchMajorFunction(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	switch (IoStackLocation->MajorFunction)
	{
	case IRP_MJ_CREATE:
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_CREATE called\r\n");
		break;
	case IRP_MJ_CLOSE:
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_CLOSE called\r\n");
		break;
	case IRP_MJ_READ:
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_READ called\r\n");
		break;
	case IRP_MJ_WRITE:
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_WRITE called\r\n");
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
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s:%d IOCTL_LGCOPYMEMORY was called\r\n", __FILE__, __LINE__);

		status = LgCopyMemory((PLGCOPYMEMORY_REQ)Irp->AssociatedIrp.SystemBuffer);
		processedIo = sizeof(PLGCOPYMEMORY_REQ);
		break;
	default:
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s:%d Invalid control code: %08x\r\n", __FILE__, __LINE__, 
			IoStackLocation->Parameters.DeviceIoControl.IoControlCode);
		status = STATUS_INVALID_PARAMETER;
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = processedIo;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}