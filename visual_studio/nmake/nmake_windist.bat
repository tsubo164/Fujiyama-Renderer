@echo off
echo Enter the version. e.g. 0.2.9
set /p FJ_DIST_VERSION=

call setupenv_python.bat
make_windist.py %FJ_DIST_VERSION%

pause
