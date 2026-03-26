# Shapez

A small classroom project inspired by [shapez.io](https://github.com/tobspr-games/shapez.io), implemented in C++ with Qt.

This project was created for an advanced programming course and focuses on building a simplified factory game prototype. Due to time and scope constraints, only a subset of the original game's systems is implemented.

## Overview

The game reproduces the core loop of placing structures and moving items on a grid-based map. The current version mainly includes:

- Belts
- Miners
- Cutters
- Trash bins
- Basic item transportation and interaction

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
- `globalupgradedialog.*`: upgrade dialog UI
- `res/` and `images/`: game assets and screenshots

## How to Build

### Requirements

Make sure the following tools are available on your machine:

- CMake 3.16 or newer
- A C++17-compatible compiler
- Qt 6 with `Widgets` and `Multimedia`

### Build Commands

```bash
cmake -S . -B build
cmake --build build
```

### Run

After building, launch the generated executable from the `build/` directory.

On Linux, it is typically:

```bash
./build/Shapez
```

You can also open the project directly in Qt Creator and build it there.

## Controls

- Mouse: most in-game interactions
- `R`: rotate the selected building
- `T`: switch belt style

## Screenshots

### Main Menu

![Main menu](./images/mainscene.png)

### Gameplay

![Gameplay 1](./images/game.png)
![Gameplay 2](./images/game2.png)

## Credits

This project is inspired by [shapez.io](https://github.com/tobspr-games/shapez.io).

Some image and game assets were sourced from the original project repository and are included here for learning purposes only.

## License Note

This repository currently contains third-party assets from another project. Because of that, you should not simply mark the entire repository as MIT unless those assets are removed or replaced.

In practice, you have two safer options:

1. Remove or replace all GPL-licensed assets, then license your own project under MIT.
2. Keep your own source code under MIT, but clearly state that the imported third-party assets remain under their original GPL-3.0 license.

If you choose the second option, the repository becomes a mixed-license project, and the asset attribution and license boundaries should be documented clearly.
