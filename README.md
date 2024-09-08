# WdmMemoryReadWriteDriver

A Windows kernel-mode driver to perform read/write operations. It handles direct [IRP](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/handling-irps) requests generated
with [`DeviceIoControl()`](https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-deviceiocontrol).

The driver uses [Windows Driver Model](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/introduction-to-wdm) routines.

## Install

Build the `WdmMemoryReadWriteDriver` project and (optionally) the `TestWdmMemoryReadWriteDriver` project with Visual Studio. <br/>

Then, to be able to load the driver, you should enable [test signing](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/test-signing) on your target computer: <br/>
```PowerShell
bcdedit.exe -set TESTSIGNING ON
```

After that you have several options:

### Load the driver as average driver:

Press the Right Mouse Button on `.\x64\Release\WdmMemoryReadWriteDriver\WdmMemoryReadWriteDriver.inf`, then choose `Install` button.

### Loading the driver as service:
```PowerShell
  sc.exe create WdmMemoryReadWriteDriver type= kernel binpath= <full\path\to\WdmMemoryReadWriteDriver.sys> DisplayName= WdmMemoryReadWriteDriver`
  sc.exe start WdmMemoryReadWriteDriver`
```

### Deploying the driver as service on a target computer:
Start remote PS session to your target computer and use
```PowerShell
cd .\Deployment\
.\DeployDriver.ps1 <TargetPSSession> <TargetSessionPath>
```
or, if you want additionally to perfrom the test, run
```PowerShell
cd .\Deployment\
.\DeployAndTest.ps1 <TargetPSSession> <TargetSessionPath>
```

## Testing

To make sure that the driver is running properly on you computer, just execute ready-to-use script:
```PowerShell
cd .\TestWdmMemoryReadWriteDriver\
.\TestDriver.ps1
```
If everything is fine, you should spot `Success!` message on your terminal.

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

You can take a look into `TestWdmMemoryReadWriteDriver` if you want to receive ready-to-use `ReadProcessMemoryDrivered()` and `WriteProcessMemoryDrivered()` functions.

## Contributing

PRs are accepted.

## License
