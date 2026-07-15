# DistroAV Multichannel Main Output Hotfix v0.1.0-alpha

This is a small **private-repository builder** for an experimental DistroAV 6.2.1 fork. It does not contain the full DistroAV source tree. GitHub Actions checks out the official DistroAV 6.2.1 tag, applies the included source patch, builds it, and produces Windows artifacts.

## What the fork does

DistroAV Main Output keeps its existing video path and its existing single NDI sender, but its audio becomes four-channel:

- **NDI channels 1–2:** OBS Track 5, stereo
- **NDI channels 3–4:** OBS Track 6, stereo

This avoids the standalone bridge sender's additional full-frame BGRA copy/conversion path. Other DistroAV output instances stay in ordinary audio mode.

## Build on GitHub

1. Create a **private** GitHub repository.
2. Upload the contents of this folder, including `.github`.
3. Open **Actions → Build DistroAV Multichannel Hotfix → Run workflow**.
4. Download the normal or portable Windows artifact after the job succeeds.

The first GitHub run is also the first complete Windows compile test for this alpha. A compile error should be treated as a patch bug rather than proof that the design cannot work.

## Install

Close OBS. Back up the current DistroAV installation, then install the produced fork over DistroAV or use the portable ZIP to copy its plugin files into the OBS installation. Do not keep two different `distroav.dll` builds active simultaneously.

The fork still requires the normal NDI Runtime expected by DistroAV.

## Sender setup

On the gaming PC:

1. In **Advanced Audio Properties**, place desktop/game audio on **Track 5**.
2. Place the microphone on **Track 6**.
3. They may also remain on Track 1 for local monitoring or other output needs.
4. Enable **DistroAV Main Output** normally.
5. Do not run the old standalone multichannel sender at the same time.

The OBS log should contain:

```text
DistroAV Multichannel Main Output hotfix enabled: Track 5 -> NDI 1-2, Track 6 -> NDI 3-4
```

## Receiver

A receiver must split the incoming four-channel audio without mixing it down:

- channels 1–2 → desktop source
- channels 3–4 → microphone source

The earlier NDI Multichannel Bridge receiver bundle is intended for that role. Stock DistroAV may expose or mix the four channels according to the OBS speaker layout; it does not currently provide two independently controllable stereo sources from one shared receiver.

## Current limitations

- Track selection is fixed to **Tracks 5 and 6** in this first alpha.
- Only the Main Output is changed to four-channel mode.
- This has not yet been validated in a real two-PC stream.
- The paired audio blocks are matched by OBS timestamp and sample count. An unmatched older block is discarded rather than allowing one pair to drift away from the other.
- This does not guarantee that two separate physical audio devices are hardware-clock-locked before OBS mixes them. It does put the resulting two OBS mixes into one NDI sender/timeline.

## Licensing and attribution

The generated fork is based on DistroAV and remains licensed under **GPL-2.0-or-later**. Preserve DistroAV's existing copyright and license notices when distributing source or binaries.

This experimental hotfix is independent and is not affiliated with or endorsed by the DistroAV project, the OBS Project, or Vizrt NDI AB.
