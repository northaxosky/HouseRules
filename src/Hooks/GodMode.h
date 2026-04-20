#pragma once

// God-mode / immortal re-enablement in Survival. Replaces PC::IsGodMode and
// PC::IsImmortal wholesale (replace_func) and hooks two PC vtable slots so
// damage actually stops when the cheat is active under our toggle.
namespace Hooks::GodMode
{
	void Install();
	void RefreshRuntimePatches();
}
