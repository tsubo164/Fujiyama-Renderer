@rem Copyright (c) 2011-2020 Hiroshi Tsubokawa
@rem See LICENSE and README

@echo off

echo ^> Setting up building environments ...
call setupenv_vs.bat
echo.

echo ^> Building Fujiyama Renderer ...
nmake
echo.

echo ^> Setting up Fujiyama environments ...
call setupenv_fujiyama.bat
echo.

echo ^> Moving to top folder ...
cd ..\..\
echo.

echo ^> Starting framebuffer viewer ...
start /b fbview --listen
echo.

echo.
echo ^> -------------------------------------------------------
echo ^> Run python scritps in scenes\ folder to render scenes.
echo ^> e.g. ^>scenes\teapot2.py
echo ^> -------------------------------------------------------
echo.

cmd /k
