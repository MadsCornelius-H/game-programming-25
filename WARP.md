# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview
This is the IT University of Copenhagen Game Programming course repository. It's a CMake-based C/C++ project using SDL3 for graphics and window management. The project follows C-style programming with minimal C++ features.

## Build System & Common Commands

### Initial Setup
```bash
# Clone with submodules (required for SDL3)
git clone --recurse-submodules https://github.com/Chris-Carvelli/game-programming-25.git

# Configure CMake (from project root)
mkdir build && cd build
cmake ..

# Build all targets
cmake --build .
```

### Building Specific Targets
```bash
# Build individual examples
cmake --build build --target 01_hello_world.cpp
cmake --build build --target 02_hello_sdl.cpp

# Build specific exercise (replace with actual filename)
cmake --build build --target E00_introduction.cpp

# Build playground files (any .cpp files added to playground/)
cmake --build build --target your_file.cpp
```

### Running Executables
All executables are built to the `build/` directory:
```bash
# Run examples
./build/01_hello_world.cpp
./build/02_hello_sdl.cpp

# Run exercises
./build/E00_introduction.cpp
```

### Development Workflow
```bash
# After adding new .cpp files, reconfigure CMake
cmake --build build

# Or force reconfigure if needed
cd build && cmake ..
```

## Architecture & Structure

### Directory Layout
- `examples/` - Reference implementations and learning examples
- `exercises/` - Course assignments and practice problems  
- `playground/` - Sandbox for testing and experimentation
- `lib/SDL/` - SDL3 library as git submodule
- `media/` - Assets and resources
- `build/` - CMake build output directory

### Key Components

#### CMake Configuration
- Root `CMakeLists.txt` defines the main project and subdirectories
- Each folder has its own `CMakeLists.txt` that auto-discovers `.cpp` files
- SDL3 is linked as a submodule and integrated into the build system
- All executables output to the build root for easy execution

#### SDL3 Integration
- All graphics programs use SDL3 for window management and rendering
- Common pattern: Create window → Create renderer → Game loop with event polling
- SDL3 headers are available via `#include <SDL3/SDL.h>`
- Debug text rendering available via `SDL_RenderDebugText()`

#### Code Patterns
- Entry point is always `int main(void)`
- C-style programming with minimal C++ features (mostly `std::cout` for simple output)
- Event-driven programming using SDL event polling
- Frame timing and rate control using SDL timing functions
- Color values typically in hexadecimal format (e.g., `0xFF, 0x00, 0x42`)

### Target Audience
Course is designed for students learning game programming fundamentals, assuming basic C/C++ knowledge but expecting reinforcement of:
- Pointers and memory management
- Control flow and event handling
- Linear algebra and trigonometry applications
- Real-time programming concepts

### Development Environment
- VSCode with C/C++ Extension Pack and CMake Tools recommended
- CMake 3.10+ required
- Cross-platform support (Windows/Mac/Linux)
- Build artifacts isolated to `build/` directory
