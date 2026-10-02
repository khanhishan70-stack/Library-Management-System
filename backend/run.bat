@echo off
REM ==========================================================================
REM  run.bat  -  Starts the C++ backend. The browser is opened by the program
REM              itself, only after the server is really listening.
REM
REM  HOW TO USE :  double click this file
REM
REM  The program expects to find this folder structure:
REM      <project folder>\backend\library_server.exe
REM      <project folder>\backend\data\books.txt
REM      <project folder>\frontend\index.html
REM  ".." is given as the project folder, so the parent folder of this
REM  backend folder is used. That is exactly the project folder.
REM ==========================================================================
echo =============================================
echo   Starting the Library Management System
echo =============================================
echo.

cd /d "%~dp0"

set "NOPAUSE="
if /i "%~1"=="/nopause" set "NOPAUSE=1"

REM ---- If the program was not compiled yet, say so instead of failing silently ----
if not exist "library_server.exe" (
    echo library_server.exe was not found.
    echo Double click build.bat first, or start.bat in the project folder
    echo which does the build and the start in one go.
    echo.
    if not defined NOPAUSE pause
    exit /b 1
)

REM ---- Start the C++ web server (port 8080) ----
REM The program prints "Server started successfully" and then opens the
REM browser by itself, so the page can never load before the server answers.
library_server.exe ".." 8080

echo.
echo The server has stopped.
if not defined NOPAUSE pause
