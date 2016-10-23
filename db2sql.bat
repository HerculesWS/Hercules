@ECHO OFF

REM This file is part of Hercules.
REM http://herc.ws - http://github.com/HerculesWS/Hercules
REM
REM Copyright (C) 2013-2015  Hercules Dev Team
REM
REM Hercules is free software: you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation, either version 3 of the License, or
REM (at your option) any later version.
REM
REM This program is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.
REM
REM You should have received a copy of the GNU General Public License
REM along with this program.  If not, see <http://www.gnu.org/licenses/>.

REM Base Author: Mumbles @ http://herc.ws

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
ECHO                     http://herc.ws/board/
ECHO.
ECHO.

ECHO Exporting databases to 'sql-files' folder...
PING -n 3 -w 1 127.0.0.1 > nul

map-server.exe --load-plugin db2sql --db2sql
ECHO.

PING -n 10 -w 1 127.0.0.1 > nul
