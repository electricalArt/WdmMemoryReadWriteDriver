param(
    [Parameter(Mandatory)] [System.Management.Automation.Runspaces.PSSession] $TargetSession,
    [Parameter(Mandatory)] [string] $TargetSessionPath
)
$SolutionDir = ".."

Write-Host
Write-Host "DeployAndTest.ps1"
Write-Host

try
{
    # Deploy
    .\DeployDriver.ps1 -TargetSession $TargetSession `
        -TargetSessionPath $TargetSessionPath
    
    # Test
    Invoke-Command -Session $TargetSession {
        cd $Using:TargetSessionPath\TestWdmMemoryReadWriteDriver
        .\TestDriver.ps1 -TargetPath $Using:TargetSessionPath
    }
}
catch
{
    Write-Error $_
}
