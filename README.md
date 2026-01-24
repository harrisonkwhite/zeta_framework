# Zeta Framework

This is a data-oriented framework for developing 2D games for Windows, Mac, and Linux.

It was originally written in C, though has been switched to a highly procedural subset of C++ 20 to leverage useful features like templates, operator overloading, function overloading, and so on.

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

## High-Level Structure (W.I.P.)

**zf_core_lib (ZCL):** A generic utilities library with essentials for ZF-style coding, like arenas, UTF-8 strings, etc. It also has some GFX and audio helpers so both ZGL and the asset builder can use them. Has *very* minimal global state.

**zf_game_lib (ZGL):** Depends on ZCL. More explicitly organised into modules, though not all modules have internal global state.

**zf_asset_builder:** Depends on ZCL. Builds ZF-specific asset files from raw ones like ".png", ".ttf", ".wav", etc. for use in ZGL. This is totally optional but recommended.

**zf_bin_to_array:** A simple tool that takes in a filename and spits out a C++ source file containing the binary blob as a constant. The only reason this exists is so that compiled shader files can be accessed in the code as binary blobs. But you might find it useful for something else.

**zf_tests:** Depends on ZGL and therefore ZCL. Has standard unit tests for things in both.

---

## Rationale (W.I.P.)

This section exists basically to explain why I created this framework in the first place, and what the rationale was behind particular non-obvious design decisions.

### Personal Background

I think laying out my personal history with programming might provide some context as to how and why I got to making this framework.

#### Influence of GameMaker: Studio

(Note that this is written based on the state of GameMaker: Studio and GameMaker: Studio 2 around the 2014 to 2020 period, I'm not sure what it's like today.)

So my first ever experiences with programming were as a child with the GameMaker: Studio engine. I made many small-to-mid-sized games with it, but the project that had the biggest influence on my programming was MANIC, which I started working on when I was about 14 and it took 2.5 years to complete. As the project progressed I found state management increasingly difficult. Part of this was of course my lack of experience as a programmer, but a lot of it I would attribute to the intensely global nature of that engine. For context, GameMaker itself is built on an object system. Considering that it is targeted for beginners, I think this very reasonable. I think humans see reality very much in terms of objects, and games are in a sense an abstraction of reality, so when a new person to programming comes along and wants to make a game, their mental model of a game is likely that it's a world comprised of objects. But the engine isn't object-oriented the sense that there are mechanisms for encapsulation and object-level public/private state distinction. Instead, any object can freely modify the state of any other object.

#### Influence of C# with XNA / MonoGame

After that project I wanted to "step up" into an engine/framework that was more of a challenge for me, which became MonoGame. MonoGame is an open-source continuation of the XNA Framework and uses C#.

My two main inspirations for choosing MonoGame were:

1. The livestreams Notch, the creator of Minecraft, did of him creating some Ludum Dare games. He used Java though, but that's still very close to C#.
2. The indie games already made with it, namely Terraria (which actually used XNA, but close enough) and Stardew Valley.

I was inspired by the livestreams that Notch, the creator of Minecraft, did for his games. He used Java.

Likewise with GameMaker, I made quite a few jam games with C# and MonoGame. But the largest project I worked on, which eventually got cancelled, was a project 

Personally I think that rather than emphasising objects, games are much better off organised in terms of explicit procedures, and the subset of read-only vs. mutable state that these procedures are exposed to.

I think MonoGame is a great framework, the only problem I have is that it uses C#. C# enforces object orientation, which at least for game development I think is quite counter-productive (I'll get to why below).

#### Conclusion

The basic designing principle of this framework was "GameMaker for more experienced programmers". So I wanted to at least get close to the same expressive power that GameMaker offers for 2D game development, but structure it for programmers who know how to effectively do memory management and set up systems tailored specifically to the game they are making. Another accurate framing of it would be "MonoGame but in C/C++".

#### Influence of RayLib

RayLib is not a framework that I have much personal experience in, but as an outsider there are some key differences between ZF and RayLib that I think are worth noting:
- RayLib both is and is marketed as being beginner-friendly and more built for toy projects. With ZF, I wanted to create a framework that could scale for more serious 2D indie projects.
- RayLib solely uses OpenGL, which has been deprecated on MacOS. I wanted ZF to truly be cross-platform at the very least on Windows, Mac, and Linux.
- RayLib is only in C. Although ZF is largely written C-style, I thought it'd be useful to leverage some of the useful features of C++ to make programming less of a hassle (see below).
- RayLib is very loose in its global state. This is a pain point I had when working with GameMaker. This is useful for small projects where you just want to get the job done fast, but for large projects becomes a serious pain. In ZF, certain systems are global by necessity, but there are systems in place (see below) to make it more manageable.

### Notable Design Decisions

There are some very important fundamental design decisions in ZF, many of which go against standard practice, that are worth explaining:

<br>

**Use of an STL "Replacement"**

The ZF core library is not at all "better" than the C++ STL (that would be a stupidly arrogant thing to say). The reason the STL is very minimally used is because it's written from a completely different design philosophy, one which emphasises RAII, object orientation, etc. The discrepancies between the design philosophies of the STL and ZF should become much clearer as you read on.

<br>

**"Procedural C++"**

This choice of language style was . The context that ZF actually began in C, not C++, might help to explain.

C++ does indeed have useful quality of life features even if 

<br>

**Heavy Emphasis on Read-Only vs. Mutable Data**

