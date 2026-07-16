param(
    [string]$ObsPath = "C:\Program Files\obs-studio"
)

$ErrorActionPreference = 'Stop'
$backupRoot = Join-Path $env:ProgramData 'NDI-Multichannel-Bridge\backup'
$identity = [Security.Principal.WindowsIdentity]::GetCurrent()
$principal = [Security.Principal.WindowsPrincipal]::new($identity)
if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    throw "Run this uninstaller from an Administrator PowerShell window."
}
if (Get-Process obs64 -ErrorAction SilentlyContinue) { throw "Close OBS completely before uninstalling." }

$installedDll = Join-Path $ObsPath 'obs-plugins\64bit\distroav.dll'
$installedData = Join-Path $ObsPath 'data\obs-plugins\distroav'
if (Test-Path $installedDll) { Remove-Item -Force $installedDll }
if (Test-Path $installedData) { Remove-Item -Recurse -Force $installedData }

$backupDll = Join-Path $backupRoot 'distroav.dll'
$backupData = Join-Path $backupRoot 'distroav-data'
if (Test-Path $backupDll) {
    New-Item -ItemType Directory -Force (Split-Path -Parent $installedDll) | Out-Null
    Copy-Item -Force $backupDll $installedDll
    if (Test-Path $backupData) {
        New-Item -ItemType Directory -Force (Split-Path -Parent $installedData) | Out-Null
        Copy-Item -Recurse -Force $backupData $installedData
    }
    Write-Host "Removed the bridge and restored the DistroAV installation backed up before the first bridge install."
} else {
    Write-Host "Removed the custom DistroAV build. No backup was available; reinstall official DistroAV if needed."
}

$backupUserDistroAV = Join-Path $backupRoot 'per-user-distroav'
$userDistroAV = Join-Path $env:APPDATA 'obs-studio\plugins\distroav'
if (Test-Path $backupUserDistroAV) {
    if (Test-Path $userDistroAV) { Remove-Item -Recurse -Force $userDistroAV }
    New-Item -ItemType Directory -Force (Split-Path -Parent $userDistroAV) | Out-Null
    Copy-Item -Recurse -Force $backupUserDistroAV $userDistroAV
    Write-Host "Restored the per-user DistroAV copy that was disabled during installation."
}
