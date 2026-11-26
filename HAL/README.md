# CoreAudio HAL integration — current status

The DSP engine in `../DSP` is implemented and independently buildable/testable. This directory documents the system-routing layer; it is **not yet a complete installable `.driver` bundle**.

A HAL virtual output device alone cannot make sound audible through the laptop speakers: it receives the system mix, but something must deliver the processed frames to the selected physical output. The complete product therefore needs two cooperating pieces:

1. An `AudioServerPlugIn` virtual duplex device (HAL driver) that accepts system output and exposes processed frames as a capture stream.
2. A small user-space relay that reads that capture stream and writes to the chosen physical output device. The DSP must run on one side of this path exactly once.

The intended real-time callback sequence is:

```text
System apps -> ALCDSP virtual output -> mixed float32 stereo frames
            -> alcdsp::Engine::Process() -> virtual input/capture stream
            -> ALCDSP relay -> selected physical output -> speakers/headphones
```

The relay must use CoreAudio AudioUnit HAL I/O, maintain sample-rate agreement, handle device removal and avoid allocations/locks in render callbacks. It should not be hidden behind a claim that a driver-only kext can transparently post-process AppleALC.

Apple's official AudioServerPlugIn sample is the appropriate starting point for the HAL object model. The plugin runs in user space and is installed under `/Library/Audio/Plug-Ins/HAL`; it is not a Lilu kext and does not use MacKernelSDK. See `Docs/REFERENCES.md`.

## Why no fake `.driver` is shipped

The AudioServerPlugIn ABI has a large property/object contract and strict real-time requirements. Shipping an incomplete driver bundle can break `coreaudiod` or create a silent device. The current release deliberately ships the real DSP core and tests, but does not pretend a nonfunctional HAL stub is a finished driver. The next milestone is to port the duplex device and relay, then validate on the target macOS version and Intel Hackintosh hardware.
