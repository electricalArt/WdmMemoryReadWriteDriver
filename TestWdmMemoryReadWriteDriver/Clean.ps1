$TargetPath = "C:\DriverTest\Drivers\WdmMemoryReadWriteDriver\"

function Clean() {
    Write-Host
    Write-Host "Clean()"
    Write-Host

    Stop-Process -ProcessName "Fixed*" -Force

    sc.exe stop WdmMemoryReadWriteDriver
    sc.exe delete WdmMemoryReadWriteDriver

    New-Item -ItemType "Directory" -Path $TargetPath -Force `
        -ErrorAction "SilentlyContinue"
}
Clean
