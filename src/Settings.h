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
			inline static REX::INI::Bool<> bCompassEnemies{ "Unlocks", "bCompassEnemies", false };
			inline static REX::INI::Bool<> bCompassLocations{ "Unlocks", "bCompassLocations", false };
			inline static REX::INI::Bool<> bNoAlchWeight{ "Unlocks", "bNoAlchWeight", false };
			inline static REX::INI::Bool<> bNoAmmoWeight{ "Unlocks", "bNoAmmoWeight", false };
			inline static REX::INI::Bool<> bReenableSurvival{ "Unlocks", "bReenableSurvival", false };
		};

		// Runtime multipliers on vanilla magnitudes. 1.0 = vanilla. 0.0 = effect nulled.
		class Magnitudes
		{
		public:
			inline static REX::INI::F32<> fStimpakHeal      { "Magnitudes", "fStimpakHeal",       1.0f };
			inline static REX::INI::F32<> fStimpakLimbRepair{ "Magnitudes", "fStimpakLimbRepair", 1.0f };
			inline static REX::INI::F32<> fRadAwayStrength  { "Magnitudes", "fRadAwayStrength",   1.0f };
			inline static REX::INI::F32<> fRadXPotency      { "Magnitudes", "fRadXPotency",       1.0f };
			inline static REX::INI::F32<> fFoodHeal         { "Magnitudes", "fFoodHeal",          1.0f };
			inline static REX::INI::F32<> fWaterSatiation   { "Magnitudes", "fWaterSatiation",    1.0f };
			inline static REX::INI::F32<> fHungerPenalty    { "Magnitudes", "fHungerPenalty",     1.0f };
			inline static REX::INI::F32<> fThirstPenalty    { "Magnitudes", "fThirstPenalty",     1.0f };
			inline static REX::INI::F32<> fSleepPenalty     { "Magnitudes", "fSleepPenalty",      1.0f };
		};

		// Developer-only. Not exposed in MCM. Edit settings.ini directly.
		class Diagnostic
		{
		public:
			inline static REX::INI::Bool<> bDumpOnLoad{
				"Diagnostic", "bDumpOnLoad", false
			};
			inline static REX::INI::Str<> sDumpFilter{
				"Diagnostic", "sDumpFilter",
				std::string{ "Hardcore,Survival,Hunger,Thirst,Sleep,Fatigue,Stimpak,RadAway,RadX,Peckish,Weary,Parched,Dehydr,Starv" }
			};
		};

		static void Update();
	};
}
