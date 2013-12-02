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
ECHO                     Database to SQL Converter
ECHO                     http://hercules.ws/board/
ECHO.
ECHO.

ECHO Exporting item databases to 'sql-files' folder...
PING -n 3 -w 1 127.0.0.1 > nul

map-server.exe --db2sql
ECHO.

PING -n 10 -w 1 127.0.0.1 > nul
