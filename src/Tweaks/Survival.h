#pragma once

// Survival rule kill-switches: writes the four vanilla HC_Rule_* TESGlobals (sustenance, sleep deprivation, diseases, adrenaline) to disable each subsystem independently without leaving Survival difficulty.
namespace Tweaks::Survival
{
	void Apply();
}
