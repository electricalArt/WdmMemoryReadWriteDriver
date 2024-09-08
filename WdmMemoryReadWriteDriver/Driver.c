#include <ntifs.h>
#include <ntstrsafe.h>
#include "WdmMemoryReadWriteDriver.h"

// Copies virtual memory from one process to another.
NTKERNELAPI NTSTATUS NTAPI MmCopyVirtualMemory(
	_In_ PEPROCESS FromProcess,
	_In_ PVOID FromAddress,
	_In_ PEPROCESS ToProcess,
	_Out_ PVOID ToAddress,
	_In_ SIZE_T BufferSize,
	_In_ KPROCESSOR_MODE PreviousMode,
	_Out_ PSIZE_T NumberOfBytesCopied
);

// Forward declaration for suppressing code analysis warnings.
DRIVER_INITIALIZE DriverEntry;

// Dispatch function.
_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CLOSE)
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH DriverDispatch;

// Performs a memory copy request.
NTSTATUS DriverCopy(_In_ PDRIVER_COPY_MEMORY copy) {
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS process;

	status = PsLookupProcessByProcessId((HANDLE)copy->ProcessId, &process);

	if (NT_SUCCESS(status)) {
		PEPROCESS sourceProcess, targetProcess;
		PVOID sourcePtr, targetPtr;

		if (copy->IsWrite) {
			sourceProcess = PsGetCurrentProcess();
			targetProcess = process;
		}
		else {
			sourceProcess = process;
			targetProcess = PsGetCurrentProcess();
		}

		sourcePtr = (PVOID)copy->Source;
		targetPtr = (PVOID)copy->Target;

		SIZE_T bytes;
		status = MmCopyVirtualMemory(sourceProcess, sourcePtr, targetProcess, targetPtr, copy->Size, KernelMode, &bytes);

		ObDereferenceObject(process);
	}

	return status;
}

// Handles a IRP request.
NTSTATUS DriverDispatch(_In_ PDEVICE_OBJECT DeviceObject, _Inout_ PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
	PVOID ioBuffer = Irp->AssociatedIrp.SystemBuffer;
	ULONG inputLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;

	if (irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
		ULONG ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

		if (ioControlCode == IOCTL_DRIVER_COPY_MEMORY) {
			if (ioBuffer && inputLength >= sizeof(DRIVER_COPY_MEMORY)) {
				Irp->IoStatus.Status = DriverCopy((PDRIVER_COPY_MEMORY)ioBuffer);
				Irp->IoStatus.Information = sizeof(DRIVER_COPY_MEMORY);
			} else {
				Irp->IoStatus.Status = STATUS_INFO_LENGTH_MISMATCH;
			}
		} else {
			Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		}
	}

	NTSTATUS status = Irp->IoStatus.Status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

// Unloads the driver.
VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING dosDeviceName;
	RtlUnicodeStringInit(&dosDeviceName, DRIVER_DOS_DEVICE_NAME);

	IoDeleteSymbolicLink(&dosDeviceName);
	IoDeleteDevice(DriverObject->DeviceObject);
}

// Entry point for the driver.
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	NTSTATUS status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(RegistryPath);

	UNICODE_STRING deviceName;
	RtlUnicodeStringInit(&deviceName, DRIVER_DEVICE_NAME);

	PDEVICE_OBJECT deviceObject = NULL;
	status = IoCreateDevice(DriverObject, 0, &deviceName, DRIVER_DEVICE_TYPE, 0, FALSE, &deviceObject);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverDispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverDispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDispatch;
	DriverObject->DriverUnload = DriverUnload;

	UNICODE_STRING dosDeviceName;
	RtlUnicodeStringInit(&dosDeviceName, DRIVER_DOS_DEVICE_NAME);

	status = IoCreateSymbolicLink(&dosDeviceName, &deviceName);

	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(deviceObject);
	}

	return status;
}
