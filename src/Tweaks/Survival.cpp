#include "PCH.h"

#include "Tweaks/Survival.h"

#include "Globals/Globals.h"
#include "Settings.h"
#include "Survival/HCManagerScript.h"

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

		void ApplyKillSwitches()
		{
			using S = MCM::Settings::Survival;
			Globals::WriteByFormID(kHCRuleSustenanceEffects, S::bDisableSustenance.GetValue()       ? 0.0f : 1.0f);
			Globals::WriteByFormID(kHCRuleSleepEffects,      S::bDisableSleepDeprivation.GetValue() ? 0.0f : 1.0f);
			Globals::WriteByFormID(kHCRuleDiseaseEffects,    S::bDisableDiseases.GetValue()         ? 0.0f : 1.0f);
			Globals::WriteByFormID(kHCRuleAdrenalineOn,      S::bDisableAdrenaline.GetValue()       ? 0.0f : 1.0f);
		}

		void ApplyScriptTuning()
		{
			using S = MCM::Settings::Survival;
			namespace HC = ::Survival::HCManagerScript;

			// Sustenance tuning
			HC::SetInt  ("FoodCostPerTick",                 S::iFoodCostPerTick.GetValue());
			HC::SetInt  ("DrinkCostPerTick",                S::iDrinkCostPerTick.GetValue());
			HC::SetFloat("GamesHoursPerTick",               S::fGamesHoursPerTick.GetValue());
			HC::SetFloat("BonusDigestionHours",             S::fBonusDigestionHours.GetValue());
			HC::SetFloat("SustenanceTickWhileSleepingMult", S::fSustenanceTickWhileSleepingMult.GetValue());
			HC::SetFloat("TickHoursCostPerCombat",          S::fTickHoursCostPerCombat.GetValue());

			// Food stage thresholds
			HC::SetInt("iFoodPoolPeckishAmount",  S::iFoodPoolPeckishAmount.GetValue());
			HC::SetInt("iFoodPoolHungryAmount",   S::iFoodPoolHungryAmount.GetValue());
			HC::SetInt("iFoodPoolFamishedAmount", S::iFoodPoolFamishedAmount.GetValue());
			HC::SetInt("iFoodPoolRavenousAmount", S::iFoodPoolRavenousAmount.GetValue());
			HC::SetInt("iFoodPoolStarvingAmount", S::iFoodPoolStarvingAmount.GetValue());

			// Drink stage thresholds
			HC::SetInt("iDrinkPoolParchedAmount",            S::iDrinkPoolParchedAmount.GetValue());
			HC::SetInt("iDrinkPoolThirstyAmount",            S::iDrinkPoolThirstyAmount.GetValue());
			HC::SetInt("iDrinkPoolMildlyDehydratedAmount",   S::iDrinkPoolMildlyDehydratedAmount.GetValue());
			HC::SetInt("iDrinkPoolDehydratedAmount",         S::iDrinkPoolDehydratedAmount.GetValue());
			HC::SetInt("iDrinkPoolSeverelyDehydratedAmount", S::iDrinkPoolSeverelyDehydratedAmount.GetValue());

			// Sleep tuning (vanilla typo "Disase..." preserved on the script property name)
			HC::SetFloat("GameTimerInterval_SleepDeprivation",              S::fGameTimerIntervalSleepDeprivation.GetValue());
			HC::SetFloat("InsomniaSleepMult",                               S::fInsomniaSleepMult.GetValue());
			HC::SetInt  ("MinHoursForCuringSleepEffects",                   S::iMinHoursForCuringSleepEffects.GetValue());
			HC::SetFloat("CaffeineInducedSleepDelay",                       S::fCaffeineInducedSleepDelay.GetValue());
			HC::SetFloat("ExtraCaffeineInducedSleepDelay",                  S::fExtraCaffeineInducedSleepDelay.GetValue());
			HC::SetFloat("DisaseSleepinessSleepDeprivationTimerMult",       S::fDiseasedSleepinessTimerMult.GetValue());

			// Disease tuning
			HC::SetFloat("DiseaseGracePeriod",            S::fDiseaseGracePeriod.GetValue());
			HC::SetFloat("DiseaseRiskRollThreshold",      S::fDiseaseRiskRollThreshold.GetValue());
			HC::SetFloat("DiseaseRiskDrainPerCycle",      S::fDiseaseRiskDrainPerCycle.GetValue());
			HC::SetFloat("CurrentDiseasePoolValueMult",   S::fCurrentDiseasePoolValueMult.GetValue());
			HC::SetFloat("DiseaseNeedMoreFoodMult",       S::fDiseaseNeedMoreFoodMult.GetValue());
			HC::SetFloat("ImmunodeficiencyDiseaseChanceMult", S::fImmunodeficiencyDiseaseMult.GetValue());
			HC::SetFloat("DiseaseRiskCannibalAmount",     S::fDiseaseRiskCannibalAmount.GetValue());
			HC::SetFloat("DiseaseRiskChemsAmount",        S::fDiseaseRiskChemsAmount.GetValue());
			HC::SetFloat("DiseaseRiskCombatantAmount",    S::fDiseaseRiskCombatantAmount.GetValue());
			HC::SetFloat("DiseaseRiskFoodHighAmount",     S::fDiseaseRiskFoodHighAmount.GetValue());
			HC::SetFloat("DiseaseRiskFoodStandardAmount", S::fDiseaseRiskFoodStandardAmount.GetValue());
			HC::SetFloat("DiseaseRiskRainAmount",         S::fDiseaseRiskRainAmount.GetValue());
			HC::SetFloat("DiseaseRiskSwimmingAmount",     S::fDiseaseRiskSwimmingAmount.GetValue());

			// Adrenaline tuning (MaxAdrenaline itself is a script-local, derived from these two)
			HC::SetInt("KillsForAdrenalinePerkLevel", S::iKillsForAdrenalinePerkLevel.GetValue());
			HC::SetInt("MaxAdrenalinePerkLevel",      S::iMaxAdrenalinePerkLevel.GetValue());

			HC::SetFloat("GameTimerInterval_Encumbrance", S::fGameTimerIntervalEncumbrance.GetValue());
		}
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}
		ApplyKillSwitches();
		ApplyScriptTuning();
	}
}
