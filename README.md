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

## Rationale (W.I.P.)

For context, I began programming with GameMaker as a child, and in my mid-teens moved over to C# with MonoGame. In recent years I've moved over to C/C++ and data-oriented styles of programming.

I think MonoGame is a great framework, the only problem I have is that it uses C#. C# enforces object orientation, which at least for game development I think is quite counter-productive (I'll get to why below).

My past experiences have shaped my approach.

So when I was developing MANIC.

So GameMaker itself is built on an object system. Considering that it is targeted for beginners, I think this very reasonable. Humans think of reality very much in terms of objects, and games are in a sense an abstraction of reality, so when a new person to programming comes along and wants to make a game, their mental model of a game is likely that it's a world comprised of objects.

The basic designing principle of this framework was "GameMaker for more experienced programmers". So I wanted to at least get close to the same expressive power that GameMaker offers for 2D game development, but structure it for programmers who know how to effectively do memory management and set up systems tailored specifically to the game they are making. Another accurate framing of it would be "MonoGame but in C/C++".

RayLib is close to this, but there are some key differences:
- RayLib both is and is marketed as being beginner-friendly and more built for toy projects. With ZF, I wanted to create a framework that could scale for more serious 2D indie projects.
- RayLib solely uses OpenGL, which has been deprecated on MacOS. I wanted ZF to truly be cross-platform at least on Windows, Mac, and Linux.
- RayLib is only in C. Although ZF is also in a procedural style, I thought it'd be useful to leverage some of the useful features of C++ to make programming less of a hassle.
- RayLib is very loose in its global state. This is a pain point I had when working with GameMaker. This is useful for small projects where you just want to get the job done fast, but for large projects becomes a serious pain. In ZF, certain systems are global by necessity, but there are systems in place (see below) to make it more manageable.

There are some common game engine features intentionally absent from ZF. Most significantly, there is no form of ECS nor any scene system.

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
