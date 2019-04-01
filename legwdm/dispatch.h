#pragma once
#include "types.h"
NTSTATUS DispatchMajorFunction(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
);

NTSTATUS DispatchDeviceControl(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
);