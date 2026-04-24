#pragma once

// God-mode / immortal re-enablement in Survival. This still uses validated
// byte patches in PlayerCharacter::IsGodMode / IsImmortal so the console's
// ToggleGodModeFunction continues to observe vanilla state transitions.
namespace Hooks::GodMode
{
	void Install();
	void RefreshRuntimePatches();
}
