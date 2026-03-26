# Changelog

All notable changes to this project will be documented in this file.

## Unreleased

### Added
- English GitHub-style `README.md` with project overview, controls, and build instructions.
- `CHANGELOG.md` for tracking repository updates.
- Language switching support between Simplified Chinese and English from the main menu.
- Blueprint placement preview that follows the cursor and updates with building rotation/state changes.
- Smart belt drag placement that follows the mouse path and automatically creates turns.
- Zooming, panning, and responsive map rendering while keeping tiles square.

### Changed
- Expanded the map size and improved camera behavior for larger layouts.
- Reworked resource spawning to create two randomized clusters per resource type in outer map regions.
- Resource cluster size is now randomized between 4 and 15 tiles.
- Building placement flow now keeps the selected blueprint active until placement or cancellation.
- Save/load logic now preserves more game state, including miner resource data and upgrade timing.
- Default language is now Simplified Chinese with persistent language preference.

### Fixed
- Removed the stray blue icon that appeared after entering the game.
- Fixed toolbar and HUD alignment issues after adding map zoom/pan support.
- Fixed blueprint previews and selected building icons moving incorrectly with the map.
- Fixed miner placement so ore is preserved and still rendered beneath the miner.
- Fixed duplicated multi-tile save data by serializing only root tiles.
- Fixed Hub/base rendering after loading a save.
- Standardized runtime debug and warning logs to English.
