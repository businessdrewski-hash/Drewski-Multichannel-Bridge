@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "Start-Process PowerShell.exe -Verb RunAs -Wait -ArgumentList '-NoProfile -ExecutionPolicy Bypass -File ""%~dp0Uninstall-MultichannelBridge.ps1""'"
if errorlevel 1 (
  echo Uninstall did not complete successfully.
) else (
  echo Uninstall command finished. Review the Administrator PowerShell window for details.
)
pause
