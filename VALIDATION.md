# Validation notes for v0.3.0-alpha

Performed in the source-generation environment:

- `patch_distroav.py` passes Python bytecode compilation.
- The patcher was run end-to-end against a clean mock DistroAV 6.2.1 tree containing the exact source anchors used by the official files.
- The official DistroAV 6.2.1 `CMakeLists.txt`, `plugin-main.cpp`, `main-output.cpp`, `ndi-output.cpp`, and `ndi-source.cpp` anchors were inspected against the patch rules.
- OBS's official raw-output callback was reviewed: it uses one shared `total_audio_frames` counter across selected mixers, which explains the stable one-block timestamp phase and is why v0.3.0 accepts that delta.
- Patch verification confirmed the CMake source list, module initialization/shutdown, sender Main Output configuration, raw multi-track callback, receiver raw-audio hook, and dock source registration.
- `multichannel-bridge.cpp` passed Clang 17 C++17 syntax checking with warnings enabled against OBS/Qt interface stubs.
- The injected sender callback and receiver hook passed C++17 syntax checking in the patched mock tree.
- A timestamp-queue simulation of the observed 21.333 ms/one-block callback phase produced continuous pairing with no repeated discards; gaps larger than one block still discard only the stale FIFO front.
- The queue simulation also covered a temporary missing-track period, silence fallback, and clean re-pairing after that track resumed.

Not performed here:

- Linking against the real Windows OBS 32.1.2/DistroAV 6.2.1 build environment.
- Loading the compiled DLL in OBS.
- Live NDI sender/receiver testing on the two physical PCs.
- Long-duration drift, reconnect, recording, and live-production validation.

The included GitHub Actions workflow is the authoritative Windows compile test. Treat this as controlled-test alpha software until the workflow passes and a local A/V sync recording is verified.
