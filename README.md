# Zeta Framework

This is a data-oriented framework for developing 2D games for Windows, Mac, and Linux.

It was originally written in C, though has been switched to a highly procedural subset of C++ 20 to leverage useful features like templates, operator overloading, function overloading, and so on.

A minimal template for creating a game with this framework can be found [here](https://github.com/harrisonkwhite/zf_game_template).

---

## Features

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

## Rationale (Incomplete)

For context, I began programming with GameMaker as a child, and in my mid-teens moved over to C# with MonoGame. In recent years I've moved over to C/C++ and data-oriented styles of programming.

I think MonoGame is a great framework, the only problem I have is that it uses C#. C# enforces object orientation, which at least for game development I think is quite counter-productive (I'll get to why below).

My past experiences have shaped my approach.

The basic designing principle of this framework was "GameMaker for more experienced programmers". So I wanted to at least get close to the same expressive power that GameMaker offers for 2D game development, but structure it for programmers who know how to effectively deal with memory management and set up systems . Another accurate framing of it would be "MonoGame but in C/C++".

RayLib is close to this, but is and is marketed as being beginner-friendly and more built for toy projects. It's global state is quite untamed, being one of the big issues I had with GameMaker.

There are some common game engine features intentionally absent from ZF. Most significantly, there is no form of ECS nor any scene system.

There is no, and will never be, any kind of entity system in ZF, and the reason for this is that the way you approach the problem of entity systems (should one even exist).

---

## Third-Party Projects

- [GLFW](https://github.com/glfw/glfw) for cross-platform windowing and input
- [BGFX](https://github.com/bkaradzic/bgfx) for a cross-platform graphics backend
- [cJSON](https://github.com/DaveGamble/cJSON) for JSON parsing in the asset builder
- [stb](https://github.com/nothings/stb) for image and font file loading
- [miniaudio](https://github.com/mackron/miniaudio) for audio loading and playing
- [PCG](https://www.pcg-random.org) for random number generation
- [SplitMix64](https://prng.di.unimi.it/splitmix64.c) for simple U64 scrambling

## Future Plans

- Networking features
- A* pathfinding helpers
