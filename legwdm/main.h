#pragma once
#include "types.h"
NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
);

VOID DriverUnload(
	IN PDRIVER_OBJECT DriverObject
);