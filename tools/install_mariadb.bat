@echo off

:: this file installs the mariadb service

if not "%1"=="am_admin" (powershell start -verb runas '%0' am_admin & exit /b)
mysqld.exe --install "MySQL"
net start MySQL
