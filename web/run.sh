#!/usr/bin/env bash
# run.sh — start the CVM++ web visualizer.
set -euo pipefail
cd "$(dirname "$0")"

PY="$(command -v python3 || command -v python || true)"
if [[ -z "$PY" ]]; then
    echo "Python not found. Install Python 3.10+." >&2
    exit 1
fi

if [[ ! -d .venv ]]; then
    echo "Creating virtualenv..."
    "$PY" -m venv .venv
fi

VENV_PY=".venv/bin/python"
[[ -x "$VENV_PY" ]] || VENV_PY=".venv/Scripts/python.exe"  # Git Bash on Windows

"$VENV_PY" -m pip install --quiet --disable-pip-version-check -r requirements.txt

echo "Server starting on http://127.0.0.1:8000  (Ctrl+C to stop)"
"$VENV_PY" -m uvicorn app:app --host 127.0.0.1 --port 8000 --reload
