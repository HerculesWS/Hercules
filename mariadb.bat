@echo off
powershell -NoLogo -ExecutionPolicy Bypass -File "%~dp0\tools\setup_mariadb.ps1"
pause >nul
