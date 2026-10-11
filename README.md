# learning-sdl

A bare-minimum SDL2 + OpenGL program: one window, one square you can move
with the keyboard. No engine or abstractions on top — everything lives in
[src/main.c](src/main.c) so you can see how a window, a GL context, a
shader, and a game loop fit together.

It reuses the same shared libraries as `engine-from-scratch`
(`C:\work\include`, `C:\work\lib` — SDL2 and glad), but nothing in that
project was touched.

## Build

Open an **x64 Native Tools Command Prompt for VS 2022**, then from this
folder:

```
build
```

This compiles `src/main.c` and `src/glad.c` with `cl.exe` and writes
`learning-sdl.exe` (plus `.obj`/`.pdb`/`.ilk` files) into this folder.

## Run

```
learning-sdl.exe
```

(`SDL2.dll` lives here too, next to the exe, so it can be found at runtime.)

## Controls

- Arrow keys / WASD — move the square
- Escape or closing the window — quit

## Clean

```
clean
```

Removes the compiled `.exe`/`.obj`/`.pdb`/`.ilk` files.

## Where to go next

- `src/main.c` is organized in five commented sections: window/GL setup,
  quad geometry, shader compilation, the main loop, and cleanup.
- The vertex shader converts a pixel-space position straight to normalized
  device coordinates instead of using a projection matrix — that's the one
  piece of GL math worth understanding first.
- Once this makes sense, `../engine-from-scratch/src` shows the same ideas
  factored out into a real engine (batched rendering, sprite sheets,
  entities, physics, audio).
