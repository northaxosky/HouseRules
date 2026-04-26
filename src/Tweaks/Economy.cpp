#include "PCH.h"

#include "Tweaks/Economy.h"

#include "Settings.h"
#include "Tweaks/GameSettings.h"

#include <array>

namespace Tweaks::Economy
{
	namespace
	{
		using GameSettings::FloatTarget;
		using GameSettings::Mode;

		// Note on semantics: raising fBarterMin pushes BUY prices up;
		// raising fBarterMax pushes SELL payouts down. MCM labels
		// surface this in plain English.
		const std::array<FloatTarget, 4> kTargets = { {
			{ "fBarterMin",     &MCM::Settings::Economy::fBarterMin,     Mode::Direct, 2.0f },
			{ "fBarterMax",     &MCM::Settings::Economy::fBarterMax,     Mode::Direct, 3.5f },
			{ "fBarterBuyMax",  &MCM::Settings::Economy::fBarterBuyMax,  Mode::Direct, 1.2f },
			{ "fBarterSellMax", &MCM::Settings::Economy::fBarterSellMax, Mode::Direct, 0.8f },
		} };
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		GameSettings::Apply("Economy", std::span<const FloatTarget>{ kTargets });
	}
}
