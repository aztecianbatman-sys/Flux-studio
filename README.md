# Flux Studio

Flux Studio is a native **C++ / Qt 6** professional 2D drawing, animation, and compositing workstation.

> **DRAW → ANIMATE → COMPOSE → EXPORT**

The architecture is deliberately shared-core so future products can become **Flux Draw**, **Flux Animate**, and the bundled **Flux Studio** without rewriting the engine.

## Current milestone — Production foundations

The `main` branch now includes the advanced drawing/layer core plus production foundations for animation, compositing, media, export, projects, recovery, tablets, commands, performance, and workflow persistence.

### Core / layers
- GPU-backed desktop canvas via `QOpenGLWidget`
- Tile-based canvas cache and compositing engine
- Canvas zoom, fit, rotation and mirrored views
- Grid, rulers, perspective guides and reference images
- Pressure-aware brush engine with tilt/rotation dynamics
- Custom `.fluxbrush` / JSON brush presets and editor
- Selection and transform systems
- Nested layer hierarchy and drag/drop ordering
- Masks, vector-mask layer entries, adjustments and blend modes
- Layer locking, clipping, alpha inheritance, styles, solo/isolate
- Multi-frame layer storage and onion skin

### Animation
- Dedicated animation model and timeline widget
- Dope-sheet / exposure presentation
- Multiple tracks and animated properties
- Keyframes, holds and interpolation data
- Frame insertion, deletion and duplication
- Timeline markers
- Scene and shot model
- Graph-editor mode foundation

### Compositing
- Node-based compositor graph model
- Image → Color → Blur → Glow → Transform → Output pipeline
- Color correction, blur, glow and transform processing
- Compositor UI with node creation and parameter editing
- JSON graph persistence model

### Media / export
- FFmpeg-backed audio/video inspection and decoding hooks
- Audio waveform generation
- Video frame extraction
- PNG/JPEG/WebP image export
- PNG sequence rendering
- GIF/MP4/WebM encoding path through FFmpeg
- Render settings for resolution, FPS, bitrate, codec, transparency and frame range
- Render queue subsystem
- Sprite-sheet and SVG export bridges

### Project / recovery
- Versioned project-manager API with migrations
- Project validation
- Atomic writes through `QSaveFile`
- Rotating project backups
- Recovery crash markers and snapshot rotation
- Portable project-directory layout foundation
- Recent-project and workspace persistence

### Tablet / workflow / performance
- Qt tablet pressure, tilt, rotation, stylus buttons and eraser handling
- Per-device tablet profile and pressure-curve persistence
- Radial Flux Wheel hit testing and hover selection
- Searchable command palette widget
- GPU capability/performance probe
- Recent projects and workspace persistence

## Architecture

```text
                         FLUX CORE
                             │
          ┌──────────────────┼──────────────────┐
          │                  │                  │
       Drawing            Animation         Compositing
       Engine              Engine              Engine
          │                  │                  │
          └──────────────────┼──────────────────┘
                             │
                     Projects / Media
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

Video/audio features use an external **FFmpeg/FFprobe** installation available on `PATH`.

This remains an actively developed engineering build. Full production-grade vector editing, advanced GPU kernels, complete Wacom/Windows Ink platform-specific integrations, HDR/color-management pipelines, deep graph editing, scripting/plugin SDKs, localization/accessibility, and native video-layer workflows remain future expansion areas.
