# Zeta Framework

This is a framework in C++ for developing 2D games, developed mostly for personal use.

You can find it being put to use in my in-development game [Kaōzeth](https://github.com/harrisonkwhite/kaozeth).

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

- [GLFW](https://github.com/glfw/glfw)
- [glad](https://github.com/Dav1dde/glad)
- [cJSON](https://github.com/DaveGamble/cJSON)
- [stb](https://github.com/nothings/stb)
- [miniaudio](https://github.com/mackron/miniaudio)
