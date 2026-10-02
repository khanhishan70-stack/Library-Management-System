@echo off
REM ==========================================================================
REM  build.bat  -  Compiles the C++ backend into library_server.exe
REM
REM  HOW TO USE :  double click this file (or run it from a command prompt)
REM
REM  STEP 1 : load the Visual Studio compiler environment, if it is installed
REM  STEP 2 : compile every .cpp file with the Microsoft compiler (cl.exe)
REM  STEP 3 : if cl.exe is still not there, try the MinGW compiler (g++)
REM  STEP 4 : if neither is installed, print what has to be installed
REM
REM  Running build.bat /nopause makes it finish without waiting for a key
REM  press. start.bat uses that, so one double click does everything.
REM
REM  Every .cpp file in this folder is compiled, so a new file is picked up
REM  automatically. One file holds one part of the project:
REM     server.cpp     starts the web server
REM     api.cpp        the /api routes
REM     http.cpp       building and reading the web answers
REM     storage.cpp    books.txt, members.txt, records.txt
REM     books.cpp      the book logic
REM     members.cpp    the member logic
REM     records.cpp    issue, return and the history
REM     dashboard.cpp  the four dashboard numbers
REM     helpers.cpp    text, JSON and date helpers
REM     digital.cpp    the bit, BCD and ASCII helpers
REM ==========================================================================

set "NOPAUSE="
if /i "%~1"=="/nopause" set "NOPAUSE=1"

REM Remember where the caller was, because "cd" below changes the folder for
REM good and start.bat checks for the exe by its own path after this returns.
set "ORIGINAL_DIR=%CD%"

echo =============================================
echo   Building the Library Management System
echo =============================================
echo.

cd /d "%~dp0"

REM ---- STEP 1 : is cl.exe already on the path? ----
where cl >nul 2>nul
if not errorlevel 1 goto buildWithMSVC

REM ---- STEP 1b : no. Look for Visual Studio and load its environment. ----
REM A double click does not start in the Visual Studio command prompt, so the
REM environment has to be loaded here, otherwise cl.exe is not found at all.
echo Looking for the Visual Studio compiler...

REM The brackets in "ProgramFiles(x86)" confuse the for command, so it is put
REM into a plain variable first.
set "PF86=%ProgramFiles(x86)%"
set "VCVARS="

for /f "delims=" %%f in ('dir /b /s "%PF86%\Microsoft Visual Studio\vcvars64.bat" 2^>nul') do (
    if not defined VCVARS set "VCVARS=%%f"
)
for /f "delims=" %%f in ('dir /b /s "%ProgramFiles%\Microsoft Visual Studio\vcvars64.bat" 2^>nul') do (
    if not defined VCVARS set "VCVARS=%%f"
)

if not defined VCVARS goto tryMinGW

echo   Found : %VCVARS%
call "%VCVARS%" >nul 2>&1

where cl >nul 2>nul
if not errorlevel 1 goto buildWithMSVC

REM ---- STEP 2 : still no cl.exe, so try MinGW ----
:tryMinGW
where g++ >nul 2>nul
if not errorlevel 1 goto buildWithMinGW

REM ---- STEP 3 : no compiler at all ----
echo.
echo ERROR: No C++ compiler was found on this computer.
echo.
echo Please install one of these:
echo    1. Visual Studio Build Tools  (cl.exe)
echo    2. MinGW-w64                 (g++)
echo.
echo Build Tools download:
echo    https://visualstudio.microsoft.com/visual-cpp-build-tools/
echo.
cd /d "%ORIGINAL_DIR%"
if not defined NOPAUSE pause
exit /b 1

REM ---- BUILDING WITH THE MICROSOFT COMPILER ----
:buildWithMSVC
echo Using the Microsoft compiler (cl.exe) ...
echo.
cl /nologo /EHsc /std:c++17 /W4 /Fe:library_server.exe *.cpp
goto checkResult

REM ---- BUILDING WITH THE MinGW COMPILER ----
:buildWithMinGW
echo Using the MinGW compiler (g++) ...
echo.
g++ -std=c++17 -O2 -Wall -o library_server.exe *.cpp -lws2_32
goto checkResult

REM ---- CHECK IF THE BUILD WORKED ----
:checkResult
echo.
cd /d "%ORIGINAL_DIR%"

if errorlevel 1 (
    echo =============================================
    echo   BUILD FAILED - please read the errors above
    echo =============================================
    if not defined NOPAUSE pause
    exit /b 1
)

echo =============================================
echo   BUILD SUCCESSFUL
echo   The file library_server.exe is ready.
echo =============================================
echo.
if not defined NOPAUSE pause
exit /b 0
