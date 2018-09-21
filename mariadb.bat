@echo off

WHERE powershell.exe >nul 2>nul
IF %ERRORLEVEL% NEQ 0 (
    ECHO ERROR: PowerShell is not installed on this computer!
    ECHO Please download it here:
    ECHO https://github.com/PowerShell/PowerShell#get-powershell
    ECHO.
    ECHO Once it is installed, please re-launch mariadb.bat
    pause >nul
    exit
)

powershell -NoLogo -ExecutionPolicy Bypass -File "%~dp0\tools\setup_mariadb.ps1"
pause >nul
