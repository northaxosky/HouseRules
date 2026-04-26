#pragma once

// Character v0.3 - Carry Weight + Health.
//
// GMST sliders for actor-value-derived carry capacity and max-health
// scaling. Formula edits feed into derived actor values and may not refresh
// until the engine reroll (level-up, equip change, fast-travel, save/load).
namespace Tweaks::CharacterStats
{
	void Apply();
}
