# Zeta Framework

This is a data-oriented framework for developing 2D games for Windows, Mac, and Linux.

It was originally written in C, though has been switched to a highly procedural subset of C++ 20 to leverage useful features like templates, operator overloading, function overloading, and so on.

### [Click here to see the framework being used to make a clone of Terraria.](https://github.com/harrisonkwhite/zf_terraria_clone)

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

## Noteworthy Design Decisions (W.I.P. Section)

There are a number of significant design decisions made with this framework that I think are worth explaining my rationale for, because many of them are contrary to standard practice. But they have all been made out of a very deliberate cost-benefit analysis done in relation to the specific goals of Zeta Framework.

### Strings

Zeta Framework provides its own string system, in which strings are NOT null-terminated, and are instead arrays of bytes (with an associated length) that are treated as representing UTF-8 text (and therefore simply getting the array length is insufficient for getting string length).

The motivation for this setup was to have a simple UTF-8 string system compatible with the data-oriented procedural style of the rest of the framework. The decision not to use null termination also helps get by safety issues and makes operations like string slicing far simpler and cheaper.

There are definitely instances, however, where standard C-style null-terminated strings need to be used. This is often the case when interacting with C-based APIs for example. There are helpers provided for converting a ZF-style string into a C-style string, and they involve pushing the necessary amount of bytes to an arena and copying the bytes of the string, leaving a null terminator add the end. The performance cost associated with this is the biggest downside to this string approach, but my reasoning is that it is not too significant of a problem assuming that these conversions are unlikely to be done on a frequent or per-frame basis.

### Module Tickets

Out of necessity, many of the modules comprising ZGL are global in nature. For example, both GLFW (used in the platform layer) and bgfx (used for graphics) work as singletons, and as such anything wrapping them should authentically mirror this.

This global state, even if it's technically necessary, damages functional purity and can make state management harder to track and reason about. For example, there is no way to tell from the outside whether some seemingly trivial helper function is actually manipulating some piece of a ZGL module's global state, and this can be a real source of bugs.

To get around this, I came up with a "ticket" system. Essentially, if a function wants to either read from or mutate the state of a particular global module, it needs the corresponding ticket struct. To ensure that you aren't just passing in any randomly created instance of the struct, the correct ticket struct has a "value" variable that is assigned to the address of some module-exclusive global variable, and the key's validity is checked against this. This ticket is returned from whatever function starts up the global module.

Now, if a function is either possibly reading or mutating some part of the GFX module's global state for example, you can easily tell because it will be provided with a ticket.

### Emphasis on "const" and Mutable vs. Read-Only Data

This is something I learnt from reading an article [John Carmack's article on functional programming](http://sevangelatos.com/john-carmack-on/), and which I have adopted very heavily into my C/C++ programming style (at least if I'm working procedurally).

The "const" keyword is used in my code as much as possible, and for any struct that containing some form of pointer, I will often have both a mutable and read-only version of it.

The rationale behind this is that if I'm providing some form of pointer to a function for example, I find it very useful to know whether that thing being pointed to is potentially going to be mutated, versus if it's just going to be read from. Also, when writing or debugging a function, I find that it helps to know which variables can change and which are fixed in place. It overall makes statement management far easier to reason about, and also discourages me from writing hacky overcomplicated code with excessive state transformations.

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
