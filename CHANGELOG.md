# Changelog

All notable changes to this project will be documented in this file.

## Unreleased

## 2.0.2 - 2026-03-27

### Fixed
- Fixed cutters so circle and diamond ores can be cut just like square ores.
- Fixed split diamond item rendering after cutting and in combined shape previews.
- Fixed miners placed directly against cutters so they can feed and split ores without an intermediate belt.

## 2.0.1 - 2026-03-27

### Added
- Remember the last rotation state for buildings during placement, matching the existing belt placement workflow.

### Changed
- Updated the GitHub release workflow for the 2026 Node.js 24 runtime transition on GitHub Actions runners.
- Upgraded core GitHub Actions steps in the release workflow to newer Node 24 compatible versions where available.

### Fixed
- Fixed cutter output positions when items enter from belts.
- Fixed cases where one blocked cutter could interrupt cutter updates for the whole map.
- Fixed cutter output item labels not being shown after new split items were created.
- Included the latest LICENSE and README licensing updates in the next patch release.

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
