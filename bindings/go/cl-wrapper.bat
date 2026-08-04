@echo off
setlocal enabledelayedexpansion
set "COMPILE_MODE="
set "ARGS="
set "MSVC_CL=""
set "SDK_INCLUDE=C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0"
set "UCRT_INCLUDE=%SDK_INCLUDE%\ucrt"
set "UM_INCLUDE=%SDK_INCLUDE%\um"
set "SHARED_INCLUDE=%SDK_INCLUDE%\shared"
set "WINRT_INCLUDE=%SDK_INCLUDE%\winrt"
set "VC_INCLUDE=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include"

if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-cl.exe" (
    set "MSVC_CL=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-cl.exe"
) else if exist "C:\Program Files\LLVM\bin\clang-cl.exe" (
    set "MSVC_CL=C:\Program Files\LLVM\bin\clang-cl.exe"
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x64\cl.exe" (
    set "MSVC_CL=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x64\cl.exe"
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx86\x64\cl.exe" (
    set "MSVC_CL=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx86\x64\cl.exe"
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x86\cl.exe" (
    set "MSVC_CL=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x86\cl.exe"
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx86\x86\cl.exe" (
    set "MSVC_CL=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx86\x86\cl.exe"
)

if not defined MSVC_CL (
    echo cl.exe not found in the expected Visual Studio installation path>&2
    exit /b 1
)

:parse
if "%~1"=="" goto run
set "ARG=%~1"
if /I "%ARG%"=="-c" (
    set "COMPILE_MODE=/c"
    set "ARGS=!ARGS! /c"
) else if /I "%ARG%"=="-o" (
    shift
    if not "%~1"=="" (
        if defined COMPILE_MODE (
            set "ARGS=!ARGS! /Fo:%~1"
        ) else (
            set "ARGS=!ARGS! /Fe:%~1"
        )
    )
) else if /I "%ARG%"=="-shared" (
    set "ARGS=!ARGS! /LD"
) else if /I "%ARG%"=="-D" (
    shift
    if not "%~1"=="" set "ARGS=!ARGS! /D%~1"
) else if /I "%ARG%"=="-I" (
    shift
    if not "%~1"=="" set "ARGS=!ARGS! /I"%~1"
) else if /I "%ARG%"=="-isystem" (
    shift
    if not "%~1"=="" set "ARGS=!ARGS! /I"%~1"
) else if /I "%ARG%"=="-include" (
    shift
    if not "%~1"=="" set "ARGS=!ARGS! /FI"%~1"
) else if /I "%ARG%"=="-m64" (
    rem ignore
) else if /I "%ARG%"=="-mthreads" (
    rem ignore
) else if /I "%ARG%"=="-ffile-prefix-map" (
    shift
) else if /I "%ARG%"=="-gno-record-gcc-switches" (
    rem ignore
) else if /I "%ARG%"=="-fno-caret-diagnostics" (
    rem ignore
) else if /I "%ARG%"=="-fmessage-length=0" (
    rem ignore
) else if /I "%ARG%"=="-Qunused-arguments" (
    rem ignore
) else if /I "%ARG%"=="-fno-stack-protector" (
    rem ignore
) else if /I "%ARG%"=="-Werror" (
    rem ignore
) else if /I "%ARG%"=="-Wall" (
    rem ignore
) else if /I "%ARG%"=="-Wdeclaration-after-statement" (
    rem ignore
) else if /I "%ARG%"=="-Wno-error=unknown-argument" (
    rem ignore
) else if /I "%ARG%"=="-Wno-unknown-argument" (
    rem ignore
) else if /I "%ARG%"=="-Wno-unused-macros" (
    rem ignore
) else if /I "%ARG%"=="-Wno-unused-function" (
    rem ignore
) else if /I "%ARG%"=="-Wno-unused-parameter" (
    rem ignore
) else if /I "%ARG%"=="-Wno-unused-variable" (
    rem ignore
) else if /I "%ARG%"=="-Wno-unused-value" (
    rem ignore
) else if /I "%ARG%"=="-fms-compatibility" (
    rem ignore
) else if /I "%ARG%"=="-fdiagnostics-color=never" (
    rem ignore
) else if /I "%ARG%"=="-Wno-uninitialized" (
    rem ignore
) else if /I "%ARG%"=="-Wno-implicit-function-declaration" (
    rem ignore
) else if /I "%ARG%"=="-Wno-int-conversion" (
    rem ignore
) else if /I "%ARG%" NEQ "" (
    set "ARGS=!ARGS! "%ARG%""
)
shift
goto parse
:run
if not defined ARGS (
    exit /b 0
)
set "CL_ARGS=/nologo /EHsc /std:c++20 /D_WIN32 /I"%VC_INCLUDE%" /I"%UCRT_INCLUDE%" /I"%UM_INCLUDE%" /I"%SHARED_INCLUDE%" /I"%WINRT_INCLUDE%" !ARGS!"
call "%MSVC_CL%" !CL_ARGS!
exit /b %ERRORLEVEL%
