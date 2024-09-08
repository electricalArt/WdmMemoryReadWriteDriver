param(
    [Parameter(Mandatory)][System.Management.Automation.Runspaces.PSSession]$TargetSession,
    [Parameter(Mandatory)][string]$TargetSessionPath
)
$SolutionPath = ".."
$ItemsToDeploy = @(
    "$SolutionPath\TestWdmMemoryReadWriteDriver",
    "$SolutionPath\x64",
    "$SolutionPath\Deployment"
)

Write-Host
Write-Host "DeployDriver.ps1"
Write-Host

if (!($TargetSession) -or ($TargetSession.State -eq "Broken")) {
    throw "The session is not exist or it is broken"
}

try
{
    Invoke-Command -Session $TargetSession {
        $CleanScriptPath = "$Using:TargetSessionPath\TestWdmMemoryReadWriteDriver\Clean.ps1"
        if (Test-Path $CleanScriptPath) {
            Invoke-Expression $CleanScriptPath
        }
    }

    $ItemsToDeploy | Foreach-Object {
        Write-Host "Deploying item: " $_
        Copy-Item -Recurse -Force -Exclude "*.swp" `
            -Path $_ -ToSession $TargetSession -Destination $TargetSessionPath
    }
}
catch
{
    Write-Error $_
}
