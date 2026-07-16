# Improvement roadmap

## Highest priority

1. **Video-only Frame Sync / raw-audio bypass**  
   Keep NDI video frame-synchronized when desired while routing the four-channel audio through the raw receive path, avoiding audible Frame Sync resampling or buffer jumps.

2. **A/V timeline watchdog and discontinuity recovery**  
   Measure incoming audio-versus-video timestamp offset, distinguish gradual drift from sudden jumps, log reconnect/rehook events, and atomically flush/restart both sides of the bridge when a discontinuity is confirmed.

3. **Receiver attachment by stable source identity**  
   Store a source UUID or other stable identity instead of relying mainly on the OBS source name, then automatically reattach after source rename, profile change, scene-collection reload, or NDI reconnect.

4. **Scene-independent receiver audio outputs**  
   Keep the split program and microphone buses active across scene changes, or provide one-click insertion into all selected scenes, so changing scenes cannot unexpectedly disable suppression or the proxy outputs.

## Expansion

5. **Dynamic bus count**  
   Support one to six OBS stereo tracks, producing 2–12 NDI audio channels and creating only the receiver buses that are enabled.

6. **Per-bus names and channel mapping**  
   Let users label buses such as Game, Discord, Music, Alerts, Mic, and Backup Mic, with mono/stereo selection and custom NDI channel placement.

7. **Health alerts and diagnostics export**  
   Surface warnings when discard, fallback, missing-channel, packet-age, or reconnect counters cross thresholds, and export a timestamped support bundle containing bridge status and relevant OBS log excerpts.

8. **Controlled adaptive audio correction**  
   Optionally apply tightly bounded PPM correction only after a repeatable gradual clock-rate mismatch is measured. Keep it disabled by default and never use it to hide sudden discontinuities.

## Release quality

9. **Authenticode signing**  
   Sign the setup EXE and DLL through protected GitHub Actions secrets to remove the Unknown Publisher warning and make release provenance clearer.

10. **Automated soak and fault-injection tests**  
    Run long sender/receiver tests and deliberately simulate source rehooks, NDI reconnects, missing audio buses, callback phase changes, scene changes, and receiver restarts.

11. **Version compatibility checks**  
    Detect unsupported OBS, DistroAV base, or NDI Runtime versions and show a clear warning before installation or startup.
