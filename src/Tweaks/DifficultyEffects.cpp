#include "PCH.h"

#include "Tweaks/DifficultyEffects.h"

#include "Settings.h"
#include "Tweaks/GameSettings.h"

#include <array>

namespace Tweaks::DifficultyEffects
{
	namespace
	{
		using GameSettings::FloatTarget;
		using GameSettings::Mode;

		// Effect duration/magnitude sliders. Multiplier mode against vanilla baseline.
		const std::array<FloatTarget, 12> kTargets = { {
			{ "fDiffMultEffectDuration_VE",  &MCM::Settings::Difficulty::fEffectDurationVeryEasy, Mode::Multiplier, 1.0f },
			{ "fDiffMultEffectDuration_E",   &MCM::Settings::Difficulty::fEffectDurationEasy,     Mode::Multiplier, 1.0f },
			{ "fDiffMultEffectDuration_N",   &MCM::Settings::Difficulty::fEffectDurationNormal,   Mode::Multiplier, 1.0f },
			{ "fDiffMultEffectDuration_H",   &MCM::Settings::Difficulty::fEffectDurationHard,     Mode::Multiplier, 1.0f },
			{ "fDiffMultEffectDuration_VH",  &MCM::Settings::Difficulty::fEffectDurationVeryHard, Mode::Multiplier, 1.0f },
			{ "fDiffMultEffectDuration_TSV", &MCM::Settings::Difficulty::fEffectDurationSurvival, Mode::Multiplier, 1.0f },

			{ "fDiffMultEffectMagnitude_VE",  &MCM::Settings::Difficulty::fEffectMagnitudeVeryEasy, Mode::Multiplier, 1.0f },
			{ "fDiffMultEffectMagnitude_E",   &MCM::Settings::Difficulty::fEffectMagnitudeEasy,     Mode::Multiplier, 1.0f },
			{ "fDiffMultEffectMagnitude_N",   &MCM::Settings::Difficulty::fEffectMagnitudeNormal,   Mode::Multiplier, 1.0f },
			{ "fDiffMultEffectMagnitude_H",   &MCM::Settings::Difficulty::fEffectMagnitudeHard,     Mode::Multiplier, 1.0f },
			{ "fDiffMultEffectMagnitude_VH",  &MCM::Settings::Difficulty::fEffectMagnitudeVeryHard, Mode::Multiplier, 1.0f },
			{ "fDiffMultEffectMagnitude_TSV", &MCM::Settings::Difficulty::fEffectMagnitudeSurvival, Mode::Multiplier, 1.0f },
		} };
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}
		GameSettings::Apply("DifficultyEffects", std::span<const FloatTarget>{ kTargets });
	}
}
