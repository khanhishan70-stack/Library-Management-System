@echo off
REM ==========================================================================
REM  build.bat  -  Compiles the C++ backend into library_server.exe
REM
REM  HOW TO USE :  double click this file (or run it from a command prompt)
REM
REM  It checks if the MSVC compiler (cl.exe) is available first.
REM  If not, it tries the MinGW compiler (g++).
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
REM ==========================================================================

echo =============================================
echo   Building the Library Management System
echo =============================================
echo.

cd /d "%~dp0"

REM ---- STEP 1 : look for the Microsoft compiler (cl.exe) ----
where cl >nul 2>nul
if %errorlevel%==0 goto buildWithMSVC

REM ---- STEP 2 : if not found, look for the MinGW compiler (g++) ----
where g++ >nul 2>nul
if %errorlevel%==0 goto buildWithMinGW

REM ---- STEP 3 : no compiler found, show the message ----
echo ERROR: No C++ compiler was found on this computer.
echo.
echo Please install one of these:
echo    1. Visual Studio Build Tools  (cl.exe)
echo    2. MinGW-w64                 (g++)
echo.
pause
exit /b 1

REM ---- BUILDING WITH THE MICROSOFT COMPILER ----
:buildWithMSVC
echo Using the Microsoft compiler (cl.exe) ...
echo.
cl /nologo /EHsc /std:c++17 /Fe:library_server.exe *.cpp
goto checkResult

REM ---- BUILDING WITH THE MinGW COMPILER ----
:buildWithMinGW
echo Using the MinGW compiler (g++) ...
echo.
g++ -std=c++17 -O2 -o library_server.exe *.cpp -lws2_32
goto checkResult

REM ---- CHECK IF THE BUILD WORKED ----
:checkResult
echo.
if errorlevel 1 (
    echo =============================================
    echo   BUILD FAILED - please read the errors above
    echo =============================================
    pause
    exit /b 1
)

echo =============================================
echo   BUILD SUCCESSFUL
echo   The file library_server.exe is ready.
echo =============================================
echo.
pause