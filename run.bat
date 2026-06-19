@echo off
title CortexOS
color 0A
call build.bat
if exist CortexOS.exe (
    echo  Launching CortexOS...
    echo.
    CortexOS.exe
)
