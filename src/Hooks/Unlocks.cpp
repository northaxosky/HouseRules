#include "PCH.h"

#include "Hooks/Unlocks.h"

#include "Settings.h"

namespace Hooks::Unlocks
{
	void Install()
	{
		// TODO: install per-restriction hooks on GetDifficultyLevel call sites.
		// Each restriction (fast travel, console, quicksave, autosave, self-save,
		// compass enemies, compass locations, alch weight, ammo weight, sleep-save,
		// god-mode, lock-survival) checks its MCM toggle and either returns the
		// true difficulty or a spoofed non-Survival value.
		spdlog::info("Hooks::Unlocks::Install (stub)");
	}
}
