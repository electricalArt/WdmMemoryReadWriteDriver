#pragma once

#define DRIVER_NAME L"WdmCheatDriver"
#define DRIVER_DEVICE_NAME     L"\\Device\\WdmCheatDriver"
#define DRIVER_DOS_DEVICE_NAME L"\\DosDevices\\WdmCheatDriver"
#define DRIVER_DEVICE_PATH  L"\\\\.\\WdmCheatDriver"
#define DRIVER_DEVICE_TYPE 0x00000022

// Request for read/writing memory from/to a process.
#define IOCTL_DRIVER_COPY_MEMORY ((ULONG)CTL_CODE(DRIVER_DEVICE_TYPE, 0x808, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS))

// Copy requeest data.
typedef struct _DRIVER_COPY_MEMORY {
	ULONGLONG Source; // Source buffer address. From where the memory will be read
	ULONGLONG Target; // Target buffer address. To where the memory will be written
	ULONGLONG Size; // Buffer size.
	ULONG ProcessId; // Source process ID
	BOOLEAN Write; // TRUE if writing, FALSE if reading.
} DRIVER_COPY_MEMORY, *PDRIVER_COPY_MEMORY;
