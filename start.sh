#!/usr/bin/env bash
# ==========================================================================
#  start.sh  -  builds if needed, then starts the server on a Codespace.
#
#  This is the friend of start.bat. It does the same three steps in the same
#  order:
#      1. look for backend/library_server
#      2. if it is missing, build it with build.sh
#      3. start the server
#
#  The third argument "no-browser" is passed to the program so it does not try
#  to open a browser. A Codespace has no browser of its own, and the link in
#  the PORTS panel is what the visitor uses instead.
#
#  HOW TO USE :  bash start.sh
# ==========================================================================

cd "$(dirname "$0")"

PORT="${PORT:-8080}"

echo "========================================================="
echo "  LIBRARY MANAGEMENT SYSTEM  -  starting everything"
echo "========================================================="
echo

# ---------------------------------------------------------------- step 1
if [ -f "backend/library_server" ]; then
    echo "[1 of 2] The program is already compiled. Skipping the build."
else
    # ------------------------------------------------------------ step 2
    echo "[1 of 2] The program is not compiled yet. Building it now..."
    echo
    bash build.sh
fi

# ---------------------------------------------------------------- step 3
echo
echo "[2 of 2] Starting the server on port $PORT..."
echo

cd backend
./library_server .. "$PORT" no-browser
