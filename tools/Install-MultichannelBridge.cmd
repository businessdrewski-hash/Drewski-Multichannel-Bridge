@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "Start-Process PowerShell.exe -Verb RunAs -Wait -ArgumentList '-NoProfile -ExecutionPolicy Bypass -File ""%~dp0Install-MultichannelBridge.ps1""'"
if errorlevel 1 (
  echo Installation did not complete successfully.
) else (
  echo Installation command finished. Review the Administrator PowerShell window for details.
)
pause
