# build.ps1 — configure + build CVM++ via CMake, then copy cvm.exe to project root.
# Usage:   .\build.ps1            (Release)
#          .\build.ps1 Debug      (Debug)
#          .\build.ps1 -Clean     (wipe build dir first)

param(
    [string]$Config = "Release",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectRoot

# Require cmake on PATH — no hardcoded install paths.
$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $cmake) {
    throw "CMake not found on PATH. Install CMake (winget install Kitware.CMake) or add it to PATH."
}

if ($Clean -and (Test-Path build)) {
    Write-Host "Cleaning build/..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force build
}

Write-Host "Configuring ($Config)..." -ForegroundColor Cyan
& $cmake -B build -S . | Out-Host

Write-Host "Building..." -ForegroundColor Cyan
& $cmake --build build --config $Config | Out-Host

# MSVC drops the exe under build/<Config>/; single-config generators put it in build/.
$candidates = @(
    "build\$Config\cvm.exe",
    "build\cvm.exe"
)
$exe = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $exe) { throw "Build succeeded but cvm.exe not found." }

Copy-Item $exe ".\cvm.exe" -Force
Write-Host "`nBuilt: .\cvm.exe" -ForegroundColor Green
Write-Host "Try:   .\cvm.exe examples\loop.cvm" -ForegroundColor Green
