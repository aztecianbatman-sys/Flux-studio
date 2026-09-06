# Flux Studio
Flux Studio is a native **C++ / Qt 6** professional 2D drawing, animation, and compositing workstation.

> **DRAW → ANIMATE → COMPOSE → EXPORT**

The architecture is deliberately shared-core so future products can become **Flux Draw**, **Flux Animate**, and the bundled **Flux Studio** without rewriting the engine.

## Windows installer

Windows x64 installer packaging is built automatically through GitHub Actions. The pipeline builds the repository source as-is, deploys Qt, creates the NSIS installer, and publishes a portable ZIP. Source compatibility fixes live in the repository rather than being generated or pushed by CI.

Windows build pipeline refreshed — 2026-09-06.
