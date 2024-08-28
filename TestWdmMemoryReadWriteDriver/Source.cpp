#include <Windows.h>
#include <stdio.h>
#include "WdmMemoryReadWriteDriver.h"

DWORD PerformTest(DRIVER_COPY_MEMORY copy, PDWORD pBuff)
{
	DWORD result = 0;
	DWORD cbReturned = 0;

	HANDLE hDevice = CreateFileW(
		DRIVER_DEVICE_PATH,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0,
		OPEN_EXISTING,
		0, 0);
	if (hDevice == INVALID_HANDLE_VALUE) {
		wprintf(L"Failed to open the device (%s). Error: %d\n", DRIVER_DEVICE_PATH, GetLastError());
		result = 1;
	}

	if (result == 0) {

		if (!DeviceIoControl(
			hDevice,
			IOCTL_DRIVER_COPY_MEMORY,
			&copy,
			sizeof(copy),
			&copy,
			sizeof(copy),
			&cbReturned,
			NULL
		)) {
			printf("DeviceIoControl() failed with error: %d\n", GetLastError());
			result = 2;
		}
	}

	printf("cbReturned: %d\n", cbReturned);
	printf("Output: %d\n", *pBuff);

	if (hDevice) {
		CloseHandle(hDevice);
	}
	
	return result;
}

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Summary:  What MyLocalFunction is for and what it does.

  Args:     TestWdmMemoryReadWriteDriver read  <ProcessId> <Pointer> <Size>
			TestWdmMemoryReadWriteDriver write <ProcessId> <Pointer> <Size> <NewValue>


  Returns:  zero
				if success.
			nonzero
				if fail
----------------------------------------------------------------F-F*/
int wmain(int argc, wchar_t* argv[])
{
	DWORD result = 0;
	DRIVER_COPY_MEMORY copy = {};
	DWORD buff = 0;

	if (argc < 5) {
		wprintf(
			L"\n"
			L"Invalid input.\n"
			L"\n"
			L"Usage:\n"
			L"TestWdmMemoryReadWriteDriver read  <ProcessId> <Pointer> <Size>\n"
			L"TestWdmMemoryReadWriteDriver write <ProcessId> <Pointer> <Size> <NewValue>\n");
		result = 1;
	}

	if (result == 0) {
		if (wcscmp(argv[1], L"read") == 0) {
			copy.Write = FALSE;
			copy.Source = (ULONGLONG)&buff;
			copy.Target = _wcstoui64(argv[3], NULL, 16);
		}
		else if (wcscmp(argv[1], L"write") == 0) {
			copy.Write = TRUE;
			copy.Source = _wcstoui64(argv[3], NULL, 16);
			copy.Target = (ULONGLONG)&buff;
			buff = wcstoul(argv[5], NULL, 10);
		}
		copy.ProcessId = wcstoul(argv[2], NULL, 10);
		copy.Size = wcstoul(argv[4], NULL, 10);

		wprintf(L"ProcessId: %d\n", copy.ProcessId);
		if (copy.Write) {
			puts("Operation: write");
			wprintf(L"Pointer: %llx\n", copy.Source);
			wprintf(L"New requested value: %d\n", buff);
		}
		else {
			puts("Operation: read");
			wprintf(L"Pointer: %llx\n", copy.Target);
		}
		wprintf(L"Size: %lld\n", copy.Size);

		result = PerformTest(copy, &buff);
	}


	return result;
}