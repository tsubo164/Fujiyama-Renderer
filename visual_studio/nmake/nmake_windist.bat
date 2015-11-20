@rem Copyright (c) 2011-2015 Hiroshi Tsubokawa
@rem See LICENSE and README

@echo off
echo Enter the version. e.g. 0.2.9
set /p FJ_DIST_VERSION=

call setupenv_vs2012.bat
nmake

call setupenv_python.bat
make_windist.py %FJ_DIST_VERSION%

pause
