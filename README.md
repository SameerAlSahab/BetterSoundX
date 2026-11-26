# ALCDSPPlugin

**Native CoreAudio DSP prototype for Hackintosh/macOS** — v0.2.0

This release implements the audio-processing engine requested: 1.5x output gain, a punchy bass shelf, a low-mid cleanup cut, and a soft peak limiter. It is written in portable C++17, allocation-free in its processing path, and processes interleaved stereo Float32 samples in-place.

> **Important status:** the DSP engine is real and has automated tests. The system-wide AudioServerPlugIn HAL virtual device and physical-output relay are not yet implemented as an installable `.driver` bundle. This archive is not a finished system-wide enhancer, and selecting a fake device will not make the speakers louder. Do not install it into `/Library/Audio/Plug-Ins/HAL` yet.

## DSP profile

- Fixed maximum linear gain: **1.50x / +3.52 dB**.
- Low shelf: **+5.5 dB at 105 Hz** for punch and weight.
- Low-mid bell: **-1.5 dB at 250 Hz, Q 0.85** to reduce muddiness.
- Soft peak control: threshold **0.94**, ceiling **0.985** to avoid hard clipping.
- Supported sample rates: 22.05–384 kHz; stereo Float32.

The 1.5x gain applies below the limiter. Hot peaks are reduced by the limiter; without this, boosting already-hot audio would clip. This does not increase the physical power capability of the laptop's speakers.

## Build and test

Requirements: CMake 3.20+ and a C++17 compiler. On macOS, install Xcode Command Line Tools first.

```bash
./Scripts/build.sh
```

Or manually:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Process a raw Float32 stereo file

`dsp_process` accepts native-endian interleaved stereo IEEE-754 Float32 PCM (not WAV):

```bash
./build/dsp_process input.f32 output.f32 48000
```

## Project structure

```text
ALCDSPPlugin/
├── DSP/                 # Real-time DSP engine (biquad EQ, gain, soft limiter)
├── HAL/                 # HAL architecture and integration status
├── Tests/               # DSP regression tests
├── Tools/               # Offline Float32 stereo processor
├── Docs/                # DSP profile and Apple API references
├── Scripts/build.sh
└── CMakeLists.txt
```

## HAL architecture and AppleALC

AppleALC provides codec/layout support through the AppleHDA path. It is not a general-purpose system-wide DSP insertion point. A native system-wide solution needs a CoreAudio `AudioServerPlugIn` virtual device plus a user-space relay that forwards processed audio to the selected physical output. The DSP engine is designed to be called from the HAL driver's real-time mix callback, but the full HAL device/relay is a separate milestone. See [`HAL/README.md`](HAL/README.md).

This project is not a Lilu kext, does not patch AppleALC, and does not need MacKernelSDK for the DSP core. A HAL plug-in is a user-space `.driver` bundle, not a kernel extension. Apple's official AudioServerPlugIn sample is the reference implementation for the missing device layer.

## Safety

Do not set a virtual audio device as your only output until the relay exists and has been tested. Keep a known-good physical output available. Audio DSP can cause clipping or uncomfortable output levels; start with low system volume.

## License

DSP core: MIT. Apple APIs and third-party reference projects retain their own terms. No Dolby code, branding, or licensed assets are included. This project is not affiliated with Apple, Acidanthera, or Dolby.

### HAL callback bridge

`DSP/ALCDSP_C.h` exports a small C ABI for the eventual AudioServerPlugIn adapter. Allocate and prepare the context on a control thread, then call `alcdsp_process_f32_stereo()` from the post-mix real-time callback. `HAL/BridgeContract.h` documents the intended call boundary. The actual HAL device object model and the output relay still need implementation and on-machine validation.
