#include "PCH.h"

#include "Tweaks/DamageFormulas.h"

#include "Settings.h"
#include "Tweaks/GameSettings.h"

#include <array>

namespace Tweaks::DamageFormulas
{
	namespace
	{
		using GameSettings::FloatTarget;
		using GameSettings::Mode;

		// Direct mode with vanilla defaults as neutral. With
		// preserveBaselineAtNeutral=true (default), the helper restores the
		// snapshotted engine baseline whenever the slider sits on neutral,
		// so other mods' edits aren't clobbered until the user opts in.
		const std::array<FloatTarget, 6> kTargets = { {
			{ "fRadsDamageFactor",            &MCM::Settings::DamageFormulas::fRadiationDamageFactor,         Mode::Direct, 0.15f  },
			{ "fRadsArmorDmgReductionExp",    &MCM::Settings::DamageFormulas::fRadiationArmorReductionExponent, Mode::Direct, 0.365f },
			{ "fPhysicalDamageFactor",        &MCM::Settings::DamageFormulas::fPhysicalDamageFactor,          Mode::Direct, 0.15f  },
			{ "fPhysicalArmorDmgReductionExp",&MCM::Settings::DamageFormulas::fPhysicalArmorReductionExponent, Mode::Direct, 0.365f },
			{ "fEnergyDamageFactor",          &MCM::Settings::DamageFormulas::fEnergyDamageFactor,            Mode::Direct, 0.15f  },
			{ "fEnergyArmorDmgReductionExp",  &MCM::Settings::DamageFormulas::fEnergyArmorReductionExponent,  Mode::Direct, 0.365f },
		} };
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		GameSettings::Apply("DamageFormulas", std::span<const FloatTarget>{ kTargets });
	}
}
