@echo off
REM ==========================================================================
REM  start.bat  -  One click starts the whole project.
REM
REM  HOW TO USE :  double click this file. Nothing else is needed.
REM
REM  WHAT IT DOES, IN THE RIGHT ORDER:
REM      1. looks for backend\library_server.exe
REM      2. if it is missing, runs backend\build.bat to compile the C++ code
REM      3. runs backend\run.bat, which starts the server
REM      4. the server opens the website by itself once it is listening
REM
REM  Step 4 is the important one. The old run.bat waited 2 seconds and opened
REM  the browser, which sometimes gave a "cannot reach the server" page. Now
REM  the C++ program opens the browser only after bind() and listen() have both
REM  succeeded, so the server is always ready before the page is shown.
REM ==========================================================================

echo ==========================================================
echo   LIBRARY MANAGEMENT SYSTEM  -  starting everything
echo ==========================================================
echo.

cd /d "%~dp0"

REM Full paths are used everywhere below, because build.bat changes the working
REM folder while it compiles. A relative path would then look in the wrong place.
set "EXE=%~dp0backend\library_server.exe"
set "BUILD_BAT=%~dp0backend\build.bat"
set "RUN_BAT=%~dp0backend\run.bat"

REM ---------------------------------------------------------------- step 1
if exist "%EXE%" (
    echo [1 of 2] The program is already compiled. Skipping the build.
    goto :run
)

REM ---------------------------------------------------------------- step 2
echo [1 of 2] The program is not compiled yet. Building it now...
echo         This takes about 20 seconds and happens only once.
echo.

call "%BUILD_BAT%" /nopause

if not exist "%EXE%" (
    echo.
    echo ==========================================================
    echo  BUILD FAILED - the server cannot be started.
    echo.
    echo  Most likely cause: the C++ compiler is not installed.
    echo  Install "Visual Studio Build Tools" or "MinGW-w64",
    echo  then double click this file again.
    echo ==========================================================
    echo.
    pause
    exit /b 1
)

REM ---------------------------------------------------------------- step 3
:run
echo.
echo [2 of 2] Starting the server...
echo.

call "%RUN_BAT%" /nopause
