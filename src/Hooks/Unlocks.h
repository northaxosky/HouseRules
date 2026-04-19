#pragma once

// Hooks each hard-coded Survival restriction (fast travel, console, saves, etc.)
// and spoofs DifficultyLevel based on the matching MCM toggle.
namespace Hooks::Unlocks
{
	void Install();
	void RefreshRuntimePatches();  // re-apply byte-patch toggles (MCM reload)
}
