#include "PCH.h"

#include "Tweaks/Settlements.h"

#include "Settings.h"
#include "Tweaks/GameSettings.h"

#include <array>
#include <cstdint>

namespace Tweaks::Settlements
{
	namespace
	{
		using GameSettings::FloatTarget;
		using GameSettings::IntTarget;
		using GameSettings::Mode;

		const std::array<FloatTarget, 8> kFloatTargets = { {
			// Build timers / repair cost / wire length
			{ "fWorkshopBuildBaseTimer",     &MCM::Settings::Settlements::fBuildBaseTimer,     Mode::Direct, 0.1f    },
			{ "fWorkshopBuildResourceTimer", &MCM::Settings::Settlements::fBuildResourceTimer, Mode::Direct, 0.4f    },
			{ "fWorkshopRepairComponentMult",&MCM::Settings::Settlements::fRepairComponentMult,Mode::Direct, 0.1f    },
			{ "fWorkshopWireMaxLength",      &MCM::Settings::Settlements::fWireMaxLength,      Mode::Direct, 1100.0f },

			// Placement radius constraints
			{ "fWorkshopRestrictedBuildSpacing",        &MCM::Settings::Settlements::fRestrictedBuildSpacing,        Mode::Direct, 32.0f },
			{ "fWorkshopRestrictedBuildMaxFloraRadius", &MCM::Settings::Settlements::fRestrictedBuildMaxFloraRadius, Mode::Direct, 50.0f },
			{ "fWorkshopRestrictedBuildMaxKeywordRadius", &MCM::Settings::Settlements::fRestrictedBuildMaxKeywordRadius, Mode::Direct, 50.0f },
			{ "fWorkshopRestrictedBuildMaxTurretRadius",  &MCM::Settings::Settlements::fRestrictedBuildMaxTurretRadius,  Mode::Direct, 70.0f },
		} };

		const std::array<IntTarget, 1> kIntTargets = { {
			{ "iWorkshopSettlerPopulationMax", &MCM::Settings::Settlements::iSettlerPopulationMax, Mode::Direct, std::int32_t{ 10 } },
		} };
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		GameSettings::Apply("Settlements", std::span<const FloatTarget>{ kFloatTargets });
		GameSettings::Apply("Settlements", std::span<const IntTarget>{ kIntTargets });
	}
}
