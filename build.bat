@echo off
title CortexOS Build
color 0A
echo.
echo  ============================================
echo   CortexOS - Build Script (MinGW GCC)
echo  ============================================
echo.

REM Check if gcc is available
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo  [ERROR] gcc not found. Please install MinGW-w64 and add to PATH.
    echo  Download: https://www.mingw-w64.org/
    pause
    exit /b 1
)

echo  [INFO] GCC found. Starting build...
echo.

REM Create data directories
if not exist "data" mkdir data
if not exist "data\mail" mkdir data\mail
if not exist "data\backups" mkdir data\backups
if not exist "data\databases" mkdir data\databases
if not exist "data\vfs" mkdir data\vfs

REM Compile all source files
set SRCS=src\main.c src\shell.c src\users.c src\security.c src\filesystem.c src\process.c src\scheduler.c src\memory.c src\network.c src\database.c src\blockchain.c src\backup.c src\apps.c src\logs.c
set CFLAGS=-Wall -Wextra -std=c99 -O2 -Isrc
set TARGET=CortexOS.exe

echo  Compiling...
gcc %CFLAGS% -o %TARGET% %SRCS% -lws2_32

if %errorlevel% neq 0 (
    echo.
    echo  [ERROR] Build failed! Check error messages above.
    pause
    exit /b 1
)

echo.
echo  ============================================
echo   Build SUCCESSFUL!  -  CortexOS.exe ready
echo  ============================================
echo.
echo  Run CortexOS.exe to start.
echo.
pause
