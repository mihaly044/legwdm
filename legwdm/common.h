#pragma once
#define LEGWDM L"legwdm"

#define LEGWDM_FILE_DEVICE		L"\\Device\\" LEGWDM
#define LEGDWM_SYMLINK_DEVICE  L"\\??\\" LEGWDM
#define LEGDWM_USERMODE_PATH	L"\\\\.\\" LEGWDM

#define IOCTL_INVALID					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x000, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_LGCOPYMEMORY				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x902, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_LGENUMMEMORYREGIONS		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x903, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_LGQUERYMEMIMAGENAME		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x904, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)