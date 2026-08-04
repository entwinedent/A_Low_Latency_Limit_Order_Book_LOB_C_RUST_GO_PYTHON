@echo off
setlocal enabledelayedexpansion
set "ARGS="
:loop
if "%~1"=="" goto run
set "ARG=%~1"
if /I "%ARG%"=="-mthreads" (
    rem ignore unsupported cgo flag
) else if /I "%ARG%"=="-g" (
    rem ignore
) else if /I "%ARG%"=="-frandom-seed" (
    rem ignore
) else if /I "%ARG%"=="-fno-caret-diagnostics" (
    rem ignore
) else if /I "%ARG%"=="-Werror" (
    rem ignore
) else if /I "%ARG%"=="-Wall" (
    rem ignore
) else if /I "%ARG%"=="-Wno-error=unknown-argument" (
    rem ignore
) else if /I "%ARG%"=="-Wno-unknown-argument" (
    rem ignore
) else if /I "%ARG%"=="-dM" (
    rem ignore
) else if /I "%ARG%"=="-E" (
    set "ARGS=!ARGS! /E"
) else if /I "%ARG%"=="-c" (
    set "ARGS=!ARGS! /c"
) else if /I "%ARG%"=="-o" (
    shift
    if not "%~1"=="" set "ARGS=!ARGS! /Fo%~1"
) else if /I "%ARG%"=="-I" (
    shift
    if not "%~1"=="" set "ARGS=!ARGS! /I"%~1"
) else if /I "%ARG%"=="-D" (
    shift
    if not "%~1"=="" set "ARGS=!ARGS! /D%~1"
) else if /I "%ARG%"=="-include" (
    shift
    if not "%~1"=="" set "ARGS=!ARGS! /FI"%~1"
) else (
    set "ARGS=!ARGS! "%ARG%""
)
shift
goto loop
:run
if not defined ARGS (
    exit /b 0
)
"C:\Program Files\LLVM\bin\clang-cl.exe" !ARGS!
exit /b %ERRORLEVEL%
