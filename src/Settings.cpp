#include "PCH.h"

#include "Settings.h"

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

		Hooks::Unlocks::RefreshRuntimePatches();
		Hooks::GodMode::RefreshRuntimePatches();
		// Magnitudes::Apply is NOT called here: on kGameDataReady, forms /
		// UI are mid-init on the worker thread and touching them crashes.
		// Main.cpp's PauseMenu-close sink handles it (safe context).
	}
}
