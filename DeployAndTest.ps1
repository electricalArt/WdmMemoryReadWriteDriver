param(
    [Parameter(Mandatory)] [System.Management.Automation.Runspaces.PSSession] $TargetSession,
    [Parameter(Mandatory)] [string] $TargetSessionPath
)


try
{
    # Deploy
    .\DeployDriver.ps1 -TargetSession $TargetSession `
        -TargetSessionPath $TargetSessionPath
    
    # Test
    Invoke-Command -Session $TargetSession {
        cd $Using:TargetSessionPath
        .\TestDriver.ps1 -TargetPath $Using:TargetSessionPath
    }
    Write-Host "Success!"
}
catch
{
    Write-Error $_
}
