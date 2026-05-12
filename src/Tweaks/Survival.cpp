#include "PCH.h"

#include "Tweaks/Survival.h"

#include "Globals/Globals.h"
#include "Settings.h"

#include <cstdint>

namespace Tweaks::Survival
{
	namespace
	{
		// FormIDs sourced from the HCManagerProbe dump (Hardcore:HC_ManagerScript bound-object surface). All four globals default to 1; writing 0 disables the corresponding rule subsystem.
		constexpr std::uint32_t kHCRuleSustenanceEffects = 0x00000854u;
		constexpr std::uint32_t kHCRuleSleepEffects      = 0x00000812u;
		constexpr std::uint32_t kHCRuleDiseaseEffects    = 0x0000088Au;
		constexpr std::uint32_t kHCRuleAdrenalineOn      = 0x00000810u;
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		// MCM semantics: toggle ON = disabled (write 0), OFF = vanilla (write 1).
		const bool disable_sustenance = MCM::Settings::Survival::bDisableSustenance.GetValue();
		const bool disable_sleep      = MCM::Settings::Survival::bDisableSleepDeprivation.GetValue();
		const bool disable_disease    = MCM::Settings::Survival::bDisableDiseases.GetValue();
		const bool disable_adrenaline = MCM::Settings::Survival::bDisableAdrenaline.GetValue();

		Globals::WriteByFormID(kHCRuleSustenanceEffects, disable_sustenance ? 0.0f : 1.0f);
		Globals::WriteByFormID(kHCRuleSleepEffects,      disable_sleep      ? 0.0f : 1.0f);
		Globals::WriteByFormID(kHCRuleDiseaseEffects,    disable_disease    ? 0.0f : 1.0f);
		Globals::WriteByFormID(kHCRuleAdrenalineOn,      disable_adrenaline ? 0.0f : 1.0f);
	}
}
