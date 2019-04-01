#include "common.h"
#include "dispatch.h"
#include "main.h"

UNICODE_STRING g_DevName;
UNICODE_STRING g_SymLink;
PDEVICE_OBJECT g_DeviceObject = NULL;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	RtlInitUnicodeString(&g_DevName, LEGWDM_FILE_DEVICE);
	RtlInitUnicodeString(&g_SymLink, LEGDWM_SYMLINK_DEVICE);

	NTSTATUS status = IoCreateDevice(DriverObject, 0, &g_DevName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE,
		&g_DeviceObject);

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to create device\r\n");
		return status;
	}

	status = IoCreateSymbolicLink(&g_SymLink, &g_DevName);
	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to create symlink\r\n");
		return status;
	}

	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = DispatchMajorFunction;
	}

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
	DriverObject->DriverUnload = DriverUnload;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Load successfull\r\n");
	return status;
}

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	IoDeleteSymbolicLink(&g_SymLink);
	IoDeleteDevice(g_DeviceObject);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Unloaded driver\r\n");
}