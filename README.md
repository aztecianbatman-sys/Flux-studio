# Flux Studio

Flux Studio is a native **C++ / Qt 6** professional 2D drawing, animation, and compositing workstation.

> **DRAW → ANIMATE → COMPOSE → EXPORT**

The architecture is deliberately shared-core so future products can become **Flux Draw**, **Flux Animate**, and the bundled **Flux Studio** without rewriting the engine.

## Current build

The `main` branch contains a working desktop editor foundation with:

- Modern dark professional UI optimized for large screens
- Zhen startup artwork and branded loading screen
- Real raster canvas backed by `QImage`
- Brush, pencil, ink and eraser drawing
- Adjustable brush size and foreground color
- Canvas zoom and fit-to-window
- Right-click Flux Wheel
- Undo / redo for strokes
- Real layer model with visibility, opacity, locking and duplication
- Multi-frame layer storage
- Onion-skin preview
- Animation frame strip and frame selection
- Frame duplication and playback
- FPS control
- Native `.flux` project metadata + frame-image persistence
- Save / Save As / Open
- Automatic periodic autosave
- PNG / JPEG / WebP image export
- Dockable Layers, Inspector and Timeline panels
- Workspace state persistence
- Cross-platform CMake project structure

## Product scope

### Drawing

Pencil, pen, ink, marker, brush, airbrush, eraser, smudge, blur, fill, gradient, picker, shapes, Bezier, selection, transform, crop, stabilization, pressure/tilt abstraction, brush dynamics, custom brushes, brush presets and palettes.

### Layers

Groups, nested groups, clipping, masks, adjustment layers, blend modes, alpha lock, locking, opacity, thumbnails, reordering, multi-selection and duplication.

### Color

RGB / HSV / HSL / HEX, color wheel, value selection, gradients, palettes, recent colors, foreground/background colors and color history.

### Flux Wheel

Right-click or press-and-hold interaction with configurable commands, favorite brushes and color access. The wheel is intended to become context-sensitive for drawing, object editing and animation workflows.

### Animation

Frame-by-frame drawing, holds, exposure-sheet workflow, onion skin, keyframes, dope-sheet timeline, graph editor, camera animation, audio tracks and scene management.

### Compositing

Blend modes, masks, transforms, effects, color correction, glow, shadows and animated effects, with a future node-compositor layer planned on top of the shared core.

### Projects and recovery

Portable `.flux` projects, thumbnails/assets, project metadata, autosave, recovery snapshots, history and future crash-recovery UI.

### Performance architecture

The long-term engine targets GPU canvas rendering, tile-based rendering, cached thumbnails/frames, background export, background autosave, multithreading, large-canvas optimization and adaptive quality.

### Cross-platform roadmap

Desktop first:

- Windows
- macOS
- Linux

Shared core is intended to support future tablet/mobile/web front ends without replacing the document and rendering model.

## Build

Requires Qt 6.5+ with Widgets and SVG and CMake 3.21+.

```bash
cmake -S . -B build
cmake --build build --config Release
```

The current build is an engine-backed desktop prototype, not yet a finished commercial-grade replacement for established suites. Advanced video/audio I/O, GPU acceleration, vector editing, node compositing, full tablet APIs and production export pipelines are the next engineering layer.
