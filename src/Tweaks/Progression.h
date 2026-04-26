#pragma once

// Progression v0.4 - Non-difficulty XP source GMSTs: cooking, weapon/armor
// workbench, settlement workshop, lockpick rewards, and mine disarm.
//
// All Direct sliders with vanilla defaults as neutral. Engine clamps
// computed crafting XP between Base and Max; Apply emits a one-shot
// warning if the user inverts a Base/Max pair but does not mutate values.
namespace Tweaks::Progression
{
	void Apply();
}
