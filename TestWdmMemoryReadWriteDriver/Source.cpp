#include <Windows.h>
#include <stdio.h>
#include <easylogging++.h>
#include <tclap/CmdLine.h>
#include "WdmMemoryReadWriteDriver.h"

INITIALIZE_EASYLOGGINGPP

// Reads virutal memory of specified process using driver.
BOOL ReadProcessMemoryDrivered(
    _In_ HANDLE hProcess,
    _In_ LPCVOID lpBaseAddress,
    _Out_ LPVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ LPDWORD lpNumberOfBytesReturned);

// Writes virutal memory of specified process using driver.
BOOL WriteProcessMemoryDrivered(
    _In_ HANDLE hProcess,
    _In_ LPCVOID lpBaseAddress,
    _In_ LPVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ LPDWORD lpNumberOfBytesReturned);

// Parses command-line arguments.
void ParseArguments(
    _In_ int argc,
    _In_ char** argv,
    _Out_ std::string& command,
    _Out_ DWORD* processId,
    _Out_ LPVOID* pointer,
    _Out_ INT32* newValue);

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Summary:  Reads/writes values from specified process with specified
            pointer address using `WdmMemoryReadWriteDriver`.

  Args:     TestWdmMemoryReadWriteDriver read  <ProcessId> <Pointer> 
			TestWdmMemoryReadWriteDriver write <ProcessId> <Pointer> <NewValue>

  Returns:  zero
				If success.
			nonzero
				If fail.
----------------------------------------------------------------F-F*/
int main(int argc, char** argv)
{
	DWORD result = EXIT_SUCCESS;
    BOOL ioResult = FALSE;
    std::string command;
    DWORD processId = 0;
    LPVOID pointer = NULL;
    INT32 newValue = 0;
    HANDLE process = NULL;
    DWORD buff = 0;
    DWORD cbReturned = 0;

    try {
        ParseArguments(argc, argv, command, &processId, &pointer, &newValue);

        process = OpenProcess(
            PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_LIMITED_INFORMATION,
            FALSE,
            processId);
        if (process == NULL)
            throw std::runtime_error("Failed to open specified process");

        if (command == "read") {
            ioResult = ReadProcessMemoryDrivered(
                process,
                pointer,
                &buff,
                sizeof(buff),
                &cbReturned);
            LOG(INFO) << "buff: " << buff;
            if (ioResult == FALSE)
                throw std::runtime_error("Failed to read from specified process");
        }
        else if (command == "write") {
            ioResult = WriteProcessMemoryDrivered(
                process,
                pointer,
                &newValue,
                sizeof newValue,
                &cbReturned);
            if (ioResult == FALSE)
                throw std::runtime_error("Failed to write to specified process");
        }
        else {
            throw TCLAP::ArgException("Invalid command is specified", "command");
        }

        std::cout << "buff: " << buff << std::endl;
        std::cout << "cbReturned: " << cbReturned << std::endl;
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        result = EXIT_FAILURE;
    }

    if (process)
        CloseHandle(process);
	return result;
}

void ParseArguments(
    _In_ int argc,
    _In_ char** argv,
    _Out_ std::string& command,
    _Out_ DWORD* processId,
    _Out_ LPVOID* pointer,
    _Out_ INT32* newValue)
{
    TCLAP::CmdLine cmd(
        "TestWdmMemoryReadWriteDriver",
        ' ',
        "1.1");
    TCLAP::UnlabeledValueArg<std::string> commandArg(
        "command", "The following commands are available: read | write", TRUE,
        "",
        "command", cmd);
    TCLAP::UnlabeledValueArg<DWORD> processIdArg(
        "process-id", "Process Id", TRUE,
        1223,
        "process-id", cmd);
    TCLAP::UnlabeledValueArg<LPVOID> pointerArg(
        "target-pointer", "Target process pointer", TRUE,
        (LPVOID)0x111111111,
        "process-id", cmd);
    TCLAP::UnlabeledValueArg<INT32> newValueArg(
        "new-value", "Value to be written", FALSE,
        0,
        "new-value", cmd);
    cmd.parse(argc, argv);

    command = commandArg.getValue();
    *processId = processIdArg.getValue();
    *pointer = pointerArg.getValue();
    *newValue = newValueArg.getValue();
    
    if (command == "write" && newValueArg.isSet() == FALSE) {
        throw std::runtime_error("`new-value` command-line argument is not set");
    }

    LOG(INFO) << "command: " << command;
    LOG(INFO) << "processId: " << *processId;
    LOG(INFO) << "pointer: 0x" << std::hex << *pointer;
    LOG(INFO) << "newValue: " << std::dec << *newValue;
}

