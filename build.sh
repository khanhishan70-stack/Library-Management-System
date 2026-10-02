#!/usr/bin/env bash
# ==========================================================================
#  build.sh  -  compiles the C++ backend on Linux / macOS / Codespaces.
#
#  This is the friend of build.bat. The .bat file cannot run on a Codespace
#  because a Codespace is Linux, so this file does the same job with g++.
#
#  HOW TO USE :  bash build.sh
#
#  NOTE: the "-lws2_32" library is only needed on Windows (MinGW). On Linux
#  the socket functions live in the normal C library, so nothing extra is
#  linked here.
# ==========================================================================

set -e
cd "$(dirname "$0")/backend"

echo "============================================="
echo "  Building the Library Management System"
echo "============================================="
echo

if ! command -v g++ >/dev/null 2>&1; then
    echo "ERROR: g++ was not found on this computer."
    echo "Install it with:  sudo apt-get install -y g++"
    exit 1
fi

echo "Using the GNU compiler (g++) ..."
echo

# every .cpp file in this folder is compiled, so a new file is picked up
# automatically, exactly like build.bat does on Windows
g++ -std=c++17 -O2 -Wall -o library_server *.cpp

echo
echo "============================================="
echo "  BUILD SUCCESSFUL"
echo "  The file library_server is ready."
echo "============================================="