This is probably the most unusual aspect of the code base. I put "const" on absolutely everything, and I also have mutable vs. readonly variations on quite a few structs (in places where it's useful).

This is a technique that I learnt from reading John Carmack's article on functional programming, and it's honestly one of the best things I've ever adopted into my coding style.

I think his argument that the vast majority of bugs in programming can be attributed to the state not being what you think it is is very reasonable. And so it makes sense that you'd want set up mechanisms to .

I've personally found that the main benefit isn't actually the compile-error level of protection it gives you over state mutation, but more the way it changes how you think.

I conceptualise a function very much as a procedure that is exposed some set of read-only state.

So let's consider a function whose sole purpose is to render something. Let's say it's the root render function of your game.

What you can do is expose to this function a READ-ONLY VIEW of the game state. This makes it very clear that the sole purpose of the function is to, from a snapshot of the game state, produce a frame (which might take the form of a sequence of render instructions). If the game state instead wasn't marked as const, you might at some point be tempted to add some hidden little state mutation within a render function. For example, in a PlayerRender() function, you might mutate the player's rotation to point to the cursor... now all of a sudden your tick logic and render logic are dealing with a completely different versions of the player! You can "enforce" these things through documentation, sure, but compile-time checks via things like const are always going to be better in my opinion.

If I was ever to design my own programming language (and I did seriously consider that at one point), this would be hugely emphasised.

Although I do align with many of the design philosophies underpinning the Odin langauge, from my brief experiments with it, the decision to completely remove const from the language and any form of read-only vs. mutable control really puts me off.

<br>

**Memory Arenas**

So with games of the scope that I've worked with, I find it extremely simple to organise memory and resource lifetimes. For example, in an engine you can generally get away with a model of game-long lifetime memory, and frame-long lifetime memory. You'll often also need some kind of scratch space arena for temporary allocations.

Let's say, for example, in a particular function you need some kind of large array of a length dependent on function inputs. Let's also suppose that the maximum possible array length you would need in this case is too large to be stack-allocated. If you were to go the more conventional modern C++ route, you might have something like an std::vector which gets cleaned up implicitly when out of scope. There are 2 main issues I have with this. Firstly, the dynamic allocation is NOT free if the allocator has to work account for fragmentation. Secondly, by depending on a destructor you're adding obfuscation to what your code is actually doing.

I really think that with arenas in game development (at least at the scope I'm dealing), the pros of using this approach MASSIVELY outweigh the cons.

The biggest downside in my opinion to arenas is that they violate the notion of memory ownership. This can be really useful for object-level encapsulation.

And for a 

- Ownership
- Downsides (object-level encapsulation usually impossible)

<br>

**Resource Groups**

So in ZGL, whenever you want to create a GFX resource (e.g. a texture, a shader program, etc.) you need to supply a "GFX resource group". Likewise for sound types, if you want to create one you need to supply a "sound type group".

These "groups" are very similar in nature to memory arenas, in the sense that resources are continuously "pushed" onto them and all resources of the group can be freed at once.

More details on how they are implemented can be found in the code.

<br>

**Defer**

This is inspired by my brief time working with the language Odin (I think Go might have this as well, though I've never used that language).

In ZF, there is a ZCL_DEFER helper macro that essentially defers running the given function body until the end of the scope (internally it's literally just a struct containing a function pointer that gets called in the destructor).

This is really useful for STANDALONE resource allocations where you want to make sure you don't forget to deallocate, most notably file streams.

It is a bit of a controversial concept however. The main counterargument, which has been posed by Eskil Steenberg, is that they add more obfuscation to the control flow and make it more challenging to mentally map C code to assembly instructions. I agree that this is a downside, however I think the benefit of being more certain that something gets deallocated outweighs it, which I why I like to (sparingly) use it in ZF.

Controversial, wow!

<br>

**Module-Level Encapsulation**

Wow!

<br>

**Custom String System (UTF-8 and Non-Terminated)**

Great idea!

<br>

**Custom Print Format Function**

Yeah!

<br>

**Tickets**

Wow!

Certain modules/subsystems in ZGL have encapsulated global state by necessity, namely the platform, GFX, and audio modules.

As I learnt from my experience developing a relatively large GameMaker project, untamed global state can very easily destroy the stability of a project.

To fix this, I set up a "ticket" system. Essentially, if a function wants to mutate the global state of one of these subsystems (via their public interfaces), it has

<br>

**Asset Builder**

Wow!

<br>

**Template and Concept Usage in ZCL**

Kinda needed!

<br>

**Misc. Style Choices**

There are strictly no methods at all aside from operator overloads (including cast operators), even in cases where there is a very strong correlation between function and data type (e.g. things like HashMapPut()). This is really just to keep the codebase consistent and predictable from a cosmetic standpoint.

<br>

**Fatal Errors**

Was quite hard for me to do!
No exceptions!

<br>

**ABSENT Features**

There are some common game engine features intentionally absent from ZF. Most significantly, there is no form of ECS nor any scene system.

As established, this framework is not designed for beginners. I think that the way you would go about entity systems is very much dependent on the specific game you're making, and in particular cases you don't even need such a system. You might argue that an optional generic ECS could be useful for game jams and prototypes, but I disagree - in such a context you're generally better off not using any kind of entity system and just going with the simplest, dumbest system (e.g. an array of entity structs each containing a discriminated union on the entity type).

There is no "scene" or "room" system for the same reason - the way you set this up is very game specific.

<br>

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
