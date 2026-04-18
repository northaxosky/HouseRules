#include "PCH.h"

#include "Tweaks/NeedRates.h"

#include "Settings.h"

#include <unordered_map>

namespace Tweaks::NeedRates
{
	namespace
	{
		// Baseline GMST values captured on first Apply(); subsequent applies
		// multiply this snapshot rather than the already-modified engine value.
		std::unordered_map<std::string_view, float> g_snapshot;

		void ApplyRate(std::string_view a_name, float a_multiplier)
		{
			const auto collection = RE::GameSettingCollection::GetSingleton();
			if (!collection) {
				REX::WARN("NeedRates: GameSettingCollection unavailable");
				return;
			}

			const auto setting = collection->GetSetting(a_name);
			if (!setting) {
				REX::WARN("NeedRates: GMST '{}' not found", a_name);
				return;
			}

			const auto [it, inserted] = g_snapshot.try_emplace(a_name, setting->GetFloat());
			const auto base = it->second;
			const auto next = base * a_multiplier;
			setting->SetFloat(next);

			REX::INFO("NeedRates: {} base={} mult={} -> {}", a_name, base, a_multiplier, next);
		}
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		ApplyRate("fSurvivalNeedHungerRateBase", MCM::Settings::NeedRates::fHungerRate.GetValue());
		ApplyRate("fSurvivalNeedThirstRateBase", MCM::Settings::NeedRates::fThirstRate.GetValue());
		ApplyRate("fSurvivalNeedSleepRateBase", MCM::Settings::NeedRates::fSleepRate.GetValue());
	}
}