BOOL ReadProcessMemoryDrivered(
    _In_ HANDLE hProcess,
    _In_ LPCVOID lpBaseAddress,
    _Out_ LPVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ LPDWORD lpNumberOfBytesReturned)
{
    BOOL result = TRUE;
    DRIVER_COPY_MEMORY copyInfo = { 0 };
    if (lpNumberOfBytesReturned) {
        *lpNumberOfBytesReturned = 0;
    }
    LOG_IF(hProcess == NULL, ERROR) << "hProcess is NULL";

    copyInfo.ProcessId = GetProcessId(hProcess);
    copyInfo.Source = (ULONGLONG)lpBaseAddress;
    copyInfo.Target = (ULONGLONG)lpBuffer;
    copyInfo.Size = nSize;
    copyInfo.IsWrite = FALSE;
    LOG_IF(copyInfo.ProcessId == 0, WARNING) << "ProcessId is 0";

    HANDLE hDevice = CreateFileW(
        DRIVER_DEVICE_PATH,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        0,
        OPEN_EXISTING,
        0, 0);
    if (hDevice == INVALID_HANDLE_VALUE) {
        LOG(ERROR) << L"Failed to open the device " << DRIVER_DEVICE_PATH
            << ". Error: " << GetLastError();
        result = FALSE;
    }

    if (result == TRUE) {
        if (!DeviceIoControl(
            hDevice,
            IOCTL_DRIVER_COPY_MEMORY,
            &copyInfo,
            sizeof(copyInfo),
            &copyInfo,
            sizeof(copyInfo),
            lpNumberOfBytesReturned,
            NULL
        )) {
            LOG(ERROR) << L"DeviceIoControl() failed with error: " << GetLastError();
            result = FALSE;
        }
        if (lpNumberOfBytesReturned)
            LOG(INFO) << "NumberOfBytesReturned: " << *lpNumberOfBytesReturned;
    }

    if (hDevice) {
        CloseHandle(hDevice);
    }
    return result;
}

BOOL WriteProcessMemoryDrivered(
    _In_ HANDLE hProcess,
    _In_ LPCVOID lpBaseAddress,
    _In_ LPVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ LPDWORD lpNumberOfBytesReturned)
{
    BOOL result = TRUE;
    DRIVER_COPY_MEMORY copyInfo = { 0 };
    if (lpNumberOfBytesReturned) {
        *lpNumberOfBytesReturned = 0;
    }
    LOG_IF(hProcess == NULL, ERROR) << "hProcess is NULL";

    copyInfo.ProcessId = GetProcessId(hProcess);
    copyInfo.Source = (ULONGLONG)lpBuffer;
    copyInfo.Target = (ULONGLONG)lpBaseAddress;
    copyInfo.Size = nSize;
    copyInfo.IsWrite = TRUE;
    LOG_IF(copyInfo.ProcessId == 0, WARNING) << "ProcessId is 0";

    HANDLE hDevice = CreateFileW(
        DRIVER_DEVICE_PATH,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        0,
        OPEN_EXISTING,
        0, 0);
    if (hDevice == INVALID_HANDLE_VALUE) {
        LOG(ERROR) << L"Failed to open the device " << DRIVER_DEVICE_PATH
            << ". Error: " << GetLastError();
        result = FALSE;
    }

    if (result == TRUE) {
        if (!DeviceIoControl(
            hDevice,
            IOCTL_DRIVER_COPY_MEMORY,
            &copyInfo,
            sizeof(copyInfo),
            &copyInfo,
            sizeof(copyInfo),
            lpNumberOfBytesReturned,
            NULL
        )) {
            LOG(ERROR) << L"DeviceIoControl() failed with error: " << GetLastError();
            result = FALSE;
        }
        if (lpNumberOfBytesReturned)
            LOG(INFO) << "NumberOfBytesReturned: " << *lpNumberOfBytesReturned;
    }

    if (hDevice) {
        CloseHandle(hDevice);
    }
    return result;
}
