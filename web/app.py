"""FastAPI server for the CVM++ web visualizer.

Pipes user code into the compiled `cvm.exe --json <file>` binary and forwards
the resulting JSON (tokens / AST / bytecode / output / error) to the browser.
"""
from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

from fastapi import FastAPI
from fastapi.responses import JSONResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel

ROOT = Path(__file__).resolve().parent.parent
CVM_BIN = ROOT / ("cvm.exe" if os.name == "nt" else "cvm")
STATIC_DIR = Path(__file__).resolve().parent / "static"

app = FastAPI(title="CVM++ visualizer")


class AnalyzeRequest(BaseModel):
    code: str
    stdin: str = ""  # optional input fed to the program's `input` statements


@app.post("/api/analyze")
def analyze(req: AnalyzeRequest):
    if not CVM_BIN.exists():
        return JSONResponse(
            {
                "ok": False,
                "error": {
                    "stage": "server",
                    "message": (
                        f"cvm binary not found at {CVM_BIN}. "
                        f"Build it first with `./build.ps1` or `./build.sh`."
                    ),
                },
            },
            status_code=500,
        )

    # Write user code to a temp .cvm file so the binary can read it
    # (stdin is reserved for the program's `input` statements).
    tmp_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            "w", suffix=".cvm", delete=False, encoding="utf-8"
        ) as f:
            f.write(req.code)
            tmp_path = Path(f.name)

        proc = subprocess.run(
            [str(CVM_BIN), "--json", str(tmp_path)],
            input=req.stdin,
            capture_output=True,
            text=True,
            timeout=10,
        )

        if not proc.stdout.strip():
            return {
                "ok": False,
                "error": {
                    "stage": "server",
                    "message": f"cvm produced no output. stderr: {proc.stderr[:500]}",
                },
            }

        try:
            return json.loads(proc.stdout)
        except json.JSONDecodeError as e:
            return {
                "ok": False,
                "error": {
                    "stage": "server",
                    "message": f"Malformed JSON from cvm: {e}",
                    "raw": proc.stdout[:1000],
                },
            }
    except subprocess.TimeoutExpired:
        return {
            "ok": False,
            "error": {
                "stage": "vm",
                "message": "execution timed out (possible infinite loop)",
            },
        }
    finally:
        if tmp_path and tmp_path.exists():
            try:
                tmp_path.unlink()
            except OSError:
                pass


# Serve the static frontend at the root.
app.mount("/", StaticFiles(directory=STATIC_DIR, html=True), name="static")


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(app, host="127.0.0.1", port=8000)
