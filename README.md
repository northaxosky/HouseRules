# Survival Architect

An F4SE plugin that curates Fallout 4's Survival mode through a single MCM menu. Selectively unlock vanilla Survival restrictions — console, fast travel, saves, compass markers, consumable/ammo weights — without leaving Survival difficulty.

## Compatibility

All three main Fallout 4 runtimes:

- OG 1.10.163 (pre-NG)
- NG 1.10.984
- AE 1.11.x

## Features

All toggles default OFF (vanilla Survival). Turning one ON applies that tweak while keeping Survival difficulty engaged.

| Toggle | Effect |
|---|---|
| Allow Console | Re-enables the developer console |
| Allow Manual Saves | Re-enables F5 quicksave and the Save/Load menu entries |
| Allow Auto-Saves | Re-enables vanilla autosaves (menu-close, pause-menu, door arrivals) |
| Allow Fast Travel | Re-enables fast travel from the Pipboy map |
| Show Enemies on Compass | Restores vanilla red-dot enemy markers |
| Show Locations on Compass | Restores vanilla undiscovered-location markers |
| Remove Chem/Aid Weight | Strips Survival's added weight from consumables |
| Remove Ammo Weight | Strips Survival's added weight from ammunition |
| Allow God Mode | `tgm` / `tim` console cheats work in Survival |
| Allow Re-entering Survival | Removes vanilla's one-way gate — switch Survival on and off freely |

`Allow Re-entering Survival` is live-toggleable; the others apply when the pause menu closes.

## Installation

1. Download the latest zip from the [Releases page](https://github.com/northaxosky/SurvivalArchitect/releases).
2. Install through your mod manager (MO2, Vortex) or unzip into `<Fallout 4>/Data/`.
3. Configure in-game under Settings → Mod Configuration Menu → Survival Architect.

Requires F4SE, Address Library for F4SE, and Mod Configuration Menu (MCM).

## Build

Requires [xmake](https://xmake.io) 3.0+ on Windows with MSVC. CommonLibF4 is a git submodule.

```sh
git clone --recursive https://github.com/northaxosky/SurvivalArchitect.git
cd SurvivalArchitect
xmake config -m releasedbg
xmake build
```

Set `COMMONLIBF4_PATH` to reuse an existing CommonLibF4 checkout instead of initialising the submodule.

## License

MIT — see [LICENSE](LICENSE).
