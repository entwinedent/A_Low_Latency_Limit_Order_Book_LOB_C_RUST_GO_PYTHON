@echo off
setlocal enabledelayedexpansion
set ARGS=
:loop
if "%~1"=="" goto run
set ARGS=!ARGS! "%~1"
shift
goto loop
:run
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\19.51.36248\bin\Hostx64\x64\cl.exe" %ARGS%
exit /b %ERRORLEVEL%
