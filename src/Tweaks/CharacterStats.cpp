#include "PCH.h"

#include "Tweaks/CharacterStats.h"

#include "Settings.h"
#include "Tweaks/GameSettings.h"

#include <array>

namespace Tweaks::CharacterStats
{
	namespace
	{
		using GameSettings::FloatTarget;
		using GameSettings::Mode;

		// Carry weight + max-health formula GMSTs. Multiplier mode scales
		// the engine baseline; derived actor values may need a refresh
		// (level-up, equip change, fast-travel, save/load) to pick up the
		// change for the existing player.
		const std::array<FloatTarget, 4> kFormulaTargets = { {
			{ "fAVDCarryWeightBase",      &MCM::Settings::Character::fCarryWeightBaseMult,        Mode::Multiplier, 1.0f },
			{ "fAVDCarryWeightMult",      &MCM::Settings::Character::fCarryWeightPerStrengthMult, Mode::Multiplier, 1.0f },
			{ "fAVDHealthStartEndMult",   &MCM::Settings::Character::fHealthPerEnduranceMult,     Mode::Multiplier, 1.0f },
			{ "fHealthEnduranceOffset",   &MCM::Settings::Character::fHealthPerLevelMult,         Mode::Multiplier, 1.0f },
		} };

		// Health regen GMSTs. Vanilla is 0 for both; Direct mode with
		// preserveBaselineAtNeutral keeps any existing baseline edits from
		// other mods until the user moves the slider off 0.
		const std::array<FloatTarget, 2> kRegenTargets = { {
			{ "fHealrate",              &MCM::Settings::Character::fHealRate,              Mode::Direct, 0.0f },
			{ "fCombatHealthRegenMult", &MCM::Settings::Character::fCombatHealthRegenMult, Mode::Direct, 0.0f },
		} };
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		GameSettings::Apply("Character", std::span<const FloatTarget>{ kFormulaTargets });
		GameSettings::Apply("Character", std::span<const FloatTarget>{ kRegenTargets });
	}
}
