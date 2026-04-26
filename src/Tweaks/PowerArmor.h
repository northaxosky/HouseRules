#pragma once

// Power Armor v0.3 - Jetpack, fusion-core drain, and PA durability GMSTs.
//
// Multiplier sliders snapshot the engine baseline once and write
// baseline * MCM. Direct sliders write verbatim, with neutral set to
// each vanilla default; preserveBaselineAtNeutral keeps any other mod's
// baseline edits until the user moves a slider off neutral.
namespace Tweaks::PowerArmor
{
	void Apply();
}
