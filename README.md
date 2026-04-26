# House Rules

An F4SE plugin that exposes high-impact vanilla Fallout 4 settings through a single, well-documented MCM menu. Its goal is to give players the tools they need to tune vanilla mechanics so they fit a modded playthrough - not to add new gameplay.

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
| Allow Fast Travel | Re-enables Pip-Boy map fast travel eligibility; fast-travel load screens use targeted HP/rads/rad-resistance protection, not God Mode or Immortal Mode |
| Show Enemies on Compass | Restores vanilla red-dot enemy markers |
| Show Locations on Compass | Restores vanilla undiscovered-location markers |
| Remove Chem/Aid Weight | Strips Survival's added weight from consumables |
| Remove Ammo Weight | Strips Survival's added weight from ammunition |
| Remove Survival Carry-Weight Penalty | ESP-backed toggle that neutralizes `HC_ReduceCarryWeightAbility` on the player and companions |
| Allow God Mode | `tgm` / `tim` console cheats work in Survival via validated byte patches |
| Allow Re-entering Survival | Removes vanilla's one-way gate - switch Survival on and off freely |

`Allow Re-entering Survival` is live-toggleable; the others apply when the pause menu closes.

### Magnitude Scalers

Conservative sliders targeting proven vanilla records:

- Stimpak healing, limb repair, RadAway, Rad-X, food/drink healing, hunger/thirst/sleep penalty severity.

Magnitude edits touch base records, so changes take effect the next time an item is consumed or re-evaluated - the Pip-Boy tooltip value caches until then.

### Difficulty Multipliers

Runtime GMST sliders for each vanilla difficulty tier:

- **Difficulty I**: Incoming damage, outgoing damage, base XP rate, Intelligence XP bonus, legendary spawn chance, and legendary rarity.
- **Difficulty II**: Effect duration and effect magnitude per difficulty tier.

`1.00` means the engine's current baseline. House Rules snapshots each GMST on first safe apply and writes `baseline * slider`, so changes do not compound during a session.

Survival's Difficulty II baselines are unusual: vanilla `fDiffMultEffectDuration_TSV` is much *longer* than Normal and `fDiffMultEffectMagnitude_TSV` is much *smaller* than Normal. The MCM slider multiplies that Survival baseline, so `1.00` on Survival is still Survival - not Normal.

### Character (Action Points, Sprint, Carry, Health)

Runtime GMST sliders for AP feel, sprint cost, carry capacity, and health:

- **Action Points**: AP pool base, AP per Agility, AP regen rate, combat regen multiplier, regen delay, out-of-breath delay, regen delay cap.
- **Sprint**: drain base term, Endurance term, overall drain multiplier.
- **Carry Weight**: base carry capacity and capacity per Strength.
- **Health**: max health per Endurance, max health per level, passive heal rate, combat health regen multiplier.

Multiplier sliders show `1.00 = vanilla`. Direct sliders show the vanilla default as their neutral value (e.g. `0.75` combat AP regen, `-0.05` Endurance term, `0` heal rates); leaving a Direct slider on its default preserves whatever baseline another mod has written.

AP pool changes (`AP Pool Base`, `AP per Agility`), carry-weight formula changes, and max-health formula changes (`Health per Endurance`, `Health per Level`) edit the actor-value formula but the player's *current* derived value may not refresh until the engine recomputes actor values - typically on level-up, equip change, fast-travel, or save/load.

The `Remove Survival Carry-Weight Penalty` Survival Unlock stacks above the carry-weight GMST baseline: this slider tunes the underlying capacity formula, the unlock removes the Survival-only `HC_ReduceCarryWeightAbility` penalty on top of it.

If `Out-of-Breath Delay` exceeds `AP Regen Delay Max`, House Rules logs a one-shot warning; the engine will clamp to the cap rather than honoring the larger value.

### Damage Formulas

Runtime GMST sliders for the raw damage and armor-reduction terms shared by every actor:

- **Radiation**: `fRadsDamageFactor` (raw rad-damage factor) and `fRadsArmorDmgReductionExp` (rad resistance exponent). Survival-relevant: rads accumulate in Survival and these sliders are the cleanest way to dial that pressure up or down.
- **Physical**: `fPhysicalDamageFactor` and `fPhysicalArmorDmgReductionExp`. Broad advanced controls - they stack with the Difficulty I incoming/outgoing damage multipliers, so a `0.10` factor here combined with a Difficulty slider compounds.
- **Energy**: `fEnergyDamageFactor` and `fEnergyArmorDmgReductionExp`. Same shape as the Physical pair but for energy damage / energy resistance.

Each slider's neutral value is the vanilla default (`0.15` for damage factors, `0.365` for armor exponents). Leaving a slider on neutral preserves whatever baseline another mod has written; moving it off neutral writes the literal value to the GMST.

### Power Armor & Jetpack

Runtime GMST sliders for jetpack feel, fusion-core drain, and Power Armor durability:

- **Jetpack**: initial and sustained fuel drain (multiplier), initial and sustained thrust (multiplier), minimum fuel required to ignite (vanilla 3), and seconds before sustained mode (vanilla 0.15).
- **Fusion Core Drain**: drain per AP spent and per second running (multiplier), and direct per-jump / per-melee-attack / per-hard-landing drain entries (vanilla 0 for all three).
- **Durability**: player PA condition damage multiplier (1.0 vanilla, 0 = unbreakable) and NPC PA condition damage multiplier (vanilla 3).

Multiplier sliders show `1.00 = vanilla`. Direct sliders show the vanilla default as their neutral value; leaving a Direct slider on its default preserves whatever baseline another mod has written.

### Developer Diagnostics

`settings.ini` also exposes dev-only diagnostics that are not shown in MCM:

- `SurvivalObserver` for correlating survival actor values / active effects with vanilla behavior.
- `SleepWait` integration logging for sit/wait/sleep boundaries.
- `sLogLevel` controls plugin log verbosity: `Quiet`, `Normal`, `Verbose`, or `Trace`.
- `bValidationAudit=1` emits structured `HRVERIFY_SUMMARY` lines for GMST writes; set `sValidationAuditMode=Full` to include per-target `HRVERIFY` lines.

To validate an audit log after launching through F4SE and loading a save:

```powershell
python tools\validate_house_rules_log.py --require-module DifficultyEffects --require-module Character --require-module DamageFormulas --require-module PowerArmor
```

## Design Principles

- **Vanilla-first.** House Rules does not add new mechanics. It exposes existing ones.
- **Yield to conflicts.** Other mods that edit the same records should win; House Rules is a floor, not a ceiling.
- **Right tool per feature.** Some settings are pre-baked via an ESP (GMSTs, static record fields), others are runtime hooks in the DLL (byte patches, composable MCM sliders). Default is ESP; DLL is used when a setting must live-toggle, compose with others, or reach a target only accessible from native code.
- **Documented everything.** Each setting has a clear MCM description. The source of every vanilla value is cited where recovered from reverse-engineering work.
- **Minimum dependencies.** F4SE + Address Library + MCM. No script extenders beyond F4SE core, no hard-required companion mods.

## Installation

1. Download the latest zip from the [Releases page](https://github.com/northaxosky/HouseRules/releases).
2. Install through your mod manager (MO2, Vortex) or unzip into `<Fallout 4>/Data/`.
3. Configure in-game under Settings > Mod Configuration Menu > House Rules.

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

MIT - see [LICENSE](LICENSE).
