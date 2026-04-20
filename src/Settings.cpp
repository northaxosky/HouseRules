#include "PCH.h"

#include "Settings.h"

#include "Hooks/GodMode.h"
#include "Hooks/Unlocks.h"
#include "Tweaks/Magnitudes.h"

namespace MCM
{
	void Settings::Update()
	{
		const auto ini = REX::INI::SettingStore::GetSingleton();
		ini->Init(
			"Data/MCM/Config/SurvivalArchitect/settings.ini",
			"Data/MCM/Settings/SurvivalArchitect.ini");
		ini->Load();

		Tweaks::Magnitudes::Apply();
		Hooks::Unlocks::RefreshRuntimePatches();
		Hooks::GodMode::RefreshRuntimePatches();
	}
}
