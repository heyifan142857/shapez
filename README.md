# Shapez

A small classroom project inspired by [shapez.io](https://github.com/tobspr-games/shapez.io), implemented in C++ with Qt.

This project was created for an advanced programming course and focuses on building a simplified factory game prototype. It is now published here as a learning and portfolio project. Due to time and scope constraints, only a subset of the original game's systems is implemented.

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

Some image and game assets were adapted from or inspired by the original project repository. Please refer to the license below before reusing repository contents.

## License

This repository is distributed under the GNU General Public License v3.0. See the [LICENSE](./LICENSE) file for details.
