#include "PCH.h"

#include "Tweaks/ActionPoints.h"

#include "Settings.h"
#include "Tweaks/GameSettings.h"

#include <array>

namespace Tweaks::ActionPoints
{
	namespace
	{
		using GameSettings::FloatTarget;
		using GameSettings::Mode;

		// AP pool / regen GMSTs. Multiplier targets snapshot vanilla and
		// write baseline*user; Direct targets preserve the baseline while
		// the user value matches the vanilla default (neutral).
		const std::array<FloatTarget, 7> kAPTargets = { {
			{ "fAVDActionPointsBase",              &MCM::Settings::Character::fAPBaseMult,           Mode::Multiplier, 1.0f },
			{ "fAVDActionPointsMult",              &MCM::Settings::Character::fAPPerAgilityMult,     Mode::Multiplier, 1.0f },
			{ "ActionPointsRate",                  &MCM::Settings::Character::fAPRegenRateMult,      Mode::Multiplier, 1.0f },
			{ "fCombatActionPointsRegenRateMult", &MCM::Settings::Character::fCombatAPRegenMult,    Mode::Direct,     0.75f },
			{ "fDamagedAPRegenDelay",              &MCM::Settings::Character::fAPRegenDelay,         Mode::Direct,     1.0f },
			{ "fOutOfBreathActionPointsRegenDelay",&MCM::Settings::Character::fOutOfBreathAPRegenDelay, Mode::Direct,  2.0f },
			{ "fActionPointsRegenDelayMax",        &MCM::Settings::Character::fAPRegenDelayMax,      Mode::Direct,     5.0f },
		} };

		// Sprint formula: drain = (base + endurance_term*Endurance) * drainMult.
		const std::array<FloatTarget, 3> kSprintTargets = { {
			{ "fSprintActionPointsEndBase",  &MCM::Settings::Character::fSprintBaseDrain,      Mode::Direct,     1.05f },
			{ "fSprintActionPointsEndMult",  &MCM::Settings::Character::fSprintEnduranceDrain, Mode::Direct,    -0.05f },
			{ "fSprintActionPointsDrainMult",&MCM::Settings::Character::fSprintDrainMult,      Mode::Multiplier, 1.0f  },
		} };
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		GameSettings::Apply("Character", std::span<const FloatTarget>{ kAPTargets });
		GameSettings::Apply("Character", std::span<const FloatTarget>{ kSprintTargets });

		// Paired-setting sanity check: the post-out-of-breath regen delay
		// is itself clamped by fActionPointsRegenDelayMax. Warn (once) if
		// the user has configured an out-of-breath delay above the cap;
		// don't silently rewrite either knob.
		const float ooBreath = MCM::Settings::Character::fOutOfBreathAPRegenDelay.GetValue();
		const float delayMax = MCM::Settings::Character::fAPRegenDelayMax.GetValue();
		if (ooBreath > delayMax) {
			GameSettings::WarnOnce(
				"Character",
				"fOutOfBreathAPRegenDelay",
				"fOutOfBreathAPRegenDelay > fAPRegenDelayMax; engine will clamp to the cap");
		}
	}
}
