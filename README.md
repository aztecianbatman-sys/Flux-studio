# Flux Studio

Flux Studio is a professional, local-first 2D drawing, animation, and compositing workstation built with **C++ and Qt 6**.

## Direction

**DRAW → ANIMATE → COMPOSE → EXPORT**

The project is designed around a portable Flux Core so future Flux Draw and Flux Animate products can share the same engine and `.flux` project format.

## Current milestone

This repository starts the native Qt desktop foundation:

- Modern dark creative-app shell
- Zhen branded startup/loading screen
- Dockable editor workspace
- Canvas viewport foundation
- Layers, inspector, toolbar, and timeline shells
- Flux Wheel interaction foundation
- Autosave/recovery architecture placeholders
- Cross-platform CMake build

## Build

Requires Qt 6 (Widgets and Svg) and CMake 3.21+.

```bash
cmake -S . -B build
cmake --build build --config Release
```
