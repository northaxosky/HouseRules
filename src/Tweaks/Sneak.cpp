#include "PCH.h"

#include "Tweaks/Sneak.h"

#include "Settings.h"
#include "Tweaks/GameSettings.h"

#include <array>

namespace Tweaks::Sneak
{
	namespace
	{
		using GameSettings::FloatTarget;
		using GameSettings::Mode;

		const std::array<FloatTarget, 7> kTargets = { {
			// Sneak attack damage
			{ "fCombatSneakGunMult",      &MCM::Settings::Sneak::fSneakAttackGunMult,      Mode::Direct, 2.0f    },
			{ "fCombatSneakHandMult",     &MCM::Settings::Sneak::fSneakAttackUnarmedMult,  Mode::Direct, 3.0f    },
			{ "fCombatSneak1HSwordMult",  &MCM::Settings::Sneak::fSneakAttackOneHandMult,  Mode::Direct, 3.0f    },
			{ "fCombatSneak2HSwordMult",  &MCM::Settings::Sneak::fSneakAttackTwoHandMult,  Mode::Direct, 3.0f    },

			// Detection
			{ "fSneakExteriorDistanceMult", &MCM::Settings::Sneak::fSneakExteriorDistanceMult, Mode::Direct, 1.25f   },
			{ "fSneakLightMult",            &MCM::Settings::Sneak::fSneakLightMult,            Mode::Direct, 0.33f   },
			{ "fSneakMaxDistance",          &MCM::Settings::Sneak::fSneakMaxDistance,          Mode::Direct, 4096.0f },
		} };
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		GameSettings::Apply("Sneak", std::span<const FloatTarget>{ kTargets });
	}
}
