$SolutionPath = ".."
$TargetProcessId = $null
$TargetPointer = "0x0000002000000000"  # Fixed
# TargetPointer should contain string of the pointer. Otherwise PowerShell will
#   immediately converse it to decimal representation (it leads to fail test). 

Write-Host 
Write-Host "TestDriver.ps1"
Write-Host (Get-Location)

function StartService() {
    Write-Host
    Write-Host "StartService()"
    Write-Host 
    $DriverFile = `
        (gi "$SolutionPath\x64\Release\WdmMemoryReadWriteDriver\WdmMemoryReadWriteDriver.sys").FullName
    Write-Host "Creating service for the driver. The driver file full name: " $DriverFile
    sc.exe create WdmMemoryReadWriteDriver type= kernel binpath= $DriverFile DisplayName= WdmMemoryReadWriteDriver
    sc.exe start WdmMemoryReadWriteDriver
    if ($LastExitCode) {
        throw
    }
}
function StartProcess() {
    Write-Host
    Write-Host "StartProcess()"
    Write-Host
    $script:TargetProcessId = `
        (Start-Process $SolutionPath\TestWdmMemoryReadWriteDriver\FixedMemoryApplication.exe `
            -ArgumentList int32 -PassThru
        ).Id
    if ($LastExitCode) {
        throw
    }
}
function TestDriverReadAbility() {
    Write-Host
    Write-Host "TestDriverReadAbility()"
    Write-Host
    & $SolutionPath\x64\Release\TestWdmMemoryReadWriteDriver.exe read $TargetProcessId $TargetPointer
    if ($LastExitCode) {
        throw
    }
}
function TestDriverWriteAbility() {
    Write-Host
    Write-Host "TestDriverWriteAbility()"
    Write-Host
    & $SolutionPath\x64\Release\TestWdmMemoryReadWriteDriver.exe write $TargetProcessId $TargetPointer 1500
    if ($LastExitCode) {
        throw
    }
}
function StopProcess() {
    Write-Host
    Write-Host "StopProcess()"
    Write-Host
    Stop-Process -Id $TargetProcessId
}

try
{
    StartService
    StartProcess
    TestDriverReadAbility
    TestDriverWriteAbility
    StopProcess
    Write-Host "Success!"
}
catch
{
    Write-Host $_
}
