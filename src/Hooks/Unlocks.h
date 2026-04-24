#pragma once

// Hooks each hard-coded Survival restriction (fast travel, console, saves, etc.)
// and replaces brittle re-enable-survival patching with runtime-anchored call hooks.
namespace Hooks::Unlocks
{
	void Install();
	void RefreshRuntimePatches();
}
