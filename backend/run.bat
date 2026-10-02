@echo off
REM ==========================================================================
REM  run.bat  -  Starts the C++ backend and opens the project in the browser
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

REM ---- If the program was not compiled yet, tell the student ----
if not exist "library_server.exe" (
    echo library_server.exe was not found.
    echo Please double click build.bat first to compile the C++ code.
    echo.
    pause
    exit /b 1
)

REM ---- Open the browser after 2 seconds, then start the server ----
start "" /min cmd /c "timeout /t 2 >nul & start http://localhost:8080"

REM ---- Start the C++ web server (port 8080) ----
library_server.exe ".." 8080

echo.
echo The server has stopped.
pause