# Flux Studio — Feature Audit

This is the authoritative implementation checklist for the product. Features are split into implemented foundations, production subsystems, and remaining engineering work. A feature is not considered complete merely because a button exists; it must operate correctly, survive save/load where appropriate, and work in the packaged build.

## 0.9 production additions

- Persistent application preferences store
- Shortcut registry with conflict detection
- UI density, accessibility, contrast, color-assist and canvas-workflow preference model
- Clipboard image interchange with transparent RGBA preservation
- Flux layer clipboard payload format
- Color-management validation primitives
- sRGB / Display P3 / Rec.709 / Linear sRGB profiles in export model
- Full/Limited range model
- Linear-workflow and premultiplied-alpha export flags
- Deterministic export metadata model
- Editable cubic vector path geometry
- Vector node/tangent model suitable for advanced path editing
- Interactive render queue dock with job add/remove/reorder controls
- Queue cancellation and frame-progress reporting
- Windows runtime deployment verification for required Qt DLLs
- Windows 0.9.0 packaging pipeline

## Working foundations already present

### Document and canvas
- Native C++20 / Qt 6 desktop architecture
- Raster canvas
- GPU-backed `QOpenGLWidget` viewport
- Tile-based canvas architecture
- Zoom and fit-to-view
- Pan
- Canvas rotation
- Horizontal/vertical mirror
- Grid
- Rulers
- Perspective-guide toggle
- Reference-image loading
- H/V symmetry
- Pixel-perfect mode
- Pressure-aware brush input
- Pressure size/opacity dynamics
- Tilt and barrel/eraser input hooks
- Brush spacing, jitter, scatter, wetness and texture parameters
- Stabilization
- Custom brush presets
- Brush Editor
- `.fluxbrush`/JSON brush import/export foundations

### Selection and transform
- Rectangle selection
- Lasso selection
- Contiguous selection foundation
- Add/subtract/intersect selection foundation
- Selection masks
- Transform foundation

### Layers
- Paint layers
- Group layers
- Raster masks
- Vector-mask foundation
- Adjustment-layer foundation
- Visibility
- Lock
- Opacity
- Blend modes: Normal, Multiply, Screen, Overlay, Add, Subtract
- Alpha inheritance
- Clipping
- Solo/isolate
- Layer label color
- Basic layer style
- Duplicate
- Delete
- Merge down
- Flatten visible
- Layer hierarchy drag/drop foundation

### Animation
- Multi-track animation model
- Keyframes
- Holds
- Interpolation
- Markers
- Scenes
- Shots
- Insert/delete/duplicate frames
- Dope-sheet/exposure timeline foundation
- Graph-mode foundation
- Playback
- FPS control
- Onion skin

### Compositing
- Compositor data model
- Image/source flow
- Color correction foundation
- Blur
- Glow
- Transform
- Shadow/effect foundations
- Levels/curves/hue-saturation node type foundations
- Mask/precomp node type foundations
- JSON compositor model foundation

### Media
- FFmpeg/FFprobe detection backend
- Audio metadata inspection
- Audio waveform generation backend
- PCM decode backend
- Video metadata inspection
- Video frame extraction backend

### Export
- PNG
- JPEG
- WebP
- PNG sequence backend
- Sprite-sheet backend
- GIF
- MP4
- WebM
- SVG raster export
- Render settings model
- Render queue data model

### Project/recovery
- `.flux` manifest + asset sidecar persistence
- Validation/migration foundation
- Atomic save foundation
- Backup/recovery directories
- Autosave timer
- Crash marker
- Recovery snapshots
- Recent projects
- Named workspace storage foundation
- Single-file Flux package container (`FluxProjectPackage`)

### Application shell
- Project Hub home screen
- New Project dialog
- Open Project
- Recent project list
- Professional top command bar foundation
- Vertical tool rail
- Dockable Layers/Inspector/Timeline/Compose panels
- Production Center dock
- Command Palette foundation
- Configurable Quick Wheel foundation
- Dark professional theme
- High-DPI-friendly minimum workspace size

## Production engineering backlog

### 1. Rendering/performance
- True dirty-region renderer
- Dirty-region tile invalidation
- Worker-thread tile rendering
- Render cancellation and prioritization
- GPU paint/composite path
- Persistent GPU texture cache
- Proxy-resolution mode affecting actual render resolution
- Real live FPS measurement
- Real frame-time measurement
- Accurate cache-memory accounting
- Memory-pressure policy and configurable limits
- Background render workers
- Render task scheduler
- Render telemetry capture

