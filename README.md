# Flux Studio

Flux Studio is a native **C++ / Qt 6** professional 2D drawing, animation, and compositing workstation.

> **DRAW → ANIMATE → COMPOSE → EXPORT**

The architecture is deliberately shared-core so future products can become **Flux Draw**, **Flux Animate**, and the bundled **Flux Studio** without rewriting the engine.

## Current milestone — Core engine

The current `main` branch includes the first advanced core layer:

- GPU-backed desktop canvas via `QOpenGLWidget`
- Tile-based canvas cache and compositing engine
- Canvas zoom, fit, rotation and mirrored views
- Large-document rendering model
- Grid, rulers and perspective-guide overlays
- Reference-image loading
- Horizontal/vertical symmetry drawing
- Pixel-perfect rendering mode
- Pressure-aware brush engine
- Tablet pressure, tilt and rotation input
- Pressure-driven size and opacity dynamics
- Spacing, jitter and scatter
- Wetness controls and texture-brush support
- Custom brush preset model
- Flux brush import/export (`.fluxbrush` / JSON)
- Interactive Flux Brush Editor
- Stabilization controls
- Rectangle and lasso selection
- Contiguous/color selection engine
- Add/subtract/intersect selection operations
- Selection masks
- Transform matrix with move/scale/rotate/shear support
- Transform handles overlay
- Real raster canvas integration
- Undo/redo for edits
- Real layer model with visibility, opacity, locking and duplication
- Multi-frame layer storage and onion skin
- Animation frame timeline and playback
- Native `.flux` project persistence
- Autosave/recovery file path
- PNG / JPEG / WebP image export
- Dockable Layers, Inspector and Timeline panels
- Zhen startup artwork and loading screen

## Drawing engine

The brush subsystem is independent of the Qt window so it can later be reused by **Flux Draw**, **Flux Animate**, and **Flux Studio**.

Supported brush controls include:

- Size
- Opacity
- Flow
- Spacing
- Jitter
- Scatter
- Wetness
- Texture strength
- Stabilization
- Pressure-to-size
- Pressure-to-opacity
- Tilt-to-size
- Velocity-to-opacity
- Texture image input

## Canvas engine

Flux uses a native OpenGL-backed widget with a reusable tiled rendering layer. The canvas coordinate system supports zoom, pan, rotation, mirroring and document-centered transforms while keeping drawing coordinates separate from screen coordinates.

The editor also exposes grid/rulers/perspective guides, symmetry and reference-image workflows.

## Selection and transforms

The selection engine supports rectangular, lasso and contiguous selection with replace/add/subtract/intersect modes. The transform subsystem provides a reusable `QTransform`-based foundation for move, scale, rotate and shear operations with visual transform handles.

## Product architecture

```text
                         FLUX CORE
                             │
              ┌──────────────┼──────────────┐
              │              │              │
          Drawing         Animation      Projects
           Engine          Engine         / Assets
              │              │              │
              └──────────────┼──────────────┘
                             │
             ┌───────────────┼───────────────┐
             ▼               ▼               ▼
         Flux Draw      Flux Animate      Flux Studio
```

## Build

Requires Qt 6.5+ with Widgets, OpenGLWidgets and SVG, plus CMake 3.21+.

```bash
cmake -S . -B build
cmake --build build --config Release
```

This is still an actively developed engineering build. Production-grade video/audio pipelines, advanced project recovery UX, full Wacom/Windows Ink platform integration, vector editing, node compositing, GPU brush kernels, advanced project version migration and production animation export remain later layers.
