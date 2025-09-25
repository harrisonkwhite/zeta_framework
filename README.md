# Zeta Framework

This is a simple framework in C++ for developing 2D games using OpenGL.

You can find it being put to use in my game [Kaōzeth](https://github.com/harrisonkwhite/terraria_clone).

There is also a [template available](https://github.com/harrisonkwhite/zfw_game_template) for making a new game with this framework.

---

## Building

Clone the repository by running `git clone --recursive https://github.com/harrisonkwhite/zeta_framework.git`.

> **Note:** Compiling this project has only been tested on Windows with MSVC.

> **Note:** If the repository was cloned non-recursively before, just run `git submodule update --init --recursive` to clone the necessary submodules.

Then go into the repository root and build with CMake:

```
mkdir build
cd build
cmake ..
```

---

## Third-Party Projects

- [GLFW](https://github.com/glfw/glfw)
- [glad](https://github.com/Dav1dde/glad)
- [stb](https://github.com/nothings/stb)
