@echo off
REM One-click launcher for the Low-Latency Order Book Engine build and run script
REM This batch file executes the PowerShell script with proper permissions

echo Starting Low-Latency Order Book Engine Build and Run...
echo.

REM Run the PowerShell script
powershell.exe -ExecutionPolicy Bypass -File "%~dp0build_and_run.ps1"

REM Keep window open if there was an error
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed with error code: %ERRORLEVEL%
    echo Press any key to exit...
    pause >nul
)
