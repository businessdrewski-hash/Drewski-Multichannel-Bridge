# Upload to GitHub

1. Download and extract `NDI-Multichannel-Bridge-v0.3.0-alpha-GitHub-ready.zip`.
2. Open the bridge repository on GitHub.
3. Replace the old repository contents with the extracted contents.
4. Make sure the hidden `.github/workflows/build-windows.yml` file is included.
5. Commit the changes to `main`.
6. Open **Actions → Build Unified Multichannel DistroAV for Windows**.
7. Run the workflow manually, or let the push trigger it.
8. Download the artifact only after the build is green.

The source package does not contain a precompiled OBS DLL. The workflow checks out DistroAV 6.2.1, patches it, builds it, and creates the Windows package.
