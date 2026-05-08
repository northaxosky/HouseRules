# House Rules - Feature Reference

A page-by-page reference for every MCM setting House Rules ships. For installation, build, and general project info see the [README](../README.md).

## Reading the tables

- **Default** is the vanilla Fallout 4 value. Leaving a slider at its default keeps whatever baseline another mod has written.
- **Slider type:**
  - **Direct** writes the literal value to the underlying GMST or ActorValue.
  - **Multiplier** snapshots the vanilla baseline on first apply and writes `baseline * slider` thereafter, so changes don't compound across sessions.
  - **Toggle** flips a boolean flag (Survival Unlock toggles).
- **Apply timing:** all sliders apply when the pause menu closes. Magnitude edits also re-evaluate when an item is consumed; record-backed toggles (carry-weight unlock, Re-enter Survival) are live.

## Front Page

| Setting | Type | Default | Effect |
|---|---|---|---|
| Enable Plugin | Toggle | On | Master switch. With this off, none of the gameplay tweaks apply; diagnostics can still run. |
| Log Level | Stepper | Normal | Controls plugin log verbosity: `Quiet`, `Normal`, `Verbose`, `Trace`. |

## Survival Unlocks

All toggles default OFF (vanilla Survival behavior). Turning one ON applies that tweak while keeping real Survival difficulty engaged - i.e. food / water / sleep / damage scaling all stay in effect.

| Toggle | Effect |
|---|---|
| Allow Console | Re-enables the developer console. |
| Allow Manual Saves | Re-enables F5 quicksave plus the pause-menu Save / Load surfaces. |
| Allow Auto-Saves | Re-enables queued vanilla autosaves (menu close, door arrival, etc.). |
| Allow Fast Travel | Re-enables Pip-Boy map fast travel. Loading screens during fast travel get targeted HP / rad / rad-resistance protection - **not** God Mode or Immortal Mode, so non-fast-travel danger is unaffected. |
| Show Enemies on Compass | Restores vanilla red-dot enemy markers. |
| Show Locations on Compass | Restores vanilla undiscovered-location markers. |
| Remove Chem/Aid Weight | Strips Survival's added weight from consumables (chems, food, drinks, stimpaks). |
| Remove Ammo Weight | Strips Survival's added weight from ammunition. |
| Remove Survival Carry-Weight Penalty | ESP-backed: neutralizes `HC_ReduceCarryWeightAbility` on the player and companions. Stacks above the carry-weight GMST sliders on the Character page. |
| Allow God Mode | `tgm` and `tim` console cheats work in Survival via a single-byte engine patch (same approach as Unlimited Survival Mode). Vanilla `IsGodMode` returns honestly, so the engine's native invulnerability path is the one running. |
| Allow Re-entering Survival | Removes vanilla's one-way Survival lock - difficulty changes work both directions. Live-toggleable. |

## Magnitudes

Conservative scalers for proven vanilla survival records. Magnitude edits touch base records, so changes take effect the next time an item is consumed or re-evaluated. Pip-Boy tooltips cache the previous value until then.

### Consumables

| Slider | Type | Default | Effect |
|---|---|---|---|
| Stimpak Heal | Multiplier | 1.0 | Scales `RestoreHealthStimpak` magnitude (vanilla: 6). |
| Stimpak Limb Repair | Multiplier | 1.0 | Scales per-limb `RestoreCondition` magnitudes (vanilla: 100). |
| RadAway Strength | Multiplier | 1.0 | Scales `RestoreRadsChem` magnitude (vanilla: 60). |
| Rad-X Potency | Multiplier | 1.0 | Scales `FortifyResistRadsRadX` magnitude (vanilla: 100). |
| Food Heal | Multiplier | 1.0 | Scales the universal `RestoreHealthFood` MGEF baseCost. |

### Survival Stage Penalties

| Slider | Type | Default | Effect |
|---|---|---|---|
| Hunger Penalty | Multiplier | 1.0 | Scales `HC_HungerEffect_*` ALCH magnitudes (Peckish/Hungry/Famished/Starving). |
| Thirst Penalty | Multiplier | 1.0 | Scales `HC_ThirstEffect_*` ALCH magnitudes (Parched/Thirsty/Dehydrated). |
| Sleep Penalty | Multiplier | 1.0 | Scales `HC_SleepEffect_*` ALCH magnitudes (Tired/Weary/Exhausted/Incapacitated). |

`0.0` on any penalty slider nulls that effect entirely.

## Difficulty I (Combat & Rewards)

Runtime GMST sliders. `1.00` is the engine's current baseline; House Rules snapshots it on first apply and writes `baseline * slider`, so changes don't compound across sessions.

