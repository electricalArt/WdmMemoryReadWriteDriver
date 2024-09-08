param(
    [Parameter(Mandatory)] [string] $TargetPath
)

$TargetProcessId = $null
$TargetPointer = "0x0000002000000000"  # Fixed
# TargetPointer should contain string of the pointer. Otherwise PowerShell will
#   immediately converse it to decimal representation (it lead to fail test). 

function StartService() {
    Write-Host
    Write-Host "StartService()"
    Write-Host
    $DriverFile = "$TargetPath\WdmMemoryReadWriteDriver\WdmMemoryReadWriteDriver.sys"
    Write-Host "Creating service for the driver. The driver file full name: " $DriverFile
    sc.exe create WdmMemoryReadWriteDriver type= kernel binpath= $DriverFile DisplayName= WdmMemoryReadWriteDriver
    sc.exe start WdmMemoryReadWriteDriver
}
function StartProcess() {
    Write-Host
    Write-Host "StartProcess()"
    Write-Host
    $script:TargetProcessId = (Start-Process .\FixedMemoryApplication.exe -ArgumentList int32 -PassThru).Id
}
function TestDriverReadAbility() {
    Write-Host
    Write-Host "TestDriverReadAbility()"
    Write-Host
    cd $TargetPath
    .\TestWdmMemoryReadWriteDriver.exe read $TargetProcessId $TargetPointer
}
function TestDriverWriteAbility() {
    Write-Host
    Write-Host "TestDriverWriteAbility()"
    Write-Host
    cd $TargetPath
    .\TestWdmMemoryReadWriteDriver.exe write $TargetProcessId $TargetPointer 1500
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
}
catch
{
    Write-Host $_
}
