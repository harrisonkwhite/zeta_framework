# Zeta Framework

This is an extensive data-oriented framework in C++ for developing 2D games.

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

- [GLFW](https://github.com/glfw/glfw) for cross-platform windowing and input (plus an OpenGL context)
- [glad](https://github.com/Dav1dde/glad) for OpenGL function pointers
- [cJSON](https://github.com/DaveGamble/cJSON) for JSON parsing in the asset packer
- [stb](https://github.com/nothings/stb) for raw image and font file loading
- [miniaudio](https://github.com/mackron/miniaudio) for audio loading and playing

---

## Future Plans

- Networking features
- Better cross-platform support via supporting other graphics APIs like Metal
