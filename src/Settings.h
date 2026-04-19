#pragma once

namespace MCM
{
	class Settings
	{
	public:
		class General
		{
		public:
			inline static REX::INI::Bool<> bEnabled{ "General", "bEnabled", true };
		};

		// All toggles: OFF = vanilla Survival behavior (default), ON = tweak applied.
		class Unlocks
		{
		public:
			inline static REX::INI::Bool<> bConsole{ "Unlocks", "bConsole", false };
			inline static REX::INI::Bool<> bFastTravel{ "Unlocks", "bFastTravel", false };
			inline static REX::INI::Bool<> bGodMode{ "Unlocks", "bGodMode", false };
			inline static REX::INI::Bool<> bSaveAuto{ "Unlocks", "bSaveAuto", false };
			inline static REX::INI::Bool<> bSaveSelf{ "Unlocks", "bSaveSelf", false };
			inline static REX::INI::Bool<> bSleepSave{ "Unlocks", "bSleepSave", false };
			inline static REX::INI::Bool<> bCompassEnemies{ "Unlocks", "bCompassEnemies", false };
			inline static REX::INI::Bool<> bCompassLocations{ "Unlocks", "bCompassLocations", false };
			inline static REX::INI::Bool<> bNoAlchWeight{ "Unlocks", "bNoAlchWeight", false };
			inline static REX::INI::Bool<> bNoAmmoWeight{ "Unlocks", "bNoAmmoWeight", false };
			inline static REX::INI::Bool<> bLockSurvival{ "Unlocks", "bLockSurvival", false };
		};

		static void Update();
	};
}
