"""Read terminal++ tabs and send text through its local Windows control pipe.

Example::

    import terminalpp_control as terminal

    for tab in terminal.list_tabs():
        print(tab["id"], tab["title"])
        print(tab["text"])

    terminal.send_text(0, "echo hello", enter=True)

The pipe is available while terminal++ is running. It is restricted to the
current Windows user and accepts one JSON request per connection.
"""

import ctypes
import json
import os
import shutil
import subprocess
import base64
import time


_PIPE = r"\\.\pipe\terminalpp-control"
_GENERIC_READ = 0x80000000
_GENERIC_WRITE = 0x40000000
_OPEN_EXISTING = 3
_ERROR_PIPE_BUSY = 231
_ERROR_FILE_NOT_FOUND = 2
_INVALID_HANDLE_VALUE = ctypes.c_void_p(-1).value

_kernel32 = None
if os.name == "nt":
    from ctypes import wintypes

    _kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    _kernel32.CreateFileW.argtypes = (
        wintypes.LPCWSTR,
        wintypes.DWORD,
        wintypes.DWORD,
        wintypes.LPVOID,
        wintypes.DWORD,
        wintypes.DWORD,
        wintypes.HANDLE,
    )
    _kernel32.CreateFileW.restype = wintypes.HANDLE
    _kernel32.WaitNamedPipeW.argtypes = (wintypes.LPCWSTR, wintypes.DWORD)
    _kernel32.WaitNamedPipeW.restype = wintypes.BOOL
    _kernel32.WriteFile.argtypes = (
        wintypes.HANDLE,
        ctypes.c_void_p,
        wintypes.DWORD,
        ctypes.POINTER(wintypes.DWORD),
        wintypes.LPVOID,
    )
    _kernel32.WriteFile.restype = wintypes.BOOL
    _kernel32.ReadFile.argtypes = (
        wintypes.HANDLE,
        wintypes.LPVOID,
        wintypes.DWORD,
        ctypes.POINTER(wintypes.DWORD),
        wintypes.LPVOID,
    )
    _kernel32.ReadFile.restype = wintypes.BOOL
    _kernel32.CloseHandle.argtypes = (wintypes.HANDLE,)
    _kernel32.CloseHandle.restype = wintypes.BOOL


def _connect():
    deadline = time.monotonic() + 5.0
    while True:
        handle = _kernel32.CreateFileW(
            _PIPE,
            _GENERIC_READ | _GENERIC_WRITE,
            0,
            None,
            _OPEN_EXISTING,
            0,
            None,
        )
        if handle != _INVALID_HANDLE_VALUE:
            return handle
        error = ctypes.get_last_error()
        if error in (_ERROR_PIPE_BUSY, _ERROR_FILE_NOT_FOUND) and time.monotonic() < deadline:
            if error == _ERROR_PIPE_BUSY:
                _kernel32.WaitNamedPipeW(_PIPE, 100)
            else:
                time.sleep(0.1)
            continue
        if error == _ERROR_FILE_NOT_FOUND:
            message = "No terminal++ control pipe found. Start the rebuilt terminalpp.exe and try again."
        else:
            message = "Could not connect to terminal++"
        raise ctypes.WinError(error, message)


def _request(message):
    payload = (json.dumps(message, ensure_ascii=True) + "\n").encode("ascii")
    if os.name != "nt":
        return _request_via_powershell(payload)

    handle = _connect()
    try:
        offset = 0
        while offset < len(payload):
            chunk = payload[offset:]
            buffer = ctypes.create_string_buffer(chunk)
            written = wintypes.DWORD()
            if not _kernel32.WriteFile(
                handle,
                ctypes.cast(buffer, ctypes.c_void_p),
                len(chunk),
                ctypes.byref(written),
                None,
            ):
                raise ctypes.WinError(ctypes.get_last_error(), "Could not send request to terminal++")
            if written.value == 0:
                raise OSError("terminal++ closed the control pipe while sending")
            offset += written.value

        response = bytearray()
        while True:
            buffer = ctypes.create_string_buffer(65536)
            received = wintypes.DWORD()
            if not _kernel32.ReadFile(
                handle,
                buffer,
                len(buffer),
                ctypes.byref(received),
                None,
            ):
                raise ctypes.WinError(ctypes.get_last_error(), "Could not read response from terminal++")
            if received.value == 0:
                raise OSError("terminal++ closed the control pipe before replying")
            response.extend(buffer.raw[: received.value])
            newline = response.find(b"\n")
            if newline >= 0:
                return _decode_response(response[:newline])
    finally:
        _kernel32.CloseHandle(handle)


def _decode_response(response):
    result = json.loads(response.decode("utf-8"))
    if "error" in result:
        raise RuntimeError(result["error"])
    return result


def _request_via_powershell(payload):
    """Use the Windows pipe API through PowerShell for Cygwin Python."""
    powershell = (
        shutil.which("pwsh.exe")
        or shutil.which("pwsh")
        or shutil.which("powershell.exe")
        or shutil.which("powershell")
    )
    if not powershell:
        raise RuntimeError("Cygwin Python needs pwsh.exe or powershell.exe on PATH")

    script = r'''
$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
$request = [Console]::In.ReadLine()
$pipe = [System.IO.Pipes.NamedPipeClientStream]::new('.', 'terminalpp-control', [System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(5000)
$encoding = [System.Text.UTF8Encoding]::new($false)
$reader = [System.IO.StreamReader]::new($pipe, $encoding, $false, 65536, $true)
$writer = [System.IO.StreamWriter]::new($pipe, $encoding, 65536, $true)
$writer.NewLine = "`n"
$writer.WriteLine($request)
$writer.Flush()
$response = $reader.ReadLine()
[Console]::Out.WriteLine($response)
$reader.Dispose()
$writer.Dispose()
$pipe.Dispose()
'''
    encoded_script = base64.b64encode(script.encode("utf-16le")).decode("ascii")
    completed = subprocess.run(
        [powershell, "-NoLogo", "-NoProfile", "-NonInteractive", "-EncodedCommand", encoded_script],
        input=payload,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=120,
        check=False,
    )
    if completed.returncode != 0:
        message = completed.stderr.decode("utf-8", errors="replace").strip()
        raise RuntimeError(message or "PowerShell could not connect to terminal++")
    response = completed.stdout.split(b"\n", 1)[0].rstrip(b"\r")
    if not response:
        raise RuntimeError("PowerShell returned an empty response from terminal++")
    return _decode_response(response)


def list_tabs():
    """Return tab metadata and text for every open tab."""
    return _request({"op": "list_tabs"})["tabs"]


def read_tab(tab_id):
    """Return the title, text, and physical text rows for one zero-based tab id."""
    return _request({"op": "read_tab", "id": int(tab_id)})


def read_all_tabs():
    """Return full text snapshots for all open tabs."""
    return _request({"op": "read_all"})["tabs"]


def send_text(tab_id, text, enter=False):
    """Send text to a tab; submit it only when ``enter=True``."""
    return _request({"op": "send_text", "id": int(tab_id), "text": str(text), "enter": bool(enter)})
