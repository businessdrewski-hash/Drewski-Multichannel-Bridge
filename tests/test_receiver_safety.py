#!/usr/bin/env python3
"""Protect receiver availability, reconnect, and split-audio activation paths."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
bridge = (ROOT / "hotfix/bridge/multichannel-bridge.cpp").read_text(encoding="utf-8")
patcher = (ROOT / "hotfix/scripts/patch_distroav.py").read_text(encoding="utf-8")

required_bridge = (
    "Video is the master clock. It passes through unchanged",
    "mcb::DownstreamSyncSnapshot sync_snapshot",
    "obs_source_set_audio_active(source, true);",
    "obs_source_set_muted(proxy, false);",
    "obs_source_set_audio_mixers(proxy, 0x3fU);",
    "bool force_reconnect()",
    'proc_handler_call(\n\t\t\tobs_source_get_proc_handler(source), "mcb_force_reconnect"',
    'obs_data_set_int(settings, "ndi_behavior", 0)',
    "matching_receiver_count",
    "add_existing_source_to_current_scene",
    "LinkedAudioClockFilter",
    "core.observe_audio_input(audio->timestamp, wall_ns)",
    "core.report_audio_output",
    "reset_linked_audio_timeline(filter, audio, true)",
)
for marker in required_bridge:
    if marker not in bridge:
        raise SystemExit(f"Receiver safety marker is missing: {marker}")

required_patcher = (
    "mcb_force_reconnect_proc",
    "void mcb_force_reconnect(out bool accepted)",
    "NDIlib_send_timecode_synthesize",
    "raw sender OBS timestamps leaked into NDI timecodes",
)
for marker in required_patcher:
    if marker not in patcher:
        raise SystemExit(f"Receiver patch contract is missing: {marker}")

print("Receiver availability and split-audio audit passed")
