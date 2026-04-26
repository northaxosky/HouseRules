#include "PCH.h"

#include "Tweaks/PowerArmor.h"

#include "Settings.h"
#include "Tweaks/GameSettings.h"

#include <array>

namespace Tweaks::PowerArmor
{
	namespace
	{
		using GameSettings::FloatTarget;
		using GameSettings::Mode;

		// Note: Bethesda shipped `fJetpackDrainInital` (sic) - the typo is
		// the real GMST name and must be preserved. Its sibling
		// `fJetpackThrustInitial` is spelled correctly.
		const std::array<FloatTarget, 13> kTargets = { {
			// Jetpack
			{ "fJetpackDrainInital",                       &MCM::Settings::PowerArmor::fJetpackInitialDrainMult,    Mode::Multiplier, 1.0f  },
			{ "fJetpackDrainSustained",                    &MCM::Settings::PowerArmor::fJetpackSustainedDrainMult,  Mode::Multiplier, 1.0f  },
			{ "fJetpackMinFuelRequired",                   &MCM::Settings::PowerArmor::fJetpackMinFuelRequired,     Mode::Direct,     3.0f  },
			{ "fJetpackThrustInitial",                     &MCM::Settings::PowerArmor::fJetpackInitialThrustMult,   Mode::Multiplier, 1.0f  },
			{ "fJetpackThrustSustained",                   &MCM::Settings::PowerArmor::fJetpackSustainedThrustMult, Mode::Multiplier, 1.0f  },
			{ "fJetpackTimeToSustained",                   &MCM::Settings::PowerArmor::fJetpackTimeToSustained,     Mode::Direct,     0.15f },

			// Fusion core drain
			{ "fPowerArmorPowerDrainPerActionPoint",       &MCM::Settings::PowerArmor::fPAPowerDrainPerAPMult,      Mode::Multiplier, 1.0f  },
			{ "fPowerArmorPowerDrainPerSecondRunning",     &MCM::Settings::PowerArmor::fPAPowerDrainRunningMult,    Mode::Multiplier, 1.0f  },
			{ "fPowerArmorPowerDrainPerJump",              &MCM::Settings::PowerArmor::fPAPowerDrainPerJump,        Mode::Direct,     0.0f  },
			{ "fPowerArmorPowerDrainPerMeleeAttack",       &MCM::Settings::PowerArmor::fPAPowerDrainPerMeleeAttack, Mode::Direct,     0.0f  },
			{ "fPowerArmorPowerDrainPerImpactLand",        &MCM::Settings::PowerArmor::fPAPowerDrainPerImpactLand,  Mode::Direct,     0.0f  },

			// Durability
			{ "fPowerArmorPCArmorDamageMultiplier",        &MCM::Settings::PowerArmor::fPAPlayerArmorDamageMultiplier, Mode::Direct, 1.0f  },
			{ "fPowerArmorNPCArmorDamageMultiplier",       &MCM::Settings::PowerArmor::fPANPCArmorDamageMultiplier,    Mode::Direct, 3.0f  },
		} };
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		GameSettings::Apply("PowerArmor", std::span<const FloatTarget>{ kTargets });
	}
}
