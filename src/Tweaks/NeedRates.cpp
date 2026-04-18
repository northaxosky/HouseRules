#include "PCH.h"

#include "Tweaks/NeedRates.h"

#include "Settings.h"

namespace Tweaks::NeedRates
{
	namespace
	{
		// TODO: cache the GMST pointers on first apply rather than re-looking them up.
		void SetGameSetting(const char* a_name, float a_value)
		{
			const auto dataHandler = RE::TESDataHandler::GetSingleton();
			if (!dataHandler) {
				return;
			}

			const auto setting = dataHandler->LookupForm<RE::TESGlobal>(0, a_name);
			if (!setting) {
				spdlog::warn("NeedRates: GMST '{}' not found", a_name);
				return;
			}

			setting->value = a_value;
		}
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled) {
			return;
		}

		// TODO: confirm the correct GMST names vs globals; Survival Options exposed
		// fSurvivalNeedHunger / fSurvivalNeedThirst / fSurvivalNeedSleep. We'll likely
		// need GameSettingCollection::GetSetting for the f* ones and TESGlobal lookup
		// only for globals. This stub logs intent; wire up properly in follow-up.
		spdlog::info(
			"NeedRates::Apply hunger={} thirst={} sleep={}",
			MCM::Settings::NeedRates::fHungerRate.GetValue(),
			MCM::Settings::NeedRates::fThirstRate.GetValue(),
			MCM::Settings::NeedRates::fSleepRate.GetValue());
	}
}
