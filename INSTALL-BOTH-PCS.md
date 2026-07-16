# Install and use on both PCs

## 1. Build once on GitHub

1. Replace the contents of the bridge repository with this source package. Preserve the hidden `.github` folder.
2. Open the repository's **Actions** tab.
3. Run **Build Unified Multichannel DistroAV for Windows**.
4. Download `NDI-Multichannel-Bridge-v0.3.0-alpha-Windows-x64` after the workflow succeeds.
5. Extract the artifact, then extract the ZIP inside it.

## 2. Install the same package on both PCs

1. Close OBS completely.
2. Double-click **Install-MultichannelBridge.cmd** and approve the Administrator prompt.
3. The PowerShell script backs up the current DistroAV installation and installs the custom build.
4. Repeat on the other PC using the exact same build.
5. Start OBS on both PCs.

The installer backs up the existing system-wide and per-user DistroAV copies on the first install, disables duplicate per-user DistroAV files, and removes the obsolete standalone `ndi-multichannel-bridge.dll`.

## 3. Gaming PC setup

1. Open **Docks → NDI Multichannel Bridge**.
2. Select **Gaming PC — send video + two stereo OBS tracks**.
3. Tick the role-confirmation box.
4. Leave Track A/Track B at **5/6**, or choose two different OBS tracks.
5. Click **Apply role and settings**.
6. Open **Edit → Advanced Audio Properties**:
   - Put desktop/game audio on Track A.
   - Put the microphone on Track B.
   - Make sure neither source is set to **Monitor Only (mute output)**.
7. Open **Tools → DistroAV Output Settings**.
8. Enable **Main Output** and name it, for example, `DrewskiGame`.
9. Confirm the dock shows **ACTIVE**, both meters move, and **paired** continuously rises.

Default mapping:

- OBS Track 5 L/R → NDI channels 1/2
- OBS Track 6 L/R → NDI channels 3/4

Do not use the old separate DistroAV audio-only filters while testing this combined feed.

## 4. Stream PC setup

1. Open **Docks → NDI Multichannel Bridge**.
2. Select **Stream PC — receive and split the four NDI channels**.
3. Tick the role-confirmation box and click **Apply role and settings**.
4. Add one regular **DistroAV NDI Source** to the scene.
5. In that source, select the gaming-PC Main Output, such as `DREWSKI (DrewskiGame)`, and keep its NDI audio enabled.
6. In the bridge dock, click **Refresh NDI sources** and select that OBS source by its OBS source name.
7. Keep **Suppress the original 4-channel audio after both split sources are ready** checked.
8. Click **Create / repair split audio sources in current scene**.
9. Two mixer faders should appear:
   - `MCB Desktop / Game`
   - `MCB Microphone`
10. Route those two sources to OBS recording/stream tracks normally.

The normal DistroAV source remains your video source. The bridge intercepts its raw four-channel audio before OBS's speaker-layout remix and sends the two stereo pairs to the generated mixer sources.

## 5. Healthy status

### Gaming PC

- `ACTIVE`
- both sender meters respond
- paired count rises continuously
- discarded count normally stays at zero; a brief rise is acceptable during a track interruption/recovery
- fallback stays at zero during normal operation
- queue depths stay small
- audio age stays low

### Stream PC

- `ATTACHED`
- split outputs show `ready`
- `4 channels detected`
- both receiver meters respond independently
- packet age stays low
- missing program/mic counters stay at zero

## 6. Changes and recovery

- After changing sender tracks, click **Apply role and settings** to rebuild Main Output.
- If a receiver source is renamed or recreated, refresh the list and reapply.
- If the generated mixer sources are missing from a scene, click **Create / repair**.
- Use **Copy diagnostics** before sharing an OBS log.
- Delete any obsolete scene source named `NDI Multichannel Video`; v0.3.0 uses a normal DistroAV video source plus two generated audio sources.
