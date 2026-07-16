param(
    [string]$ObsPath = "C:\Program Files\obs-studio"
)

$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$backupRoot = Join-Path $env:ProgramData 'NDI-Multichannel-Bridge\backup'

$identity = [Security.Principal.WindowsIdentity]::GetCurrent()
$principal = [Security.Principal.WindowsPrincipal]::new($identity)
if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    throw "Run this installer from an Administrator PowerShell window."
}
if (-not (Test-Path $ObsPath)) { throw "OBS folder not found: $ObsPath" }
if (Get-Process obs64 -ErrorAction SilentlyContinue) { throw "Close OBS completely before installing." }

$packageDll = Join-Path $here 'obs-plugins\64bit\distroav.dll'
if (-not (Test-Path $packageDll)) { throw "Package DLL is missing: $packageDll" }

# Preserve the user's pre-bridge DistroAV installation once, so the uninstall
# script can restore it instead of simply deleting DistroAV.
if (-not (Test-Path (Join-Path $backupRoot 'backup-created.txt'))) {
    New-Item -ItemType Directory -Force $backupRoot | Out-Null
    $installedDll = Join-Path $ObsPath 'obs-plugins\64bit\distroav.dll'
    $installedData = Join-Path $ObsPath 'data\obs-plugins\distroav'
    if (Test-Path $installedDll) {
        Copy-Item -Force $installedDll (Join-Path $backupRoot 'distroav.dll')
    }
    if (Test-Path $installedData) {
        Copy-Item -Recurse -Force $installedData (Join-Path $backupRoot 'distroav-data')
    }
    $userDistroAV = Join-Path $env:APPDATA 'obs-studio\plugins\distroav'
    if (Test-Path $userDistroAV) {
        Copy-Item -Recurse -Force $userDistroAV (Join-Path $backupRoot 'per-user-distroav')
    }
    "Created $(Get-Date -Format o) from $ObsPath" | Set-Content -Encoding utf8 (Join-Path $backupRoot 'backup-created.txt')
}

# Remove the obsolete standalone alpha module. v0.3.0 is integrated into the
# custom distroav.dll; loading both would create duplicate source IDs and UI.
$legacyTargets = @(
    (Join-Path $ObsPath 'obs-plugins\64bit\ndi-multichannel-bridge.dll'),
    (Join-Path $ObsPath 'data\obs-plugins\ndi-multichannel-bridge'),
    (Join-Path $env:APPDATA 'obs-studio\plugins\ndi-multichannel-bridge'),
    (Join-Path $env:APPDATA 'obs-studio\plugins\ndi-multichannel-bridge.dll')
)
foreach ($target in $legacyTargets) {
    if (Test-Path $target) { Remove-Item -Recurse -Force $target }
}

foreach ($folder in @('obs-plugins','data')) {
    $source = Join-Path $here $folder
    $destination = Join-Path $ObsPath $folder
    if (-not (Test-Path $source)) { throw "Package folder is missing: $source" }
    New-Item -ItemType Directory -Force $destination | Out-Null
    Copy-Item -Path (Join-Path $source '*') -Destination $destination -Recurse -Force
}

$userPluginDir = Join-Path $env:APPDATA 'obs-studio\plugins\distroav'
if (Test-Path $userPluginDir) {
    Remove-Item -Recurse -Force $userPluginDir
    Write-Host "Disabled the duplicate per-user DistroAV copy after backing it up."
}
Write-Host "Installed NDI Multichannel Bridge v0.3.0-alpha to $ObsPath"
Write-Host "Open OBS > Docks > NDI Multichannel Bridge and select this PC's role."
