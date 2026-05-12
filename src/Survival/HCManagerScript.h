#pragma once

#include <cstdint>

// Typed writers for the Hardcore:HC_ManagerScript bound-object properties exposed by the HCManagerProbe dump. Looks up the HC_Manager quest (FormID 0x80E), finds the script's bound object on it, then mutates property storage directly via Object::GetProperty. No-op if the script isn't loaded (e.g. no Survival save).
namespace Survival::HCManagerScript
{
	bool SetFloat(const char* a_property, float a_value);
	bool SetInt  (const char* a_property, std::int32_t a_value);

	// Drop cached script-object pointer; called on per-save state-clear messages so a fresh save load resolves the new object.
	void Reset();
}
