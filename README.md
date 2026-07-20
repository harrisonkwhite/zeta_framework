# Zeta Framework

This is a data-oriented framework for developing 2D games for Windows, Mac, and Linux.

It was originally written in C11, though has been switched to a highly procedural subset of C++20 to leverage useful features like templates, operator overloading, function overloading, and so on.

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

## Noteworthy Design Decisions

There are a number of significant design decisions made with this framework that I think are worth explaining my rationale for, because many of them are contrary to standard practice. But they have all been made out of a very deliberate cost-benefit analysis done in relation to the specific goals of Zeta Framework.

### Use of a Procedural Style

For context, this framework *did* begin life written in C11. But even with the transition into C++20, I still strongly think that a procedural style is most ideal.

My philosophy is that both game engine and gameplay code are far easier to reason about and debug when they are laid out as an explicit sequence of instructions, rather than having operations implicitly occur in things like destructors as you might with OOP. The lack of OOP-style encapsulation is for sure a downside, but within the framework (and especially throughout ZGL) there is heavy use of opaque handles as a kind of substitute (i.e. structs that are only defined in the source file). Assertions are also heavily used as a way of validating state at particular points in the control flow during debug mode, especially at the beginning of functions. There is also *heavy* use of the "const" keyword (elaborated on later) as another way of helping manage state despite the absence of OOP encapsulation.

It is with this that the decision to use very little of the C++ standard library should be obvious, since it is written from an entirely different design philosophy (e.g. heavy use of OOP).

### Approach to Memory Management

Rather than appealing to the conventional approach of RAII, this framework is very heavily based around the use of memory arenas. Memory arenas can effectively be seen as a way of grouping lifetimes together. You effectively have a single big block of memory that is allocated all at once, or perhaps multiple big blocks of memory that are allocated only on occassion and then strung together as a linked list. An arena allocation, or a "push", is generally nothing more than incrementing an offset - *far* cheaper than any call to malloc() or use of "new" for example. Everything allocated within the same arena has the same lifetime, meaning that when the arena's memory is freed everything that was allocated within it is freed too.

I *very strongly* believe this is the superior approach for what Zeta Framework is trying to do. With independent game development it is generally very easy to group resource lifetimes; for example, you might have certain resources existing for the full duration of the game, some for the duration of a particular level, some only for the duration of a frame, and so on. And each of these lifetimes can have its own arena with a very clearly defined life cycle in the scheme of the overall control flow. This keeps memory management very simple, and also keeps allocations very efficient which is important for the high-performance needs of games.

This philosophy of grouping lifetimes is used as often as possible in the framework, beyond just the provided memory arena structure. For example, in the GFX module of ZGL, GFX resources (e.g. textures or shader programs) are required to be allocated in the context of a given GFX resource group, which internally is set up as a linked list of GFX resources that are all to be freed at once, and are also associated with the same memory arena structure.

### Custom String System

Zeta Framework provides its own string system, in which strings are NOT null-terminated, and are instead arrays of bytes (with an associated length) that are treated as representing UTF-8 text (and therefore simply getting the array length is insufficient for getting string length).

The motivation for this setup was to have a simple UTF-8 string system compatible with the data-oriented procedural style of the rest of the framework. The decision to not use null termination also helps get past some safety issues, and also makes operations like string slicing far simpler and cheaper.

There are definitely instances, however, where standard C-style null-terminated strings need to be used. This is often the case when interacting with C-based APIs for example. There are helpers provided for converting a ZF-style string into a C-style string, and they involve pushing the necessary amount of bytes to an arena and copying the bytes of the string, leaving a null terminator add the end. The performance cost associated with this is the biggest downside to this string approach, but my reasoning is that it is not too significant of a problem as my assumption is that these conversions are unlikely to be done on a frequent or per-frame basis.

### Module Tickets

Out of necessity, many of the modules comprising ZGL are global in nature. For example, both GLFW (used in the platform layer) and bgfx (used for graphics) work as singletons, and as such anything wrapping them should authentically mirror this.

This global state, even if it's technically necessary, damages functional purity and can make state management harder to track and reason about. For example, there is no way to tell from the outside whether some seemingly trivial helper function is actually manipulating some piece of a ZGL module's global state, and this can be a real source of bugs.

To get around this, I came up with a "ticket" system. Essentially, if a function wants to either read from or mutate the state of a particular global module, it needs the corresponding ticket struct. To ensure that you aren't just passing in any randomly created instance of the struct, the correct ticket struct has a "value" variable that is assigned to the address of some module-exclusive global variable, and the key's validity is checked against this. This ticket is returned from whatever function starts up the global module.

Now, if a function is either possibly reading or mutating some part of the GFX module's global state for example, you can easily tell because it will be provided with a ticket.

### Emphasis on "const" and Mutable vs. Read-Only Data

This is something I learnt from reading an article [John Carmack's article on functional programming](http://sevangelatos.com/john-carmack-on/), and which I have adopted very heavily into my C/C++ programming style (at least if I'm working procedurally).

The "const" keyword is used in my code as much as possible, and for any struct that containing some form of pointer, I will often have both a mutable and read-only version of it.

The rationale behind this is that if I'm providing some form of pointer to a function for example, I find it very useful to know whether that thing being pointed to is potentially going to be mutated, versus if it's just going to be read from. Also, when writing or debugging a function, I find that it helps to know which variables can change and which are fixed in place. It overall makes statement management far easier to reason about, and also discourages me from writing hacky overcomplicated code with excessive state transformations.

### "Logic" vs. "State" Source Files in ZGL

This is partly inspired by [this video I saw by Brian Will](https://www.youtube.com/watch?v=0iyB0_qPvWk).

As mentioned, ZGL modules have global state out of necessity, because the systems being encapsulated are inherently global in nature. But the actual public interface for each module has a mixture of functions that need to work with this global state, versus functions that only need to operate on the data that's explicitly passed to them. Thus, to help keep the source code implementing these modules clean and manageable, I have put the definition of the functions that interact with the global state (alongside the global state itself) in their own source file (suffixed with "_state.cpp"), and the definition of functions that don't (suffixed with "_logic.cpp") in another. I have tried to keep the "state" source files as minimal as possible, so as to minimise the amount of code manipulating global state.

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
