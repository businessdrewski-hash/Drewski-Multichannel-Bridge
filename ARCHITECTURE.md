# Architecture

## Sender

The patch keeps DistroAV's existing asynchronous raw-video callback and NDI sender. It changes the registered output to OBS raw multi-track audio mode. Main Output requests only OBS mixers 5 and 6.

`raw_audio2` receives the two stereo blocks. A tiny pairing state matches blocks by timestamp and frame count. Once both are present, it creates one planar four-channel block and passes it into DistroAV's existing `NDIlib_audio_frame_v3_t` send path:

```text
OBS Track 5 L -> NDI channel 1
OBS Track 5 R -> NDI channel 2
OBS Track 6 L -> NDI channel 3
OBS Track 6 R -> NDI channel 4
```

Video and four-channel audio are transmitted by the same DistroAV NDI sender instance.

## Why this should be lighter than the standalone alpha

The standalone alpha requested BGRA output and copied every full video frame into its own double buffers before NDI submission. This fork does not add a second video output implementation. It uses DistroAV's existing video path and only adds small audio-buffer copies.

## Timing behavior

The two OBS audio tracks are packed only when their block timestamps and sample counts match. This keeps their relationship deterministic. The NDI audio and video frames retain DistroAV's synthesized NDI timecode behavior.
