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
			inline static REX::INI::Bool<> bNoSurvivalCarryWeight{ "Unlocks", "bNoSurvivalCarryWeight", false };
			inline static REX::INI::Bool<> bReenableSurvival{ "Unlocks", "bReenableSurvival", false };
		};

		// Runtime magnitude settings. 1.0 = vanilla for implemented paths; some
		// reserved entries remain inert until backed by runtime evidence.
		class Magnitudes
		{
		public:
			inline static REX::INI::F32<> fStimpakHeal      { "Magnitudes", "fStimpakHeal",       1.0f };
			inline static REX::INI::F32<> fStimpakLimbRepair{ "Magnitudes", "fStimpakLimbRepair", 1.0f };
			inline static REX::INI::F32<> fRadAwayStrength  { "Magnitudes", "fRadAwayStrength",   1.0f };
			inline static REX::INI::F32<> fRadXPotency      { "Magnitudes", "fRadXPotency",       1.0f };
			inline static REX::INI::F32<> fFoodHeal         { "Magnitudes", "fFoodHeal",          1.0f };
			inline static REX::INI::F32<> fHungerPenalty    { "Magnitudes", "fHungerPenalty",     1.0f };
			inline static REX::INI::F32<> fThirstPenalty    { "Magnitudes", "fThirstPenalty",     1.0f };
			inline static REX::INI::F32<> fSleepPenalty     { "Magnitudes", "fSleepPenalty",      1.0f };
		};

		// Per-difficulty multipliers for vanilla GMSTs. 1.0 = vanilla. Survival
		// here means the current TSV difficulty; legacy SV GMSTs are skipped.
		class Difficulty
		{
		public:
			// Incoming damage (player damage taken)
			inline static REX::INI::F32<> fIncomingDamageVeryEasy{ "Difficulty", "fIncomingDamageVeryEasy", 1.0f };
			inline static REX::INI::F32<> fIncomingDamageEasy    { "Difficulty", "fIncomingDamageEasy",     1.0f };
			inline static REX::INI::F32<> fIncomingDamageNormal  { "Difficulty", "fIncomingDamageNormal",   1.0f };
			inline static REX::INI::F32<> fIncomingDamageHard    { "Difficulty", "fIncomingDamageHard",     1.0f };
			inline static REX::INI::F32<> fIncomingDamageVeryHard{ "Difficulty", "fIncomingDamageVeryHard", 1.0f };
			inline static REX::INI::F32<> fIncomingDamageSurvival{ "Difficulty", "fIncomingDamageSurvival", 1.0f };

			// Outgoing damage (player damage dealt)
			inline static REX::INI::F32<> fOutgoingDamageVeryEasy{ "Difficulty", "fOutgoingDamageVeryEasy", 1.0f };
			inline static REX::INI::F32<> fOutgoingDamageEasy    { "Difficulty", "fOutgoingDamageEasy",     1.0f };
			inline static REX::INI::F32<> fOutgoingDamageNormal  { "Difficulty", "fOutgoingDamageNormal",   1.0f };
			inline static REX::INI::F32<> fOutgoingDamageHard    { "Difficulty", "fOutgoingDamageHard",     1.0f };
			inline static REX::INI::F32<> fOutgoingDamageVeryHard{ "Difficulty", "fOutgoingDamageVeryHard", 1.0f };
			inline static REX::INI::F32<> fOutgoingDamageSurvival{ "Difficulty", "fOutgoingDamageSurvival", 1.0f };

			// XP progression
			inline static REX::INI::F32<> fXPBase{ "Difficulty", "fXPBase", 1.0f };
			inline static REX::INI::F32<> fXPMult{ "Difficulty", "fXPMult", 1.0f };

			// Legendary chance
			inline static REX::INI::F32<> fLegendaryChanceVeryEasy{ "Difficulty", "fLegendaryChanceVeryEasy", 1.0f };
			inline static REX::INI::F32<> fLegendaryChanceEasy    { "Difficulty", "fLegendaryChanceEasy",     1.0f };
			inline static REX::INI::F32<> fLegendaryChanceNormal  { "Difficulty", "fLegendaryChanceNormal",   1.0f };
			inline static REX::INI::F32<> fLegendaryChanceHard    { "Difficulty", "fLegendaryChanceHard",     1.0f };
			inline static REX::INI::F32<> fLegendaryChanceVeryHard{ "Difficulty", "fLegendaryChanceVeryHard", 1.0f };
			inline static REX::INI::F32<> fLegendaryChanceSurvival{ "Difficulty", "fLegendaryChanceSurvival", 1.0f };

			// Legendary rarity
			inline static REX::INI::F32<> fLegendaryRarityVeryEasy{ "Difficulty", "fLegendaryRarityVeryEasy", 1.0f };
			inline static REX::INI::F32<> fLegendaryRarityEasy    { "Difficulty", "fLegendaryRarityEasy",     1.0f };
			inline static REX::INI::F32<> fLegendaryRarityNormal  { "Difficulty", "fLegendaryRarityNormal",   1.0f };
			inline static REX::INI::F32<> fLegendaryRarityHard    { "Difficulty", "fLegendaryRarityHard",     1.0f };
			inline static REX::INI::F32<> fLegendaryRarityVeryHard{ "Difficulty", "fLegendaryRarityVeryHard", 1.0f };
			inline static REX::INI::F32<> fLegendaryRaritySurvival{ "Difficulty", "fLegendaryRaritySurvival", 1.0f };

			// Effect duration (Difficulty II). Multiplies the per-difficulty
			// effect-duration GMST. Survival's TSV baseline is much longer
			// than Normal, so 1.00 here means "vanilla Survival baseline".
			inline static REX::INI::F32<> fEffectDurationVeryEasy{ "Difficulty", "fEffectDurationVeryEasy", 1.0f };
			inline static REX::INI::F32<> fEffectDurationEasy    { "Difficulty", "fEffectDurationEasy",     1.0f };
			inline static REX::INI::F32<> fEffectDurationNormal  { "Difficulty", "fEffectDurationNormal",   1.0f };
			inline static REX::INI::F32<> fEffectDurationHard    { "Difficulty", "fEffectDurationHard",     1.0f };
			inline static REX::INI::F32<> fEffectDurationVeryHard{ "Difficulty", "fEffectDurationVeryHard", 1.0f };
			inline static REX::INI::F32<> fEffectDurationSurvival{ "Difficulty", "fEffectDurationSurvival", 1.0f };

			// Effect magnitude (Difficulty II). Multiplies the per-difficulty
			// effect-magnitude GMST. Survival's TSV baseline is much smaller
			// than Normal, so 1.00 here means "vanilla Survival baseline".
			inline static REX::INI::F32<> fEffectMagnitudeVeryEasy{ "Difficulty", "fEffectMagnitudeVeryEasy", 1.0f };
			inline static REX::INI::F32<> fEffectMagnitudeEasy    { "Difficulty", "fEffectMagnitudeEasy",     1.0f };
			inline static REX::INI::F32<> fEffectMagnitudeNormal  { "Difficulty", "fEffectMagnitudeNormal",   1.0f };
			inline static REX::INI::F32<> fEffectMagnitudeHard    { "Difficulty", "fEffectMagnitudeHard",     1.0f };
			inline static REX::INI::F32<> fEffectMagnitudeVeryHard{ "Difficulty", "fEffectMagnitudeVeryHard", 1.0f };
			inline static REX::INI::F32<> fEffectMagnitudeSurvival{ "Difficulty", "fEffectMagnitudeSurvival", 1.0f };
		};

		// Developer-only. Not exposed in MCM. Edit settings.ini directly.
		class Diagnostic
		{
		public:
			inline static REX::INI::Bool<> bSurvivalObserver{
				"Diagnostic", "bSurvivalObserver", false
			};
			inline static REX::INI::Bool<> bSurvivalObserverActorValues{
				"Diagnostic", "bSurvivalObserverActorValues", true
			};
			inline static REX::INI::Bool<> bSurvivalObserverActiveEffects{
				"Diagnostic", "bSurvivalObserverActiveEffects", true
			};
			inline static REX::INI::Bool<> bSurvivalObserverLifecycle{
				"Diagnostic", "bSurvivalObserverLifecycle", true
			};
			inline static REX::INI::Str<> sSurvivalObserverFilter{
				"Diagnostic", "sSurvivalObserverFilter",
				std::string{ "Hardcore,Survival,Hunger,Thirst,Sleep,Fatigue,Peckish,Weary,Parched,Dehydr,Starv" }
			};
			inline static REX::INI::Bool<> bSleepWaitObserver{
				"Diagnostic", "bSleepWaitObserver", false
			};
			inline static REX::INI::Bool<> bSleepWaitObserverLifecycle{
				"Diagnostic", "bSleepWaitObserverLifecycle", true
			};
			inline static REX::INI::Bool<> bSleepWaitObserverSnapshots{
				"Diagnostic", "bSleepWaitObserverSnapshots", true
			};
			inline static REX::INI::Bool<> bDumpOnLoad{
				"Diagnostic", "bDumpOnLoad", false
			};
			inline static REX::INI::Bool<> bMagnitudesTrace{
				"Diagnostic", "bMagnitudesTrace", false
			};
			inline static REX::INI::Bool<> bMagnitudesTraceDeltaOnly{
				"Diagnostic", "bMagnitudesTraceDeltaOnly", true
			};
			inline static REX::INI::Bool<> bMagnitudesAlwaysSummarize{
				"Diagnostic", "bMagnitudesAlwaysSummarize", false
			};
			inline static REX::INI::Bool<> bDifficultyTrace{
				"Diagnostic", "bDifficultyTrace", false
			};
			inline static REX::INI::Bool<> bGameSettingsTrace{
				"Diagnostic", "bGameSettingsTrace", false
			};
			inline static REX::INI::Str<> sDumpFilter{
				"Diagnostic", "sDumpFilter",
				std::string{ "Hardcore,Survival,Hunger,Thirst,Sleep,Fatigue,Stimpak,RadAway,RadX,Peckish,Weary,Parched,Dehydr,Starv" }
			};
		};

		static void Update();
	};
}