| Slider | GMST | Type | Effect |
|---|---|---|---|
| Incoming Damage | `fDiffMultHPByPCVS<difficulty>` | Multiplier | Damage you take per difficulty tier. |
| Outgoing Damage | `fDiffMultHPToPCVS<difficulty>` | Multiplier | Damage you deal per difficulty tier. |
| Base XP Rate | `fXPDifficultyMult` | Multiplier | Base XP scaling. |
| Intelligence XP Bonus | `fXPPerIntelligenceMult` | Multiplier | Bonus XP per Intelligence point. |
| Legendary Chance | `fLegendaryDropChance<tier>` | Multiplier | Probability a kill drops a legendary at this difficulty. |
| Legendary Rarity | `fLegendaryDropRarityMult<tier>` | Multiplier | Quality / rarity weight applied when a legendary does drop. |

## Difficulty II (Effect Duration & Magnitude)

Per-tier effect-balance multipliers. Survival's baselines are unusual: vanilla `fDiffMultEffectDuration_TSV` is much *longer* than Normal and `fDiffMultEffectMagnitude_TSV` is much *smaller* than Normal. The slider multiplies that Survival baseline, so `1.00` on Survival is still Survival - not Normal.

| Slider Group | GMST Family | Type | Notes |
|---|---|---|---|
| Effect Duration (per tier) | `fDiffMultEffectDuration_*` | Multiplier | Tiers: VeryEasy / Easy / Normal / Hard / VeryHard / Survival. The legacy `_SV` is intentionally skipped. |
| Effect Magnitude (per tier) | `fDiffMultEffectMagnitude_*` | Multiplier | Same tier set. |

## Character (Action Points, Sprint, Carry, Health)

Runtime GMST and player-ActorValue sliders for AP feel, sprint cost, carry capacity, and health.

### Action Points

| Slider | Surface | Default | Effect |
|---|---|---|---|
| AP Pool Base | GMST `fAVDActionPointsBase` | Direct | Base AP pool. |
| AP per Agility | GMST `fAVDActionPointsAgilityMult` | Direct | AP gained per Agility point. |
| AP Regen Rate | ActorValue `restoreAPRate` | Direct (player AV) | Out-of-combat AP regeneration. |
| Combat AP Regen Multiplier | GMST `fCombatActionPointsRegenMultiplier` | Direct | Multiplier applied to AP regen during combat (vanilla 0.75). |
| AP Regen Delay | GMST `fCombatActionPointsRegenDelay` | Direct | Seconds after AP spend before regen resumes. |
| Out-of-Breath Delay | GMST `fCombatActionPointsRegenDelayMax` | Direct | Cap on the regen delay. **Warning logged if you set this lower than `AP Regen Delay`.** |
| Regen Delay Cap | GMST `fSprintStopActionPointsRegenDelay` | Direct | Sprint-stop regen delay floor. |

### Sprint

| Slider | Surface | Default | Effect |
|---|---|---|---|
| Sprint Drain Base | GMST `fActorSprintAPCost` | Direct | Per-second AP cost while sprinting. |
| Sprint Endurance Term | GMST `fAVDActionPointsEnduranceMult` | Direct | Endurance reduction term (vanilla -0.05). |
| Sprint Drain Multiplier | GMST `fActorSprintAPCostMult` | Multiplier | Overall sprint drain scalar. |

### Carry Weight

| Slider | Surface | Default | Effect |
|---|---|---|---|
| Carry Weight Base | GMST `fAVDCarryWeightBase` | Direct | Base capacity before SPECIAL bonuses. |
| Carry Weight per Strength | GMST `fAVDCarryWeightStrengthMult` | Direct | Capacity gained per Strength point. |

The `Remove Survival Carry-Weight Penalty` Survival Unlock stacks on top of this: this slider tunes the underlying capacity formula; the unlock removes the Survival-only `HC_ReduceCarryWeightAbility` penalty.

### Health

| Slider | Surface | Default | Effect |
|---|---|---|---|
| Health per Endurance | GMST `fAVDHealthEnduranceMult` | Direct | Max-health gain per Endurance point. |
| Health per Level | GMST `fAVDHealthLevelMult` | Direct | Max-health gain per character level. |
| Passive Health Regen | ActorValue `restoreHealthRate` | Direct (player AV) | Out-of-combat health regen. |
| Combat Health Regen Multiplier | ActorValue `combatHealthRegenMult` | Direct (player AV) | Health regen scalar during combat. |

