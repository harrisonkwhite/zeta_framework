# Zeta Framework

This is a data-oriented framework in C++ for developing small-scale and large-scale 2D PC games.

---

## Features

- A clear game tick structure to work within
- Input handling for keyboard, mouse, and gamepad
- 2D batch-based texture and UTF-8 string rendering
- Surfaces (render targets) that can be displayed using custom shaders
- Sound playing
- RNG helpers
- An optional asset packing tool
- A core library with procedural-style utilities for memory arenas, UTF-8 strings, math, data structures, and more

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

## Third-Party Projects

- [GLFW](https://github.com/glfw/glfw) for cross-platform windowing and input (plus an OpenGL context)
- [glad](https://github.com/Dav1dde/glad) for OpenGL function pointers
- [cJSON](https://github.com/DaveGamble/cJSON) for JSON parsing in the asset packer
- [stb](https://github.com/nothings/stb) for raw image and font file loading
- [miniaudio](https://github.com/mackron/miniaudio) for audio loading and playing

---

## Future Plans

- Networking features
- WASM support
- Better cross-platform support via supporting other graphics APIs like Metal
