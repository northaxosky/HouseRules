#pragma once

// Installs hooks that selectively report a non-Survival DifficultyLevel to the
// game's gate functions for fast travel, console, quicksaves, autosaves, compass,
// etc. The approach mirrors BakaSurvivalSettings (hook `GetDifficultyLevel` at
// each restriction site and lie based on the corresponding MCM toggle), but the
// hook IDs, offsets, and wiring are to be implemented in a follow-up iteration.
namespace Hooks::Unlocks
{
	void Install();
}