### 2. Canvas quality/navigation
- Zoom centered around cursor
- Smooth zoom animation
- Rotation snapping and arbitrary rotation UX
- Canvas background/checkerboard modes
- Pan inertia/touchpad gesture handling
- Draggable guides
- Editable perspective-grid handles
- Ruler units and measurement overlays
- Multiple reference images with transforms, opacity and locking
- Navigator/minimap
- Safe-area/action-area overlays
- Camera framing guides
- Custom canvas color

### 3. Drawing/tool completeness
- Production line tool
- Rectangle tool
- Ellipse tool
- Polygon tool
- Star/polystar tool
- Bezier/path tool
- Gradient tool
- Bucket/flood fill
- Color sampler modes and sample radius
- Text tool
- Transform handles
- Free transform
- Warp/distort/perspective transform
- Multi-layer transform behavior
- Shape snapping
- Advanced vector layer/path editing
- Shape boolean operations
- Stroke expansion
- Pathfinder-style combine modes
- Perspective-aware drawing assist
- Symmetry presets beyond horizontal/vertical

### 4. Brush system
- Brush-library browser with thumbnails
- Brush categories/tags/search
- Preset favorites
- User brush libraries
- Brush preview renderer
- Texture tiling/mapping controls
- Better dab stamping for high-speed strokes
- Brush dynamics curve editors
- Per-device pressure curves in the UI
- Brush preset persistence inside `.flux` package
- Brush groups and nested tags
- Import of common brush interchange formats where licensing permits
- Wet-mix/color-smear brush behavior
- Dual-brush engine
- Stamp rotation by path direction

### 5. Layer system
- True multi-selection operations
- Batch opacity/blend/visibility changes
- Robust hierarchy-aware reparenting
- Layer insertion/reordering semantics identical to pro editors
- Nested group compositing
- Group masks
- Group blend isolation
- Rich styles: inner/outer shadow, glow, outline, bevel
- Non-destructive levels/curves/hue-saturation adjustment layers
- LUT/color-look adjustment layer
- Editable vector masks
- Mask painting UI
- Mask feather/expand/contract controls
- Layer color labels with presets
- Clipping-chain visualization
- Reference/guide layers
- Filter masks
- Layer search/filter

### 6. Animation system
- Animation properties bound to actual document/layer properties
- Persistent animation tracks in `.flux`
- Full Bezier graph editor
- Tangent editing
- Auto/linear/constant/smooth/break tangent modes
- Value and time snapping
- Retiming tool
- Speed ramps
- Motion paths
- Exposure-number editing
- Hold/step editing
- Marker editing and colors
- Scene/shot browser
- Camera track
- Multi-property tracks
- Animation clipboard
- Consistent playback vs final render evaluation
- Nested compositions
- Animation presets
- Cycle/repeat expressions
- Time warp/remap
- Keyframe scaling/stretching

### 7. Timeline/media editing
- Audio clips placed directly on timeline
- Waveform drawing in timeline
- Audio playback/scrubbing
- Video clips on timeline
- Video thumbnails
- Media trim/in/out controls
- Proxy video generation
- Media cache
- Timeline snapping
- Audio sync and frame-accurate scrubbing
- Ripple edits
- Slip/slide edits
- Track enable/mute/solo/lock
- Clip markers
- Timeline zoom and navigator
- Multiple scene timelines

### 8. Compositor
- Real node canvas using graphics scene/view
- Draggable nodes
- Port/socket UI
- Bezier wires
- Multi-input nodes
- Node selection/multi-select
- Pan/zoom canvas
- Node alignment/distribution
- Node groups
- Graph save/restore
- Node position persistence
- Actual branching graph evaluation
- Precomps
- Render passes
- Time/remap nodes
- Levels/curves/hue-sat implementation
- Mask/combine nodes
- Chroma/keying node
- Blend/merge node
- Transform node with full 2D controls
- Color-space-aware processing
- Track matte nodes
- Morphology nodes
- Convolution/sharpen nodes
- LUT node
- Noise/grain node
- Distortion/displacement nodes
- Vector-to-raster and raster-to-vector bridges

### 9. Render queue
- Queue dock
- Add/remove/reorder jobs
- Job progress
- Per-job progress bars
- Pause/cancel
- Retry failures
- Background worker threads
- Multiple output presets
- Output folder validation
- Render logs
- Completion notifications
- Queue persistence
- Concurrent render slots
- Priority scheduling
- Failure retry policy
- Estimated time remaining
- Disk-space preflight
- Output collision policy

### 10. Export/color management
- Full frame range UI
- Start/end/step controls
- Output filename patterns
- Codec profile presets
- Audio muxing
- Alpha/premultiplication control
- Linear vs sRGB handling
- Color range control
- Color-space metadata
- Export validation
- Render preview
- True editable SVG/vector export
- EXR export
- TIFF export
- APNG export
- ProRes/other codec profiles where FFmpeg supports them
- Alpha-capable video presets
- Safe color conversion pipeline
- Embedded profile metadata

