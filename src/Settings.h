#pragma once

namespace MCM
{
	class Settings
	{
	public:
		class General
		{
		public:
			inline static REX::INI::Bool<>bEnabled{ "General", "bEnabled", true };
		};

		// `false` leaves the vanilla Survival restriction in place.
		class Unlocks
		{
		public:
			inline static REX::INI::Bool<>bConsole{ "Unlocks", "bConsole", true };
			inline static REX::INI::Bool<>bFastTravel{ "Unlocks", "bFastTravel", false };
			inline static REX::INI::Bool<>bGodMode{ "Unlocks", "bGodMode", false };
			inline static REX::INI::Bool<>bSaveAuto{ "Unlocks", "bSaveAuto", false };
			inline static REX::INI::Bool<>bSaveSelf{ "Unlocks", "bSaveSelf", false };
			inline static REX::INI::Bool<>bSleepSave{ "Unlocks", "bSleepSave", true };
			inline static REX::INI::Bool<>bCompassEnemies{ "Unlocks", "bCompassEnemies", false };
			inline static REX::INI::Bool<>bCompassLocations{ "Unlocks", "bCompassLocations", false };
			inline static REX::INI::Bool<>bAlchWeight{ "Unlocks", "bAlchWeight", true };
			inline static REX::INI::Bool<>bAmmoWeight{ "Unlocks", "bAmmoWeight", true };
			inline static REX::INI::Bool<>bLockSurvival{ "Unlocks", "bLockSurvival", false };
		};

		// 1.0 = vanilla rate; 0.0 disables the need entirely.
		class NeedRates
		{
		public:
			inline static REX::INI::F32<>fHungerRate{ "NeedRates", "fHungerRate", 1.0f };
			inline static REX::INI::F32<>fThirstRate{ "NeedRates", "fThirstRate", 1.0f };
			inline static REX::INI::F32<>fSleepRate{ "NeedRates", "fSleepRate", 1.0f };
		};

		static void Update();
	};
}
