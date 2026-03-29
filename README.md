# Shapez

A small classroom project inspired by [shapez.io](https://github.com/tobspr-games/shapez.io), implemented in C++ with Qt.

This project was created for an advanced programming course and focuses on building a simplified factory game prototype. It is now published here as a learning and portfolio project. Due to time and scope constraints, only a subset of the original game's systems is implemented.

## Overview

The game reproduces the core loop of placing structures and moving items on a grid-based map. The current version already includes a broader factory gameplay loop with:

- Belts with drag placement, automatic turns, and remembered placement rotation
- Miners for extracting square, circle, diamond, and dye resources
- Cutters for splitting shapes into valid halves
- Balancers for distributing throughput across two outputs
- Underground belts with automatic entry/exit detection and placement preview
- Rotators for turning shapes before later processing
- Stackers for combining compatible shapes into more complex outputs
- Mixers for additive RGB dye mixing into yellow, cyan, purple, and white
- Painters for applying dyes to produced shapes
- Trash bins for removing incorrect or excess products
- Red, green, and blue dye ore clusters that spawn on the map
- Level-based target shapes and progression goals, including colored late-game goals
- Per-level upgrade choices for belts, balancers, underground belts, miners, cutters, rotators, stackers, mixers, and painters
- Save/load support for map state, progression, and upgrade tiers
- Zooming, panning, responsive map rendering, blueprint placement previews, and resizable windows
- Simplified Chinese / English language switching from the main menu

This makes the project more than a static prototype: it already supports a small but complete production-chain loop with progression, logistics optimization, and multiple building interactions.

## Tech Stack

- C++17
- Qt Widgets
- Qt Multimedia
- CMake 3.16+

## Project Structure

- `main.cpp`: application entry point
- `mainscene.*`: main menu scene
- `gamescene.*`: gameplay scene and interactions
- `map.*`: map logic
- `tile.*`: tile definitions and behavior
- `item.*`: item data and processing
- `configmanager.*`: save/config state management
- `res/` and `images/`: game assets and screenshots

## How to Build

### Requirements

Make sure the following tools are available on your machine:

- CMake 3.16 or newer
- A C++17-compatible compiler
- Qt 6 with `Widgets` and `Multimedia`

### Build Commands

Single-config generators such as `Ninja` or `Unix Makefiles` (common on Linux and many macOS setups):

```bash
cmake -S . -B build
cmake --build build
```

Multi-config generators such as Visual Studio (common on Windows):

```powershell
cmake -S . -B build
cmake --build build --config Release
```

### Optional Debug Cheats Build

Debug-only cheats are compiled out by default. If you want the in-game debug helpers such as the next-level button / shortcut, configure the build with:

```bash
cmake -S . -B build-debug -DSHAPEZ_ENABLE_DEBUG_CHEATS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

On Windows / other multi-config generators, configure once and pick the `Debug` configuration when building:

```powershell
cmake -S . -B build-debug -DSHAPEZ_ENABLE_DEBUG_CHEATS=ON
cmake --build build-debug --config Debug
```

Without `-DSHAPEZ_ENABLE_DEBUG_CHEATS=ON`, those cheats are not included in the executable.

### Run

After building, launch the generated executable from the `build/` directory.

With single-config generators on Linux, it is typically:

```bash
./build/Shapez
```

With Visual Studio on Windows, it is typically:

```powershell
.\build\Release\Shapez.exe
```

If you built the optional debug-cheat variant with a multi-config generator, that path is usually:

```powershell
.\build-debug\Debug\Shapez.exe
```

If you launch the executable outside Qt Creator on Windows, make sure the required Qt runtime DLLs are available in your environment or deployed alongside the executable.

You can also open the project directly in Qt Creator and build it there.

## Controls

- Mouse: most in-game interactions
- `R`: rotate the selected building
- `T`: switch belt style
- `Esc`: cancel the current placement
- `1`-`0`: quick-select toolbar buildings
- `F10`: instantly complete the current level and open the upgrade choice when built with `SHAPEZ_ENABLE_DEBUG_CHEATS=ON`

## Screenshots

### Main Menu

![Main menu](./images/mainscene.png)

### Gameplay

![Gameplay 1](./images/game.png)
![Gameplay 2](./images/game2.png)

## Credits

This project is inspired by [shapez.io](https://github.com/tobspr-games/shapez.io).

Some image and game assets were adapted from or inspired by the original project repository. Please refer to the license below before reusing repository contents.

## License

This repository is distributed under the GNU General Public License v3.0. See the [LICENSE](./LICENSE) file for details.
