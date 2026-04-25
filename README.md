# House Rules

An F4SE plugin that exposes high-impact vanilla Fallout 4 settings through a single, well-documented MCM menu. Its goal is to give players the tools they need to tune vanilla mechanics so they fit a modded playthrough — not to add new gameplay.

> "Their game, their rules."

## Compatibility

All three main Fallout 4 runtimes:

- OG 1.10.163 (pre-NG)
- NG 1.10.984
- AE 1.11.x

Requires **F4SE** and **Address Library for F4SE**. Mod Configuration Menu (MCM) is required for the settings UI.

## Status

House Rules is under active development. The plugin started as a Survival-focused curation tool and is expanding into a broader vanilla-tweak utility. Below are the features that ship today; the scope charter in this repo will document what's planned next.

## Currently Shipping

### Survival Unlocks

All toggles default OFF (vanilla behavior). Turning one ON applies that tweak while keeping real Survival difficulty engaged.

| Toggle | Effect |
|---|---|
| Allow Console | Re-enables the developer console |
| Allow Manual Saves | Re-enables F5 quicksave plus the pause-menu Save/Load surfaces |
| Allow Auto-Saves | Re-enables queued vanilla autosaves such as menu-close and door-arrival saves |
| Allow Fast Travel | Re-enables Pip-Boy map fast travel eligibility |
| Show Enemies on Compass | Restores vanilla red-dot enemy markers |
| Show Locations on Compass | Restores vanilla undiscovered-location markers |
| Remove Chem/Aid Weight | Strips Survival's added weight from consumables |
| Remove Ammo Weight | Strips Survival's added weight from ammunition |
| Remove Survival Carry-Weight Penalty | Neutralizes `HC_ReduceCarryWeightAbility` on the player and companions |
| Allow God Mode | `tgm` / `tim` console cheats work in Survival via validated byte patches |
| Allow Re-entering Survival | Removes vanilla's one-way gate — switch Survival on and off freely |

`Allow Re-entering Survival` is live-toggleable; the others apply when the pause menu closes.

### Magnitude Scalers

Conservative sliders targeting proven vanilla records:

- Stimpak healing, limb repair, RadAway, Rad-X, food/drink healing, hunger/thirst/sleep penalty severity.

Magnitude edits touch base records, so changes take effect the next time an item is consumed or re-evaluated — the Pip-Boy tooltip value caches until then.

### Developer Diagnostics

`settings.ini` also exposes dev-only diagnostics that are not shown in MCM:

- `SurvivalObserver` for correlating survival actor values / active effects with vanilla behavior.
- `SleepWait` integration logging for sit/wait/sleep boundaries.

## Design Principles

- **Vanilla-first.** House Rules does not add new mechanics. It exposes existing ones.
- **Yield to conflicts.** Other mods that edit the same records should win; House Rules is a floor, not a ceiling.
- **Right tool per feature.** Some settings are pre-baked via an ESP (GMSTs, static record fields), others are runtime hooks in the DLL (byte patches, composable MCM sliders). Default is ESP; DLL is used when a setting must live-toggle, compose with others, or reach a target only accessible from native code.
- **Documented everything.** Each setting has a clear MCM description. The source of every vanilla value is cited where recovered from reverse-engineering work.
- **Minimum dependencies.** F4SE + Address Library + MCM. No script extenders beyond F4SE core, no hard-required companion mods.

## Installation

1. Download the latest zip from the [Releases page](https://github.com/northaxosky/HouseRules/releases).
2. Install through your mod manager (MO2, Vortex) or unzip into `<Fallout 4>/Data/`.
3. Configure in-game under Settings → Mod Configuration Menu → House Rules.

## Build

Requires [xmake](https://xmake.io) 2.9.4+ on Windows with MSVC. CommonLibF4 is a git submodule.

```sh
git clone --recursive https://github.com/northaxosky/HouseRules.git
cd HouseRules
xmake config -m releasedbg
xmake build
```

Set `COMMONLIBF4_PATH` to reuse an existing CommonLibF4 checkout instead of initialising the submodule.

## License

MIT — see [LICENSE](LICENSE).
