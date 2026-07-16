# v0.3.0-alpha release notes

## Major changes

- One integrated custom DistroAV build is installed on both gaming and stream PCs.
- Added an explicit, confirmed PC-role selector in an OBS dock.
- Reworked sender alignment around bounded FIFO queues with one-block phase tolerance for OBS raw multitrack callbacks.
- Fixes the observed continuous 21.333 ms mismatch that caused every audio block to be discarded in the sender-only prototype.
- Added bounded queue growth and a reported silence fallback if one selected mix stops producing callbacks.
- Moved receiver splitting into DistroAV's raw planar NDI receive path, before OBS speaker-layout remix/downmix.
- Added two independent receiver stereo sources, one-click creation/repair, and safe original-audio suppression only when both outputs are ready.
- Added sender and receiver meters, queue/channel/packet diagnostics, reset counters, setup warnings, and copyable diagnostics.
- Receiver role disables DistroAV Main Output on that PC to reduce accidental loops.
- Installer now backs up system-wide and per-user DistroAV installations, disables duplicate copies, removes the obsolete standalone bridge DLL, and includes elevated double-click wrappers.

## Validation status

- The Python patcher passes Python syntax validation.
- The patcher includes structural verification for sender, receiver, dock, and build-list modifications.
- A Windows OBS/DistroAV DLL cannot be linked or loaded in this environment. GitHub Actions is still the authoritative compile test.
- Treat this as a controlled-test alpha. Verify with a local recording and audible/visible sync markers before a live production.