### 11. Project format
- Embed images/audio/video/brushes/previews directly in one package
- Package manifest with explicit checksums
- Content-addressed assets
- Incremental package updates
- Package repair mode
- Version migration tests
- File corruption detection
- Project thumbnails/previews
- Project browser with thumbnail grid
- Non-blocking save worker
- Package compression options
- Asset deduplication
- Transaction journal
- Autosave generations
- Portable relative-path repair

### 12. Recovery
- Automatic startup recovery detection
- Recovery browser dialog
- Preview each snapshot
- Restore snapshot as new project
- Compare recovery vs saved project
- Background snapshot thread
- Snapshot deduplication
- Automatic cleanup/retention policy
- Recovery health indicator
- Recovery checksum validation
- Last-known-good document fallback

### 13. Workspace/preferences
- Real workspace geometry/state serialization
- Workspace switching
- Workspace import/export
- Keyboard shortcut editor
- Tool preset manager
- UI density control
- Theme variants
- Accessibility sizing
- Color-blind-friendly UI options
- Preferences dialog
- Per-workspace shortcut maps
- Startup behavior settings
- Autosave settings panel
- Performance budget settings
- Reset individual preference groups

### 14. Input/tablets
- Live device discovery UI
- Device-specific profile creation
- Device-specific primary/barrel/eraser mappings
- Pressure-curve editor
- Tilt mapping editor
- Rotation mapping editor
- Windows Ink native behavior integration
- Wacom native SDK integration where licensed/available
- Platform-specific Apple Pencil integration for supported ports
- Multi-tablet profile switching
- Stylus button visualizer
- Pressure test panel
- Raw-vs-processed pressure preview

### 15. Commands/help
- Register every UI action with Command Palette
- Global shortcut conflict detection
- Configurable shortcuts
- Searchable settings
- Context-sensitive help
- Complete Help/About system
- User documentation
- First-run onboarding
- Keyboard shortcut cheat sheet
- Tooltips with shortcut hints
- Diagnostic report generator

### 16. Clipboard/import
- Copy/paste selected pixels
- Copy/paste layers
- Transparent clipboard handling
- Drag/drop image import
- Drag/drop project import
- External application paste compatibility
- Multi-format clipboard targets
- Clipboard history
- Pasted-layer naming rules
- Import scaling policy
- Image orientation metadata handling

### 17. Assets/library
- Project asset browser
- Imported media deduplication
- Asset relinking
- Missing-asset report
- Search/filter assets
- Thumbnail generation
- Asset usage audit
- Brush/texture/material library manager
- Reference board
- Palette library

### 18. Color/palette
- Color wheel
- Sliders/HSB/HSL/OKLCH controls
- Palette extraction from image
- Palette library
- Swatch naming
- Recent colors
- Harmony generation
- Color history
- On-canvas color picker
- Gamut warning display

### 19. Testing/quality
- Unit tests for document serialization
- Unit tests for layer compositing
- Unit tests for animation evaluation
- Unit tests for selection
- Unit tests for package format
- Import/export golden tests
- Recovery tests
- Render determinism tests
- Windows packaged smoke test
- Crash/startup smoke test
- Performance regression benchmark
- Large-document stress tests
- Fuzz tests for project/package parsing
- Corrupt asset tests
- Memory leak checks
- UI interaction smoke tests

### 20. Cross-platform release
- Linux build/package pipeline
- macOS build/package pipeline
- Platform-specific installer/signing workflows
- Runtime dependency validation on each platform
- Native file associations
- Native drag/drop behavior
- Native high-DPI behavior
- Wayland/X11 validation
- macOS sandbox/signing/notarization path

### 21. Professional workflow extras
- Fullscreen canvas mode
- Presentation mode
- Reference-only viewing mode
- Before/after comparison
- Split-view comparison
- A/B render preview
- Safe-mode startup
- Diagnostic mode
- Crash reporter with local logs
- Project statistics
- Document size estimator
- Undo history viewer
- Action history journal
- Batch processing
- Command-line render mode
- Headless rendering
- Scriptable automation hooks
- Plugin API foundation
- Extension manager
- Localization framework

### 22. Future Flux ecosystem
- Shared Flux Core API
- Flux Draw product target
- Flux Animate product target
- Bundled Flux Studio product target
- Common brush/document/project formats
- Cross-product preset portability
- Plugin SDK
- Developer console
- Extension sandboxing

## Definition of feature-complete

Flux Studio should not be called feature-complete until every item above either:

1. is implemented and verified in the packaged application, or
2. is explicitly marked platform/licensing dependent with the best practical fallback implemented.
