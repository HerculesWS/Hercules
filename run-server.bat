@echo off
rem This is and auto-restart script for the Server Emulator.
rem It will also keep the map server OPEN after it crashes to that errors may be
rem more easily identified
rem Writen by Jbain
start cmd /k login-server.bat
start cmd /k char-server.bat
start cmd /k map-server.bat
