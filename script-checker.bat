@ECHO OFF

REM Copyright (c) Hercules Dev Team, licensed under GNU GPL.
REM See the LICENSE file
REM Base Author: Mumbles @ http://hercules.ws

COLOR 0F

ECHO.
ECHO                  Hercules Development Team presents
ECHO                 _   _                     _
ECHO                ^| ^| ^| ^|                   ^| ^|
ECHO                ^| ^|_^| ^| ___ _ __ ___ _   _^| ^| ___  ___
ECHO                ^|  _  ^|/ _ \ '__/ __^| ^| ^| ^| ^|/ _ \/ __^|
ECHO                ^| ^| ^| ^|  __/ ^| ^| (__^| ^|_^| ^| ^|  __/\__ \
ECHO                \_^| ^|_/\___^|_^|  \___^|\__,_^|_^|\___^|^|___/
ECHO.
ECHO                       Script Syntax Checker
ECHO                     http://hercules.ws/board/
ECHO.
ECHO                  Drag and drop or input manually
ECHO.
ECHO.

:LOOP
	SET /P SCRIPT="Enter path/to/your/script.txt: " %=%
	map-server.exe --script-check --load-script %SCRIPT%
	ECHO.
GOTO LOOP
