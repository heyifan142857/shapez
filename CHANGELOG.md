# Changelog

All notable changes to this project will be documented in this file.

## Unreleased

### Added
- Added dye resource spawning for red, green, and blue dye ore clusters, with miners now able to extract dye tiles just like normal ore.
- Added working `Mixer` and `Painter` buildings so factories can combine dyes with additive RGB color mixing and apply paint to produced shapes.
- Added colored level goals in later stages, including secondary-color goals that require the dye production chain.
- Added per-item color state and save/load support for dyed shapes, dye items, and dye-backed miners.
- Added an optional build-time cheat toggle via `-DSHAPEZ_ENABLE_DEBUG_CHEATS=ON`, exposing the debug-only next-level shortcut and button only in those builds.
- Added upgrade tiers for `Mixer` and `Painter`, including save/load persistence and separate processing speeds in-game.
- Added legacy save migration for the old dye system, remapping historical yellow/orange/green save data onto the new additive RGB palette when loading older saves.
- Added persistent saved upgrade points, allowing level rewards to be skipped and spent later from either the level-complete upgrade dialog or the upgrade overview panel.

### Changed
- Updated hub validation so deliveries now require both matching shape parts and matching colors before they count toward level progress.
- Reworked dye progression to use additive RGB mixing, so red/green/blue now produce yellow/cyan/purple and full RGB combinations produce white.
- Updated mixer and painter tooltips/building descriptions to reflect their real behavior.
- Reworked the level-complete upgrade dialog into a card-based layout with building icons, persistent progress bars, descriptions, and hoverable `➕` upgrade buttons.
- Updated both upgrade panels so they show available/saved upgrade points, allow multiple queued spends before closing, and keep progress visible even at max tier.
- Reworked the main menu and gameplay windows to start within the current screen size and remain resizable on smaller displays.
- Updated the main menu layout and artwork placement to scale with the window instead of assuming a fixed `1600x900` frame.
- Improved dragged belt placement so paths can bend toward valid nearby connections even when the straight path is blocked, and preserved manual `T` belt variants during placement.
- Expanded `README.md` build/run instructions to cover Windows Visual Studio style builds in addition to single-config desktop setups.

### Fixed
- Fixed dye ore rendering so color tiles use their actual icon size instead of collapsing to zero-sized sprites.
- Adjusted dye resource generation to prefer spawning attached to existing ore fields, making them easier to find during gameplay.
- Fixed painter input direction handling so the default 1x2 layout correctly accepts shapes from the left and dye from above the right-hand cell.
- Fixed dye item rendering order so painted items stay above belts instead of occasionally drawing underneath them.
- Fixed missing fallback rendering for colors without a dedicated sprite asset so mixed outputs such as white remain visible.
- Fixed upgrade point persistence so skipping an upgrade no longer discards the earned point and saved points survive save/load cycles.

## 2.1.0 - 2026-03-27

### Added
- Added level-based production goals that generate target shapes for each stage and require delivering matching items to advance.
- Added a per-level upgrade selection flow with up to five tiers each for belts, balancers, underground belts, miners, cutters, rotators, and stackers.
- Added dedicated update loops for balancers, underground belts, rotators, and stackers so these buildings now process items automatically during gameplay.
- Added an in-game upgrade overview panel showing current tiers and effective throughput values for each upgradable building type.
- Added tooltip tier labels and speed information for major logistics buildings to make current production performance visible in the UI.
- Added underground belt connection preview rendering and auto entry/exit detection during placement.
- Added support for stackers to buffer a secondary input item before combining shapes.

### Changed
- Reworked level completion handling so all simulation timers pause during upgrade selection and resume with updated speeds afterward.
- Replaced the old persistent boolean upgrade system with numeric tier-based progression stored in save data, while keeping compatibility with older save fields.
- Rebalanced default processing intervals so belts remain the fastest logistics layer and other buildings scale consistently with their upgrade tiers.
- Updated hub delivery logic to score produced items by shape parts while only counting deliveries toward progress when they exactly match the current goal.
- Expanded building insertion and routing rules so belts, balancers, cutters, underground belts, rotators, stackers, trash, and the hub share a common item handoff path.
- Removed the standalone global upgrade dialog source files and folded upgrade flow/overview UI into `Gamescene`.
- Updated `README.md` and the build target source list to reflect the removal of the old global upgrade dialog module.

### Fixed
- Fixed cutter, balancer, rotator, stacker, and underground belt outputs so they now respect destination availability and no longer drop items through inconsistent placement checks.
- Fixed wide-building input and output routing by distinguishing root, primary, alternate, and secondary cells for multi-tile buildings.
- Fixed underground belts so entries transfer items across up to four tiles into matching exits and exits forward buffered items normally.
- Fixed stacked or buffered item rendering so items stay visually inside buildings instead of floating above building sprites.
- Fixed item and tile copying/destruction for buildings with buffered secondary items to avoid leaks and stale pointers.

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
