# Zeta Framework

This is a data-oriented framework for developing 2D games for Windows, Mac, and Linux.

It was originally written in C, though has been switched to a highly procedural subset of C++ 20 to leverage useful features like templates, operator overloading, function overloading, and so on.

A simple Terraria clone made with this framework can be found [here](https://github.com/harrisonkwhite/zf_terraria_clone).

A minimal template for creating a game with this framework can be found [here](https://github.com/harrisonkwhite/zf_game_template).

---

## Key Features

- A well-defined init-tick-render-cleanup structure to work within
- Input handling for keyboard, mouse, and gamepad
- 2D batch-based texture and UTF-8 string rendering
- The ability to render using custom shaders and to assign textures as render targets
- Audio playing and streaming
- An optional asset building tool
- A core library with utilities for memory arenas, UTF-8 strings, 2D math, data structures, RNG, and more

---

## Building

Clone the repository by running `git clone --recursive https://github.com/harrisonkwhite/zeta_framework.git`.

> **Note:** If the repository was cloned non-recursively before, just run `git submodule update --init --recursive` to clone the necessary submodules.

Then go into the repository root and build with CMake:

```
mkdir build
cd build
cmake ..
```

---

## High-Level Structure

**zf_core_lib (ZCL):** A generic utilities library with procedural-style helpers for things like memory arenas, UTF-8 strings, hash maps, and so on. It also has some GFX and audio helpers so both ZGL and the asset builder can use them. Intentionally has *very* minimal explicit global state (the code for handling fatal errors has some).

**zf_game_lib (ZGL):** A library specifically for supporting the game executable, containing the platform layer, rendering system, audio system, and so on. Is more explicitly organised into modules, most of which have encapsulated global state out of necessity.

**zf_bin_to_array:** A simple tool that takes in a filename and spits out a C++ source file containing the binary blob as a constant. The only reason this exists is so that compiled shader files can be accessed in ZGL code as binary blobs. But you might find it useful for something else.

**zf_asset_builder:** Builds ZF-specific asset files from raw ones like ".png", ".ttf", ".wav", etc. for use in ZGL. This is totally optional but recommended.

**zf_tests:** Has simple unit tests for various things in both ZCL and ZGL.

---

## Third-Party Projects

- [GLFW](https://github.com/glfw/glfw) for cross-platform windowing and input
- [BGFX](https://github.com/bkaradzic/bgfx) for a cross-platform graphics backend
- [cJSON](https://github.com/DaveGamble/cJSON) for JSON parsing in the asset builder
- [stb](https://github.com/nothings/stb) for image and font file loading
- [miniaudio](https://github.com/mackron/miniaudio) for audio loading and playing
- [PCG](https://www.pcg-random.org) for random number generation
- [SplitMix64](https://prng.di.unimi.it/splitmix64.c) for simple U64 scrambling

---

## Future Plans

- Networking features
- A* pathfinding helpers