AP pool, carry, and max-health formula edits update the actor-value formula but the player's *current* derived value may not refresh until the engine recomputes (level-up, equip change, fast-travel, save/load).

The three ActorValue regen sliders (`AP Regen Rate`, `Passive Health Regen`, `Combat Health Regen`) write the player's live ActorValue bases because they aren't GMSTs. Unlike GMST Direct sliders, these use absolute vanilla-neutral values so saved player ActorValues always have a deterministic reset path.

## Damage Formulas

The raw damage and armor-reduction terms shared by every actor (player and NPCs).

| Slider | GMST | Default | Effect |
|---|---|---|---|
| Radiation Damage Factor | `fRadsDamageFactor` | 0.15 | Raw rad-damage factor. Survival-relevant: rads accumulate in Survival. |
| Radiation Armor Exponent | `fRadsArmorDmgReductionExp` | 0.365 | Rad-resistance exponent. |
| Physical Damage Factor | `fPhysicalDamageFactor` | 0.15 | Stacks with the Difficulty I incoming/outgoing multipliers - they compound. |
| Physical Armor Exponent | `fPhysicalArmorDmgReductionExp` | 0.365 | Physical resistance exponent. |
| Energy Damage Factor | `fEnergyDamageFactor` | 0.15 | Same shape as Physical, for energy damage. |
| Energy Armor Exponent | `fEnergyArmorDmgReductionExp` | 0.365 | Energy resistance exponent. |

All Direct. Leaving a slider on its default preserves whatever baseline another mod has written.

## Power Armor & Jetpack

Runtime GMSTs for jetpack feel, fusion-core drain, and Power Armor durability.

### Jetpack

| Slider | GMST | Default | Type |
|---|---|---|---|
| Initial Drain | `fJetpackDrainInital` (engine typo preserved) | 1.0 | Multiplier |
| Sustained Drain | `fJetpackDrainSustained` | 1.0 | Multiplier |
| Initial Thrust | `fJetpackThrustInitial` | 1.0 | Multiplier |
| Sustained Thrust | `fJetpackThrustSustained` | 1.0 | Multiplier |
| Min Fuel to Ignite | `fJetpackMinFuel` | 3 | Direct |
| Sustained-Mode Threshold | `fJetpackSustainedTime` | 0.15 sec | Direct |

### Fusion Core Drain

| Slider | GMST | Default | Type |
|---|---|---|---|
| Per AP Spent | `fFusionCoreDrainPerAP` | 1.0 | Multiplier |
| Per Second Running | `fFusionCoreDrainPerSec` | 1.0 | Multiplier |
| Per Jump | `fFusionCoreDrainJump` | 0 | Direct |
| Per Melee Attack | `fFusionCoreDrainMelee` | 0 | Direct |
| Per Hard Landing | `fFusionCoreDrainLanding` | 0 | Direct |

### Durability

| Slider | GMST | Default | Effect |
|---|---|---|---|
| Player PA Damage Multiplier | `fPowerArmorPlayerCondDmgMult` | 1.0 | `0` makes PA effectively unbreakable on the player. |
| NPC PA Damage Multiplier | `fPowerArmorNPCCondDmgMult` | 3 | Higher = NPC PA breaks faster. |

## Economy (Vendor Pricing)

| Slider | GMST | Default | Effect |
|---|---|---|---|
| Buy Price Floor | `fBarterMin` | 2.0 | Minimum buy-side multiplier. **Higher = vendors charge more.** |
| Sell Price Floor | `fBarterMax` | 3.5 | Sell-side divisor. **Higher = vendors pay less.** |
| Max Buy Multiplier | `fBarterBuyMax` | 1.2 | Cap the Barter perk / Charisma curve can reach when buying. |
| Max Sell Multiplier | `fBarterSellMax` | 0.8 | Cap reachable when selling. |

All Direct. Leaving a slider on its default preserves whatever baseline another mod has written.

## Progression (XP)

Non-difficulty XP sources. Difficulty I covers the global multipliers; this page covers crafting, lockpicking, and minor sources.

### Crafting XP

| Slider Group | GMST Triplet | Defaults |
|---|---|---|
| Cooking | `fCookingExp{Base,Max,Mult}` | 1 / 10 / 0.15 |
| Weapon/Armor Workbench | `fWorkbenchExperience{Base,Max,Mult}` | 2 / 50 / 0.03 |
| Settlement Workshop | `fWorkshopExperience{Base,Max,Mult}` | 2 / 25 / 0.10 |

Engine clamps computed XP between Base and Max. If you invert a pair (Base > Max), House Rules logs a one-shot warning rather than mutating the values.

### Lockpick XP

