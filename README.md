# Zeta Framework

This is a data-oriented framework for developing 2D games for Windows, Mac, and Linux.

It was originally written in C, though has been switched to a procedural subset of C++ 20 to leverage useful features like operator overloading, function overloading, constexpr, templates, and more.

---

## Features

- A well-defined init-tick-render-cleanup structure to work within
- Input handling for keyboard, mouse, and gamepad
- 2D batch-based texture and UTF-8 string rendering
- Surfaces (render targets) that can be displayed using custom shaders
- Sound playing
- An optional asset packing tool
- A core library with procedural-style utilities for memory arenas, UTF-8 strings, 2D math, data structures, and more

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

## Noteworthy Design Decisions

Strings are non-terminated and UTF-8:

Add explanation.

## Third-Party Projects

- [GLFW](https://github.com/glfw/glfw) for cross-platform windowing and input (plus an OpenGL context)
- [BGFX](https://github.com/bkaradzic/bgfx) for a cross-platform graphics backend
- [cJSON](https://github.com/DaveGamble/cJSON) for JSON parsing in the asset packer
- [stb](https://github.com/nothings/stb) for raw image and font file loading
- [miniaudio](https://github.com/mackron/miniaudio) for audio loading and playing

---

## Future Plans

- Networking features
- A* pathfinding helpers
