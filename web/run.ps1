# run.ps1 — start the CVM++ web visualizer.
# Usage: from project root, run:  .\web\run.ps1
$ErrorActionPreference = "Stop"
$webDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $webDir

# Find python
$py = (Get-Command python -ErrorAction SilentlyContinue).Source
if (-not $py) { $py = (Get-Command py -ErrorAction SilentlyContinue).Source }
if (-not $py) { throw "Python not found. Install Python 3.10+." }

# Create venv if missing
if (-not (Test-Path .venv)) {
    Write-Host "Creating virtualenv..." -ForegroundColor Cyan
    & $py -m venv .venv
}

$venvPy = ".\.venv\Scripts\python.exe"
& $venvPy -m pip install --quiet --disable-pip-version-check -r requirements.txt

Write-Host "Server starting on http://127.0.0.1:8000  (Ctrl+C to stop)" -ForegroundColor Green
& $venvPy -m uvicorn app:app --host 127.0.0.1 --port 8000 --reload
