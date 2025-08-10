# Zeta Framework

This is a simple framework in C for developing 2D games using OpenGL.

You can find it being put to use in my [clone of Terraria](https://github.com/harrisonkwhite/terraria_clone).

There is also a [template available](https://github.com/harrisonkwhite/zfw_game_template) for making a new game with this framework.

> **Note:** I made this primarily for learning purposes and for my own use. If you want to make your own games in C, I would highly recommend instead using the more substantial [Raylib](https://github.com/raysan5/raylib).

---

## Building

Building this project has been tested on Windows and Linux.

You can follow the process you normally would with CMake:

```
mkdir build
cd build
cmake ..
```

For Linux, there are a number of dependencies you might need to manually install. CMake will report if any are missing.

---

## Third-Party Projects

- [GLFW](https://github.com/glfw/glfw)
- [glad](https://github.com/Dav1dde/glad)
- [stb](https://github.com/nothings/stb)
