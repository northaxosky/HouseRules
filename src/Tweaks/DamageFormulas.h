#pragma once

// Damage Formulas v0.3 -- radiation, physical, and energy damage factor
// + armor reduction exponent GMSTs. Direct mode with the vanilla default
// as the neutral value; preserveBaselineAtNeutral keeps any other-mod
// baseline edits intact until the user moves a slider off neutral.
namespace Tweaks::DamageFormulas
{
	void Apply();
}
