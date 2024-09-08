param(
    [Parameter(Mandatory)][System.Management.Automation.Runspaces.PSSession]$TargetSession,
    [Parameter(Mandatory)][string]$TargetSessionPath
)

if (!($TargetSession) -or ($TargetSession.State -eq "Broken")) {
    throw "The session is not exist or it is broken"
}

$TargetSessionPath = $TargetSessionPath
$TargetSession = $TargetSession
$ItemsToDeploy = @(
    ".\x64\Release\*.exe",
    ".\TestWdmMemoryReadWriteDriver\FixedMemoryApplication.exe",
    ".\x64\Release\WdmMemoryReadWriteDriver\",
    "*.ps1")

try
{
    Invoke-Command -Session $TargetSession {
        & $Using:TargetSessionPath\Clean.ps1
    }

    $ItemsToDeploy | Foreach-Object {
        Write-Host "Deploying item: " $_
        Copy-Item -Recurse -Force -Path $_ -ToSession $TargetSession -Destination $TargetSessionPath
    }
}
catch
{
    Write-Error $_
}
