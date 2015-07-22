NAME
----
A Chess Suite

COMPILING
To compile in linux (tested in Ubuntu 10.10) you need:
-gcc compiler,
-QT SDK 4.7.0

In the root directory containing Asciiuichess, ChessPerfTest and AChess run "make all".
Executables and library are created in /src directories.
To compile GUI open ChessQMLui.pro with QT Creator and set build directory to its directory before compiling.

To compile in windows XP (tested in Windows XP professional SP3 version 2002) you need:
-MinGW (minimalist GNU for Windows) with MSYS
-Qt SDK 4.7.0 for windows

Mount root directory in MinGW shell and run "make all" in it, open ChessQMLui.pro in QT Creator and set build directory to its directory before compiling.