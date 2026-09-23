# Mac Unreal Toolchain Doctor

This repository includes a macOS preflight/repair script for Unreal Engine projects:

```sh
./tools/mac-unreal-doctor.sh
./tools/mac-unreal-doctor.sh --repair
./tools/mac-unreal-doctor.sh --repair --launch
```

## What it checks

- macOS architecture
- Xcode installation and active developer directory
- Xcode first-launch setup
- known Unreal/Xcode compatibility warnings
- a **real Metal shader compilation**, not only whether the `metal` executable is present
- Clang availability
- the Unreal `.uproject` file
- UnrealEditor installation
- optional direct launch with logging

## Why this exists

Unreal on macOS depends on Apple's Xcode and Metal toolchain. A project can be healthy while Unreal fails during Render Hardware Interface initialization because the optional Metal toolchain is missing or the selected Xcode is not compatible with the installed Unreal version.

The doctor keeps those layers separate:

1. project files
2. Xcode selection
3. Metal shader compiler
4. Unreal Editor
5. project build/runtime

## Repair behavior

`--repair` may:

- switch `xcode-select` to `/Applications/Xcode.app/Contents/Developer`
- accept the Xcode license
- run Xcode first-launch setup
- install Apple's optional Metal Toolchain with `xcodebuild -downloadComponent metalToolchain`
- re-run a Metal shader compile to verify the repair

It does not delete project files.

## UE 5.8 note

Epic's documented macOS guidance for Unreal Engine 5.8 should be treated as the compatibility baseline. If you are using a newer Xcode than Epic's documented recommendation, the doctor warns rather than assuming compatibility.

## Emulation and game-runtime architecture notes

For future emulator/runtime work, keep the major layers independently testable:

- CPU execution: interpreter, dynamic translation/JIT, exception behavior, timing
- memory: address translation, MMIO, alignment, endianness, cache-visible behavior
- graphics: guest GPU/API model -> translation layer -> host API (Metal/Vulkan/Direct3D/OpenGL as appropriate)
- input: deterministic mapping, hotplug, controller state, latency
- audio: clocking, resampling, buffer sizing, A/V synchronization
- timing: frame pacing, timers, interrupts, vsync, determinism
- storage: save states, persistent saves, virtual media/filesystems
- platform services: threads, files, networking, windows, permissions
- verification: conformance tests, replay tests, regression ROM/test programs, render hashes, audio timing tests

Avoid coupling platform setup with guest-system emulation logic. A broken host compiler/toolchain should fail preflight before the emulator or game runtime starts.
