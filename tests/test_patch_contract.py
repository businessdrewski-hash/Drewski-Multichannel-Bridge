#!/usr/bin/env python3
"""Lightweight static checks for the shipped patcher itself."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
patcher = (root / "scripts" / "patch_distroav.py").read_text(encoding="utf-8")
workflow = (root / ".github" / "workflows" / "build-windows.yml").read_text(encoding="utf-8")

required_patcher = [
    "OBS_OUTPUT_AV | OBS_OUTPUT_MULTI_TRACK",
    "raw_audio2 = ndi_output_rawaudio2",
    "multichannel_track_a",
    "multichannel_track_b",
    "obs_output_set_mixers",
    "o->audio_channels = 4",
]
required_workflow = [
    "repository: DistroAV/DistroAV",
    "ref: 6.2.1",
    "Build-Windows.ps1",
    "Package-Windows.ps1",
]

for marker in required_patcher:
    assert marker in patcher, marker
for marker in required_workflow:
    assert marker in workflow, marker
print("Static patch contract checks passed.")
