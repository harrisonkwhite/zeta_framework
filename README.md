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

Personally I think that rather than emphasising objects, games are much better off organised in terms of explicit procedures, and the subset of read-only vs. mutable state that these procedures are exposed to.

The basic designing principle of this framework was "GameMaker for more experienced programmers". So I wanted to at least get close to the same expressive power that GameMaker offers for 2D game development, but structure it for programmers who know how to effectively do memory management and set up systems tailored specifically to the game they are making. Another accurate framing of it would be "MonoGame but in C/C++".

RayLib is close to this, but there are some key differences:
- RayLib both is and is marketed as being beginner-friendly and more built for toy projects. With ZF, I wanted to create a framework that could scale for more serious 2D indie projects.
- RayLib solely uses OpenGL, which has been deprecated on MacOS. I wanted ZF to truly be cross-platform at the very least on Windows, Mac, and Linux.
- RayLib is only in C. Although ZF is largely written C-style, I thought it'd be useful to leverage some of the useful features of C++ to make programming less of a hassle (see below).
- RayLib is very loose in its global state. This is a pain point I had when working with GameMaker. This is useful for small projects where you just want to get the job done fast, but for large projects becomes a serious pain. In ZF, certain systems are global by necessity, but there are systems in place (see below) to make it more manageable.

There are some very important fundamental design decisions in ZF, many of which go against standard practice, that are worth explaining:

Use of an STL "Replacement":
- The ZF core library is not at all "better" than the C++ STL (that would be a stupidly arrogant thing to say). The reason the STL is very minimally used is because it's written from a completely different design philosophy, one which emphasises RAII, object orientation, etc.

Memory Arenas:

So with games of the scope that I've worked with, I find it extremely simple to organise memory and resource lifetimes. For example, in an engine you can generally get away with a model of game-long lifetime memory, and frame-long lifetime memory. You'll often also need some kind of scratch space arena for temporary allocations.

Let's say, for example, in a particular function you need some kind of large array of a length dependent on function inputs. Let's also suppose that the maximum possible array length you would need in this case is too large to be stack-allocated. If you were to go the more conventional modern C++ route, you might have something like an std::vector which gets cleaned up implicitly when out of scope. There are 2 main issues I have with this. Firstly, the dynamic allocation is NOT free if the allocator has to work account for fragmentation. Secondly, by depending on a destructor you're adding obfuscation to what your code is actually doing.

I really think that with arenas in game development (at least at the scope I'm dealing), the pros of using this approach MASSIVELY outweigh the cons.

And for a 

- Ownership
- Downsides (object-level encapsulation usually impossible)

Resource Groups:

Defer:
- Controversial, wow!

Module-level encapsulation:
- 

Unique string system (UTF-8 and non-terminated):
- 

Custom Print Function:
- 

Tickets:
- Wow!

Asset Builder:
-

Template and concept usage:
- 

Misc. Style Things:
- No methods at all aside from operator overloads (including cast operators). This is really just for API consistency.

Fatal Errors:
- Was quite hard for me to do
- No exceptions

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

---

## Future Plans

- Networking features
- A* pathfinding helpers
