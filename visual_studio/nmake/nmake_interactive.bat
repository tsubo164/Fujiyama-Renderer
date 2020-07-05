@rem Copyright (c) 2011-2020 Hiroshi Tsubokawa
@rem See LICENSE and README

call setupenv_vs.bat

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
