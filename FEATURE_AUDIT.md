# Flux Studio — Feature Audit

This is the authoritative implementation checklist for the product. Features are split into working foundations, current production shell, and remaining engineering work. A feature is not considered complete merely because a button exists; it must operate on the document, survive save/load where appropriate, and work in the packaged build.

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

## Current production-shell additions

- Production Center with Project / Animation / Media / Export / Performance / Input / Workspace tabs
- Single-file package import/export path
- Recovery snapshot inspection entry
- Media import/inspection UI
- Render settings UI
- Proxy/cache/background-render preference controls
- Live process-memory diagnostics
- Quick Wheel radius persistence
- Saved tablet-profile browser
- Windows Ink preference storage
- Workspace preset storage
- Windows x64 CI build + NSIS packaging

## Remaining engineering work — do not mark these complete yet

### 1. Rendering/performance
- True dirty-region renderer
- True tile cache invalidation by changed regions rather than whole document
- Worker-thread tile rendering
- Render cancellation and prioritization
- GPU paint/composite path instead of CPU composition for every frame
- Persistent GPU texture cache
- Proxy-resolution mode actually changing render resolution
- Real live FPS measurement
- Real frame-time measurement
- Accurate cache-memory accounting
- Memory-pressure policy and configurable limits

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

### 3. Drawing/tool completeness
- Production line tool
- Rectangle tool
- Ellipse tool
- Polygon tool
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

### 12. Recovery
- Automatic startup recovery detection
- Recovery browser dialog
- Preview each snapshot
- Restore snapshot as new project
- Compare recovery vs saved project
- Background snapshot thread
- Snapshot deduplication
- Automatic cleanup/retention policy

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

### 15. Commands/help
- Register every UI action with Command Palette
- Global shortcut conflict detection
- Configurable shortcuts
- Searchable settings
- Context-sensitive help
- Complete Help/About system
- User documentation

### 16. Clipboard/import
- Copy/paste selected pixels
- Copy/paste layers
- Transparent clipboard handling
- Drag/drop image import
- Drag/drop project import
- External application paste compatibility

### 17. Testing/quality
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

### 18. Cross-platform release
- Linux build/package pipeline
- macOS build/package pipeline
- Platform-specific installer/signing workflows
- Runtime dependency validation on each platform

## Definition of feature-complete

Flux Studio should not be called feature-complete until every item above either:

1. is implemented and verified in the packaged application, or
2. is explicitly marked platform/licensing dependent with the best practical fallback implemented.
