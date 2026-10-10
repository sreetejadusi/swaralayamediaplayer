@echo off
echo Running build and installer creation...
powershell.exe -ExecutionPolicy Bypass -File "%~dp0build_installer.ps1"
pause
