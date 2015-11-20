@rem Copyright (c) 2011-2015 Hiroshi Tsubokawa
@rem See LICENSE and README

@rem set PATH=%PATH%;%CD%\bin;C:\bin
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86_amd64


@echo off

:wait
echo target?
set /p CMD=

if "%CMD%"=="" (
goto build
) else if "%CMD%"=="clean" (
goto clean
) else if "%CMD%"=="q" (
goto end
) else (
goto wait
)

:build
nmake
goto wait

:clean
set CMD=
nmake clean
goto wait

:end
