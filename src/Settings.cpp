#include "PCH.h"

#include "Settings.h"

#include "Diagnostics/Logging.h"
#include "Globals/Globals.h"
#include "Hooks/GodMode.h"
#include "Hooks/Unlocks.h"

namespace MCM
{
	void Settings::Update()
	{
		const auto ini = REX::INI::SettingStore::GetSingleton();
		ini->Init(
			"Data/MCM/Config/HouseRules/settings.ini",
			"Data/MCM/Settings/HouseRules.ini");
		ini->Load();

		Diagnostics::Logging::ApplyLogLevel();

		Hooks::Unlocks::RefreshRuntimePatches();
		Hooks::GodMode::RefreshRuntimePatches();

		// ESP-backed toggles: write the gating globals shipped in
		// HouseRules.esp. If the ESP is absent, Globals::Write logs once
		// and no-ops -- everything else keeps working.
		::Globals::WriteBool("HR_NoCarryWeight",
			Unlocks::bNoSurvivalCarryWeight.GetValue());

		// Magnitudes::Apply is NOT called here: on kGameDataReady, forms /
		// UI are mid-init on the worker thread and touching them crashes.
		// Main.cpp's PauseMenu-close sink handles it (safe context).
	}
}
