# NDI Multichannel Bridge v0.3.0-alpha

A unified custom build of **DistroAV 6.2.1** for a two-PC OBS setup. Install the **same compiled package on both PCs**, then choose the role for each machine in **Docks → NDI Multichannel Bridge**.

## What it does

### Gaming PC / Sender

- Keeps DistroAV Main Output's existing video sender.
- Captures two configurable OBS stereo tracks.
- Packs Track A into NDI channels 1-2 and Track B into channels 3-4.
- Sends one NDI source containing video and all four audio channels.
- Uses bounded timestamp FIFO queues and explicitly accepts OBS's normal one-block phase between two raw mixer callbacks. The observed 21.333 ms delta is treated as a valid pair instead of being discarded.
- Uses silence only as a reported fallback when one selected OBS track genuinely stops producing callbacks.

### Stream PC / Receiver

- Uses one normal DistroAV NDI Source for the combined gaming-PC feed.
- Splits raw planar NDI audio **inside DistroAV before OBS can downmix it**.
- Creates two ordinary OBS audio-only sources with independent mixer faders:
  - NDI channels 1-2: Desktop / game
  - NDI channels 3-4: Microphone
- Keeps the original DistroAV source as the video source.
- Can suppress the original four-channel audio packets only after both split outputs are ready, preventing duplicate/downmixed audio without muting or hiding the video source.

## QoL dock

The dock includes:

- Required **Gaming PC / Stream PC** role selection and confirmation
- Configurable sender OBS tracks
- Receiver DistroAV-source discovery, refresh, and one-click source properties
- One-click creation/repair of both receiver mixer sources
- Sender program/mic meters, paired/discarded/fallback counters, timestamp delta, queue depth, and packet age
- Receiver program/mic meters, detected NDI channel count, output readiness, packet age, missing-channel counters, and suppression counter
- 48 kHz warning
- Reset-counters and copy-diagnostics buttons
- Automatic Main Output shutdown in Receiver mode to prevent accidental loopback sending

## Build

Upload this package to the root of the bridge repository, preserving the hidden `.github` folder. Run **Actions → Build Unified Multichannel DistroAV for Windows**, then download the resulting Windows x64 artifact.

This is a GitHub-ready **source package**, not a precompiled DLL. The included workflow checks out the official DistroAV 6.2.1 tag, applies the unified patch, builds it, and produces a drag-and-drop OBS package.

## License and attribution

The patched DistroAV build remains subject to DistroAV's GPL-2.0-or-later license. This experimental bridge is not affiliated with or endorsed by the DistroAV project or Vizrt NDI AB.

## Documentation

- `INSTALL-BOTH-PCS.md` — complete setup guide
- `TROUBLESHOOTING.md` — expected status and common failures
- `RELEASE-NOTES.md` — changes and test status
- `UPLOAD-TO-GITHUB.md` — repository update steps
