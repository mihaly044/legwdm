#pragma once
#include <wdm.h>

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
);

VOID DriverUnload(
	IN PDRIVER_OBJECT DriverObject
);