#pragma once

// Character v0.3 - Action Points / Sprint.
//
// AP pool/regen and sprint-drain GMST sliders. Uses the shared
// Tweaks::GameSettings helper. Pool-shape settings (fAVDActionPointsBase,
// fAVDActionPointsMult) only affect newly recomputed actor values; the
// player's current AP pool may not refresh until the engine reroll
// (level-up, equip change, fast-travel, save/load).
namespace Tweaks::ActionPoints
{
	void Apply();
}
