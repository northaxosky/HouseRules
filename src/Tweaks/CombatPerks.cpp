#include "PCH.h"

#include "Tweaks/CombatPerks.h"

#include "Settings.h"
#include "Tweaks/GameSettings.h"

#include <array>
#include <cstdint>

namespace Tweaks::CombatPerks
{
	namespace
	{
		using GameSettings::FloatTarget;
		using GameSettings::IntTarget;
		using GameSettings::Mode;

		const std::array<FloatTarget, 9> kFloatTargets = { {
			// Light Armor perk tier multipliers
			{ "fPerkLightArmorNoviceDamageMult",     &MCM::Settings::CombatPerks::fLightArmorNoviceDamageMult,     Mode::Direct, 1.5f  },
			{ "fPerkLightArmorJourneymanDamageMult", &MCM::Settings::CombatPerks::fLightArmorJourneymanDamageMult, Mode::Direct, 0.5f  },
			{ "fPerkLightArmorExpertSpeedMult",      &MCM::Settings::CombatPerks::fLightArmorExpertSpeedMult,      Mode::Direct, 0.0f  },
			{ "fPerkLightArmorMasterRatingMult",     &MCM::Settings::CombatPerks::fLightArmorMasterRatingMult,     Mode::Direct, 1.5f  },

			// Heavy Armor perk tier multipliers
			{ "fPerkHeavyArmorNoviceDamageMult",     &MCM::Settings::CombatPerks::fHeavyArmorNoviceDamageMult,     Mode::Direct, 1.5f  },
			{ "fPerkHeavyArmorJourneymanDamageMult", &MCM::Settings::CombatPerks::fHeavyArmorJourneymanDamageMult, Mode::Direct, 0.5f  },
			{ "fPerkHeavyArmorExpertSpeedMult",      &MCM::Settings::CombatPerks::fHeavyArmorExpertSpeedMult,      Mode::Direct, 0.5f  },
			{ "fPerkHeavyArmorMasterSpeedMult",      &MCM::Settings::CombatPerks::fHeavyArmorMasterSpeedMult,      Mode::Direct, 0.0f  },
			{ "fPerkHeavyArmorSinkGravityMult",      &MCM::Settings::CombatPerks::fHeavyArmorSinkGravityMult,      Mode::Direct, 15.0f },
		} };

		const std::array<IntTarget, 9> kIntTargets = { {
			// Disarm / stagger / knockdown / paralyze chances
			{ "iPerkAttackDisarmChance",          &MCM::Settings::CombatPerks::iAttackDisarmChance,          Mode::Direct, std::int32_t{ 50 } },
			{ "iPerkBlockDisarmChance",           &MCM::Settings::CombatPerks::iBlockDisarmChance,           Mode::Direct, std::int32_t{ 50 } },
			{ "iPerkBlockStaggerChance",          &MCM::Settings::CombatPerks::iBlockStaggerChance,          Mode::Direct, std::int32_t{  5 } },
			{ "iPerkHandToHandBlockRecoilChance", &MCM::Settings::CombatPerks::iHandToHandBlockRecoilChance, Mode::Direct, std::int32_t{ 25 } },
			{ "iPerkMarksmanKnockdownChance",     &MCM::Settings::CombatPerks::iMarksmanKnockdownChance,     Mode::Direct, std::int32_t{  5 } },
			{ "iPerkMarksmanParalyzeChance",      &MCM::Settings::CombatPerks::iMarksmanParalyzeChance,      Mode::Direct, std::int32_t{  5 } },

			// Armor perk integer thresholds
			{ "iPerkLightArmorMasterMinSum",      &MCM::Settings::CombatPerks::iLightArmorMasterMinSum,      Mode::Direct, std::int32_t{  5 } },
			{ "iPerkHeavyArmorJumpSum",           &MCM::Settings::CombatPerks::iHeavyArmorJumpSum,           Mode::Direct, std::int32_t{ 35 } },
			{ "iPerkHeavyArmorSinkSum",           &MCM::Settings::CombatPerks::iHeavyArmorSinkSum,           Mode::Direct, std::int32_t{ 35 } },
		} };
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		GameSettings::Apply("CombatPerks", std::span<const FloatTarget>{ kFloatTargets });
		GameSettings::Apply("CombatPerks", std::span<const IntTarget>{ kIntTargets });
	}
}