| Lock Tier | GMST | Default |
|---|---|---|
| Apprentice | `fLockpickXPRewardEasy` | 5 |
| Adept | `fLockpickXPRewardAverage` | 10 |
| Expert | `fLockpickXPRewardHard` | 15 |
| Master | `fLockpickXPRewardVeryHard` | 20 |

### Other

| Slider | GMST | Default |
|---|---|---|
| Mine Disarm XP | `iMineDisarmExperience` | 5 |

All Direct.

## VATS

| Slider | GMST | Default | Effect |
|---|---|---|---|
| Max Engage Distance | `fVATSMaxEngageDistance` | 5000 | Range where VATS can activate. `0` disables VATS targeting. |
| Target-Select Time | `fVATSTimeMultTargetSelect` | 0.04 | Lower values slow time more while choosing targets. |
| Player Damage Multiplier | `fVATSPlayerDamageMult` | 0.10 | Incoming damage multiplier while VATS is active. `1.0` removes the vanilla damage reduction. |

## Skills (Pickpocket, Hacking, Lockpicking)

### Pickpocket

| Slider | GMST | Default | Effect |
|---|---|---|---|
| Min Chance | `fPickPocketMinChance` | 0 | Floor on pickpocket success. |
| Max Chance | `fPickPocketMaxChance` | 90 | Ceiling on pickpocket success. |
| Reverse-Pickpocket Grenade Timer | `fProjectileInventoryGrenadeTimer` | 2 sec | Fuse on grenades reverse-pickpocketed onto NPCs. |

### Hacking

| Slider | GMST | Default | Effect |
|---|---|---|---|
| Min Words | `iHackingMinWords` | 6 | Smallest terminal puzzle size. |
| Max Words | `iHackingMaxWords` | 20 | Largest terminal puzzle size. |

### Lockpicking

Lock-tier sliders for break threshold, sweet-spot base, durability, and sweet-spot width. Lockpick XP remains on the Progression page.

## Sneak

### Sneak Attack Damage Multipliers

| Slider | GMST | Default |
|---|---|---|
| Gun | `fSneakAttackMult` | varies by weapon class |
| Unarmed / Fist | `fSneakAttackMultUnarmed` | varies |
| One-Handed Melee | `fSneakAttackMult1H` | varies |
| Two-Handed Melee | `fSneakAttackMult2H` | varies |

### Detection

| Slider | GMST | Default | Effect |
|---|---|---|---|
| Exterior Detection Distance | `fDetectionExteriorDistanceMult` | 1.0 | Multiplier on outdoor enemy detection range. |
| Light Sensitivity | `fDetectionLightMod` | 1.0 | How much ambient light affects detection. |
| Max Detection Distance | `fDetectionMaxDistance` | varies | Hard cap on enemy detection range. |

NPC search timers and stealth-point internals are intentionally deferred.

## Coming Soon

Tracks under active development:

- **Companions Affinity** - 9 sliders backed by direct vanilla TESGlobal writes (5 reaction thresholds + 4 cooldown durations). Replicates the Companions Affinity mod's functionality natively.
- **Settlements** - vanilla settlement GMSTs (workshop attack distance, recruitment caps, settler happiness rates, defense scaling). Does not replace Uncapped Settlement Surplus; covers the GMST-track adjacent.
- **Economy expansion** - additional vendor / persuasion / barter knobs beyond the four current barter sliders.
- **Progression expansion** - global XP curve (`fLevelUpExperienceFactor`), sneak XP scaling, perk pacing, kill XP scalers.

## Recommended Companion Mods

House Rules is designed to stack cleanly with mods that own the surfaces it doesn't:

- **[Uncapped Settlement Surplus](https://www.nexusmods.com/fallout4/mods/8635)** - removes hardcoded Papyrus settlement production caps. House Rules covers GMST settlement knobs; USS covers the script-side caps. Run both.
- **MCM** (required) - the configuration menu UI House Rules registers into.

## Validation

House Rules ships an in-game validation audit. Open the console and type `HRVERIFY` to emit a per-module PASS/FAIL/SKIP report into the plugin log. Every shipped GMST or ActorValue writer should print PASS with no failed or skipped targets.

To validate the audit log after a launch:

```powershell
python tools\validate_house_rules_log.py --require-module DifficultyEffects --require-module Character --require-module ActorValues --require-module DamageFormulas --require-module PowerArmor --require-module Economy --require-module Progression --require-module VATS --require-module Skills --require-module Sneak
```

Per-module validation will be extended for each new track (Companions, Settlements, Economy expansion, Progression expansion) as they ship.
