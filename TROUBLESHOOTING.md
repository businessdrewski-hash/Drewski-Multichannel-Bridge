# Troubleshooting

## The dock is missing

Search **Help → Log Files → View Current Log** for:

- `[multichannel-bridge] Registered receiver proxy sources`
- `[multichannel-bridge] Dock initialized`

The dock is under **Docks → NDI Multichannel Bridge**. DistroAV still reports version 6.2.1; the bridge dock/log markers identify the custom build.

If the log says `Skipping module 'ndi-multichannel-bridge', is disabled`, that refers to the obsolete standalone alpha. The integrated v0.3.0 build lives inside `distroav.dll`; the installer removes the obsolete DLL.

## Gaming PC has video but no audio

The sender dock must show **ACTIVE**, and **paired** must rise.

1. Confirm this PC is **Gaming PC / Sender**.
2. Confirm DistroAV Main Output is enabled.
3. Route desktop/game to Track A and mic to Track B in **Advanced Audio Properties**.
4. Use two different tracks.
5. Make sure neither source is **Monitor Only (mute output)**.
6. Use 48 kHz OBS audio.

A timestamp delta near 21.333 ms at 48 kHz is normal for two selected OBS raw mixes and should still produce a continuously rising paired count. A continuously rising discarded count means the tracks are more than one full audio block apart or their block sizes differ. A rising fallback count means one selected OBS mix stopped producing callbacks.

## Stream PC gets video but no split audio

1. Confirm this PC is **Stream PC / Receiver**.
2. Add one normal DistroAV NDI Source and select the combined gaming-PC feed.
3. Keep NDI audio enabled in that source.
4. Select the OBS source name in the bridge dock.
5. Click **Create / repair split audio sources**.
6. Confirm `ATTACHED`, split outputs `ready`, and `4 channels detected`.

If only two channels are detected, the gaming PC is sending ordinary stereo, the sender role is not active, or the wrong NDI source was selected.

## Duplicate or downmixed audio

Keep **Suppress the original 4-channel audio after both split sources are ready** checked. The custom DistroAV receiver then skips only the original audio submission after both proxy sources exist; video continues normally.

Remove or mute the old separate `NDI Desktop Audio` and `NDI MIC only` sources during testing.

## OBS reports a missing old source type

Delete the obsolete `NDI Multichannel Video` source. v0.3.0 uses:

- one normal DistroAV NDI Source for video/input selection
- `MCB Desktop / Game`
- `MCB Microphone`

## Wrong DistroAV copy loads

Only one `distroav.dll` should be visible to OBS. Check both:

- `C:\Program Files\obs-studio\obs-plugins\64bit\distroav.dll`
- `%APPDATA%\obs-studio\plugins\distroav\bin\64bit\distroav.dll`

The installer backs up and disables the common per-user duplicate so the integrated custom build is the only DistroAV copy OBS can load.

## Uninstall

Close OBS and double-click `Uninstall-MultichannelBridge.cmd`. It restores the system-wide and per-user DistroAV installations backed up before the first bridge install when those backups are available.
