# WdmMemoryReadWriteDriver

A windows kernel-mode driver to perform read/write operations. It handles direct IRP requests generated
with `DeviceIoControl()`.

The driver uses Windows Driver Model routines.

## Install

Build the `WdmMemoryReadWriteDriver` project and (optionally) `TestWdmMemoryReadWriteDriver` project with Visual Studio. <br/>
<br/>
After that you have several options.

You can load the driver as service:
* Start remote PS session to your target computer and use <br/>
  `.\DeployDriver.ps1 <TargetPSSession> <TargetSessionPath>` <br/>
  or <br/>
  `.\DeployAndTest.ps1 <TargetPSSession> <TargetSessionPath>` <br/>
  script.
* Or, if you prefer, start the driver as service. Run the following commands in PowerShell either Command prompt:
  * Enable testsigning to be able to load that unsigned driver: <br/>
    `bcdedit.exe -set TESTSIGNING ON`
  * Restart your computer.
  * Create service: <br/>
    `sc.exe create WdmMemoryReadWriteDriver type= kernel binpath= <full\path\to\WdmMemoryReadWriteDriver.sys> DisplayName= WdmMemoryReadWriteDriver`
  * Start service: <br/>
    `sc.exe start WdmMemoryReadWriteDriver`

You also can load the driver as average driver:
* Press the Right Mouse Button on `WdmMemoryReadWriteDriver.inf`, then choose Install.

## Usage

Using the driver:
* Sending request to driver to read from specfied process memory:
```C++
DRIVER_COPY_MEMORY copyInfo = { 0 };
copyInfo.ProcessId = GetProcessId(hProcess);
copyInfo.Source = (ULONGLONG)lpBaseAddress;
copyInfo.Target = (ULONGLONG)lpBuffer;
copyInfo.Size = nSize;
copyInfo.IsWrite = FALSE;

// Sending request to driver
DeviceIoControl(
    hDevice,
    IOCTL_DRIVER_COPY_MEMORY,
    &copyInfo,
    sizeof(copyInfo),
    &copyInfo,
    sizeof(copyInfo),
    lpNumberOfBytesReturned,
    NULL);
```
* Sending request to driver to write to specified process memory:
```C++
DRIVER_COPY_MEMORY copyInfo = { 0 };
copyInfo.ProcessId = GetProcessId(hProcess);
copyInfo.Source = (ULONGLONG)lpBuffer;
copyInfo.Target = (ULONGLONG)lpBaseAddress;
copyInfo.Size = nSize;
copyInfo.IsWrite = TRUE;

// Sending request to driver
DeviceIoControl( 
    hDevice,
    IOCTL_DRIVER_COPY_MEMORY,
    &copyInfo,
    sizeof(copyInfo),
    &copyInfo,
    sizeof(copyInfo),
    lpNumberOfBytesReturned,
    NULL);
```

## Contributing

PRs are accepted.

## License
