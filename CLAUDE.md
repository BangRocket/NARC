# NARC - Not Another RayCaster: Deep Code Analysis

## Project Overview

**NARC** is a sophisticated 2.5D game engine written in modern C++ that creates retro-style first-person shooter games reminiscent of Wolfenstein 3D and early Doom. The project demonstrates advanced game development techniques while maintaining a nostalgic aesthetic.

## Architecture & Technology Stack

### Core Technologies
- **Language**: C++17/20
- **Graphics**: OpenGL 4.5 Core (hardware accelerated, shader-based rendering)
- **Platform**: Windows (Visual Studio 2022)
- **Build System**: MSBuild/Visual Studio project files
- **Key Dependencies**: 
  - Custom lwmf (lightweight media framework) - author's own library for window management and OpenGL context
  - stb_truetype (modified for C++ compatibility)
  - SIMD intrinsics (requires SSE 4.2 support)

### Project Structure
```
NARC/
├── Sources/              # Core engine source code (~13,937 lines total)
│   ├── lwmf/            # Custom media framework library
│   ├── stb/             # Modified STB libraries
│   └── *.hpp/cpp        # Game engine modules
├── DATA/                # Game configuration and assets
│   ├── Assets_*/        # Entity, weapon, door configurations
│   ├── GameConfig/      # Engine configuration files
│   └── Levels/          # Level definitions and maps
├── GFX/                 # Graphics assets (textures, sprites)
├── SFX/                 # Sound effects and music
└── Fonts/               # TrueType fonts for UI
```

## Key Components & Implementation Details

### 1. Rendering Engine (`Sources/Game_Raycaster.hpp`)
- **Advanced Raycasting**: Implements a sophisticated raycasting algorithm for 2.5D graphics
- **Multi-threaded Rendering**: Uses thread pools for parallel ray processing
- **Rendering Passes**: Separate passes for walls, floor, ceiling with different optimization strategies
- **Dynamic Effects**: Real-time lighting, fog-of-war, texture mapping
- **Texture Support**: Variable sizes from 64x64 to 8192x8192 pixels
- **Z-Buffer**: Custom implementation for proper sprite/wall occlusion

### 2. Entity System (`Sources/Game_EntityHandling.hpp`)
- **Entity Types**: 
  - Enemies: Soldier, Demon (with AI behaviors)
  - Neutral: AmmoBox (pickups)
  - Static: Turret (defensive structures)
- **Animation System**: Frame-based animations for walking, attacking, dying
- **Hit Detection**: Pixel-precise collision detection
- **Entity Management**: Efficient sorting and culling for rendering

### 3. AI & Pathfinding (`Sources/Game_PathFinding.hpp`)
- **A* Algorithm**: Full implementation with diagonal movement support
- **Dynamic Pathfinding**: Real-time path calculation as player moves
- **Efficient Storage**: Uses flattened arrays for map representation
- **Enemy Behaviors**: Chase, patrol, attack patterns

### 4. Weapon System (`Sources/Game_WeaponHandling.hpp`)
- **Weapon Types**: Multiple weapons with unique properties
- **Mechanics**: Ammunition, reloading, firing rates, damage values
- **Visual Effects**: Muzzle flash, weapon sway, procedural animations
- **Hit Detection**: Ray-based hit scanning with damage falloff

### 5. Input System (`Sources/Game_InputHandling.hpp`)
- **Mouse Control**: Raw input for precise aiming, adjustable sensitivity
- **Keyboard**: WASD movement, action keys
- **Gamepad**: Full Xbox 360 controller support
- **Input Mapping**: Configurable key bindings

### 6. Audio System
- **Format Support**: MP3 playback for all audio
- **3D Audio**: Positional sound for entities
- **Sound Categories**: Weapons, footsteps, ambient music, UI
- **Dynamic Mixing**: Volume adjustment based on distance

### 7. User Interface (`Sources/Game_UI_*.hpp`)
- **HUD Elements**: Health bar, minimap, weapon display, ammo counter
- **Menu System**: Hierarchical navigation, settings screens
- **Performance**: FPS counter, debug information
- **Customization**: Configurable UI layout and colors

## Design Patterns & Architecture

### Software Architecture
1. **Modular Design**: Each system in separate header with clear interfaces
2. **Namespace Organization**: Logical grouping of related functionality
3. **Data-Driven**: Configuration files control game behavior
4. **Component-Based**: Entities composed of reusable components
5. **Resource Management**: Centralized loading and caching

### Performance Optimizations
1. **SIMD Usage**: SSE intrinsics for vector math operations
2. **Multi-threading**: Parallel rendering and update loops
3. **Spatial Optimization**: Frustum culling, distance-based LOD
4. **Memory Layout**: Cache-friendly data structures
5. **Fixed Timestep**: Consistent 60 FPS update rate

### Code Quality Features
- **Error Handling**: Comprehensive file and resource validation
- **Logging System**: Detailed debug output capabilities
- **Consistent Style**: Uniform naming conventions and formatting
- **Modern C++**: Uses C++17/20 features appropriately
- **RAII**: Resource management through constructors/destructors

## File Formats & Data Structures

### Game Data Formats
- **Maps**: 2D arrays in CSV format (e.g., `Level1.map`)
- **Configuration**: INI format for all settings
- **Graphics**: PNG for textures and sprites
- **Audio**: MP3 for sound effects and music
- **Fonts**: TrueType for text rendering

### Key Configuration Files
- `GameConfig.ini`: Master game settings
- `Assets_*.ini`: Entity and weapon definitions
- `Level*.ini`: Level-specific configurations

## Build & Development

### Build Requirements
- Visual Studio 2022 or later
- Windows SDK
- C++17/20 compiler support
- SSE 4.2 capable CPU

### Development Workflow
1. Modify configuration files for content changes
2. Edit source code for engine modifications
3. Build using Visual Studio solution
4. Test with included demo levels

## Purpose & Applications

NARC serves multiple purposes:
1. **Educational**: Learn game engine architecture and raycasting
2. **Practical**: Create custom retro-style FPS games
3. **Technical Demo**: Showcase modern C++ game development
4. **Research**: Experiment with rendering techniques

The engine successfully recreates the feel of early 90s shooters while using modern programming practices and optimizations, making it both nostalgic and technically impressive.

## Notable Technical Achievements

1. **Pure Software Rendering**: Despite using OpenGL, all pixel operations are software-based
2. **Scalable Performance**: Runs efficiently on modern hardware
3. **Extensible Design**: Easy to add new entities, weapons, levels
4. **Complete Feature Set**: Full game engine with all necessary systems
5. **Clean Architecture**: Well-organized, maintainable codebase

This analysis provides a comprehensive understanding of the NARC engine's architecture, implementation, and capabilities for future development or modification.