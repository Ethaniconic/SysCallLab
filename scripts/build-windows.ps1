param(
    [string]$Configuration = "Release",
    [string]$OutputDirectory = "release\SysCallLab"
)

$ErrorActionPreference = "Stop"

$repositoryRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$buildDirectory = Join-Path $repositoryRoot "build-windows"
$outputPath = Join-Path $repositoryRoot $OutputDirectory

Write-Host "Configuring SysCallLab..."
cmake -S $repositoryRoot -B $buildDirectory

Write-Host "Building SysCallLab ($Configuration)..."
cmake --build $buildDirectory --config $Configuration --parallel

if (Test-Path $outputPath) {
    Remove-Item $outputPath -Recurse -Force
}

New-Item -ItemType Directory -Path $outputPath -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $outputPath "data") -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $outputPath "bin") -Force | Out-Null

$runtimeDirectory = Join-Path $buildDirectory "bin"
$configurationDirectory = Join-Path $buildDirectory $Configuration
if (Test-Path (Join-Path $configurationDirectory "SysCallLab.exe")) {
    $runtimeDirectory = $configurationDirectory
}

Copy-Item (Join-Path $runtimeDirectory "SysCallLab.exe") $outputPath
Copy-Item (Join-Path $runtimeDirectory "test_child.exe") (Join-Path $outputPath "bin")
Copy-Item (Join-Path $runtimeDirectory "test_executable.exe") (Join-Path $outputPath "bin")
Copy-Item (Join-Path $repositoryRoot "frontend") $outputPath -Recurse

@"
@echo off
setlocal
cd /d "%~dp0"
start "" SysCallLab.exe 8080
timeout /t 2 /nobreak >nul
start "" http://127.0.0.1:8080
"@ | Set-Content (Join-Path $outputPath "Start-SysCallLab.bat") -Encoding ASCII

@"
SysCallLab portable Windows package

Run Start-SysCallLab.bat to start the local server and open the dashboard.
Run SysCallLab.exe 8080 from this directory to start the server without opening a browser.
"@ | Set-Content (Join-Path $outputPath "README.txt") -Encoding ASCII

Write-Host ""
Write-Host "Portable package created at: $outputPath"
