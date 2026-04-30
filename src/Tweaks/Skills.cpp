#include "PCH.h"

#include "Tweaks/Skills.h"

#include "Settings.h"
#include "Tweaks/GameSettings.h"

#include <array>
#include <string>

namespace Tweaks::Skills
{
	namespace
	{
		using GameSettings::FloatTarget;
		using GameSettings::IntTarget;
		using GameSettings::Mode;

		const std::array<FloatTarget, 13> kFloatTargets = { {
			// Pickpocket
			{ "fPickPocketMinChance",             &MCM::Settings::Skills::fPickPocketMinChance,        Mode::Direct, 0.0f  },
			{ "fPickPocketMaxChance",             &MCM::Settings::Skills::fPickPocketMaxChance,        Mode::Direct, 90.0f },
			{ "fProjectileInventoryGrenadeTimer", &MCM::Settings::Skills::fReversePickpocketTimer,     Mode::Direct, 2.0f  },

			// Lockpicking
			{ "fLockPickBreakBase",            &MCM::Settings::Skills::fLockPickBreakBase,            Mode::Direct, 0.05f },
			{ "fLockpickSkillSweetSpotBase",   &MCM::Settings::Skills::fLockpickSkillSweetSpotBase,   Mode::Direct, 0.20f },
			{ "fLockpickBreakApprentice",      &MCM::Settings::Skills::fLockpickBreakApprentice,      Mode::Direct, 2.0f  },
			{ "fLockpickBreakAdept",           &MCM::Settings::Skills::fLockpickBreakAdept,           Mode::Direct, 2.0f  },
			{ "fLockpickBreakExpert",          &MCM::Settings::Skills::fLockpickBreakExpert,          Mode::Direct, 2.0f  },
			{ "fLockpickBreakMaster",          &MCM::Settings::Skills::fLockpickBreakMaster,          Mode::Direct, 2.0f  },
			{ "fSweetspotEasy",                &MCM::Settings::Skills::fSweetspotApprentice,          Mode::Direct, 80.0f },
			{ "fSweetspotAverage",             &MCM::Settings::Skills::fSweetspotAdept,               Mode::Direct, 60.0f },
			{ "fSweetspotHard",                &MCM::Settings::Skills::fSweetspotExpert,              Mode::Direct, 40.0f },
			{ "fSweetspotVeryHard",            &MCM::Settings::Skills::fSweetspotMaster,              Mode::Direct, 20.0f },
		} };

		const std::array<IntTarget, 2> kIntTargets = { {
			{ "iHackingMinWords", &MCM::Settings::Skills::iHackingMinWords, Mode::Direct, std::int32_t{ 6 }  },
			{ "iHackingMaxWords", &MCM::Settings::Skills::iHackingMaxWords, Mode::Direct, std::int32_t{ 20 } },
		} };

		void WarnIfInverted(const char* a_label, float a_min, float a_max, const char* a_key)
		{
			if (a_min > a_max) {
				GameSettings::WarnOnce(
					"Skills",
					a_key,
					std::string{ a_label } + " minimum > maximum; engine behavior may clamp or ignore the range");
			}
		}

		void WarnIfInverted(const char* a_label, std::int32_t a_min, std::int32_t a_max, const char* a_key)
		{
			if (a_min > a_max) {
				GameSettings::WarnOnce(
					"Skills",
					a_key,
					std::string{ a_label } + " minimum > maximum; engine behavior may clamp or ignore the range");
			}
		}
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		GameSettings::Apply("Skills", std::span<const FloatTarget>{ kFloatTargets });
		GameSettings::Apply("Skills", std::span<const IntTarget>{ kIntTargets });

		WarnIfInverted("Pickpocket chance",
			MCM::Settings::Skills::fPickPocketMinChance.GetValue(),
			MCM::Settings::Skills::fPickPocketMaxChance.GetValue(),
			"fPickPocketMinChance");
		WarnIfInverted("Hacking word count",
			MCM::Settings::Skills::iHackingMinWords.GetValue(),
			MCM::Settings::Skills::iHackingMaxWords.GetValue(),
			"iHackingMinWords");
	}
}
