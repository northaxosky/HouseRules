#include "PCH.h"

#include "Tweaks/Survival.h"

#include "Globals/Globals.h"
#include "Settings.h"

#include <cstdint>

namespace Tweaks::Survival
{
	namespace
	{
		// FormIDs sourced from the HCManagerProbe dump (Hardcore:HC_ManagerScript bound-object surface). Both globals default to 1; writing 0 disables the corresponding rule subsystem.
		constexpr std::uint32_t kHCRuleSustenanceEffects = 0x00000854u;
		constexpr std::uint32_t kHCRuleSleepEffects      = 0x00000812u;
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		// MCM semantics: toggle ON = disabled (write 0), OFF = vanilla (write 1).
		const bool disable_sustenance = MCM::Settings::Survival::bDisableSustenance.GetValue();
		const bool disable_sleep      = MCM::Settings::Survival::bDisableSleepDeprivation.GetValue();

		Globals::WriteByFormID(kHCRuleSustenanceEffects, disable_sustenance ? 0.0f : 1.0f);
		Globals::WriteByFormID(kHCRuleSleepEffects,      disable_sleep      ? 0.0f : 1.0f);
	}
}
