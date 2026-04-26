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

		// Character v0.3 -- AP pool/regen + sprint-drain sliders. Defaults
		// reflect vanilla; with preserveBaselineAtNeutral, Direct entries
		// preserve any baseline edits from other mods until the user moves
		// the slider off the vanilla default.
		class Character
		{
		public:
			// AP pool / regen
			inline static REX::INI::F32<> fAPBaseMult             { "Character", "fAPBaseMult",              1.0f };
			inline static REX::INI::F32<> fAPPerAgilityMult       { "Character", "fAPPerAgilityMult",        1.0f };
			inline static REX::INI::F32<> fAPRegenRateMult        { "Character", "fAPRegenRateMult",         1.0f };
			inline static REX::INI::F32<> fCombatAPRegenMult      { "Character", "fCombatAPRegenMult",       0.75f };
			inline static REX::INI::F32<> fAPRegenDelay           { "Character", "fAPRegenDelay",            1.0f };
			inline static REX::INI::F32<> fOutOfBreathAPRegenDelay{ "Character", "fOutOfBreathAPRegenDelay", 2.0f };
			inline static REX::INI::F32<> fAPRegenDelayMax        { "Character", "fAPRegenDelayMax",         5.0f };

			// Sprint formula
			inline static REX::INI::F32<> fSprintBaseDrain        { "Character", "fSprintBaseDrain",         1.05f };
			inline static REX::INI::F32<> fSprintEnduranceDrain   { "Character", "fSprintEnduranceDrain",   -0.05f };
			inline static REX::INI::F32<> fSprintDrainMult        { "Character", "fSprintDrainMult",         1.0f };

			// Carry weight (Multiplier mode; 1.0 = vanilla)
			inline static REX::INI::F32<> fCarryWeightBaseMult       { "Character", "fCarryWeightBaseMult",       1.0f };
			inline static REX::INI::F32<> fCarryWeightPerStrengthMult{ "Character", "fCarryWeightPerStrengthMult",1.0f };

			// Health scaling (Multiplier mode; 1.0 = vanilla)
			inline static REX::INI::F32<> fHealthPerEnduranceMult    { "Character", "fHealthPerEnduranceMult",    1.0f };
			inline static REX::INI::F32<> fHealthPerLevelMult        { "Character", "fHealthPerLevelMult",        1.0f };

			// Health regen (Direct mode; vanilla 0)
			inline static REX::INI::F32<> fHealRate                  { "Character", "fHealRate",                  0.0f };
			inline static REX::INI::F32<> fCombatHealthRegenMult     { "Character", "fCombatHealthRegenMult",     0.0f };
		};

		// Damage Formulas v0.3 -- raw damage factors and armor reduction
		// exponents for radiation, physical, and energy damage. All Direct
		// mode with vanilla defaults; preserveBaselineAtNeutral keeps any
		// baseline edits from other mods until the user moves a slider off
		// the vanilla default.
		class DamageFormulas
		{
		public:
			// Radiation
			inline static REX::INI::F32<> fRadiationDamageFactor       { "DamageFormulas", "fRadiationDamageFactor",        0.15f };
			inline static REX::INI::F32<> fRadiationArmorReductionExponent{ "DamageFormulas", "fRadiationArmorReductionExponent", 0.365f };

			// Physical
			inline static REX::INI::F32<> fPhysicalDamageFactor        { "DamageFormulas", "fPhysicalDamageFactor",         0.15f };
			inline static REX::INI::F32<> fPhysicalArmorReductionExponent{ "DamageFormulas", "fPhysicalArmorReductionExponent", 0.365f };

			// Energy
			inline static REX::INI::F32<> fEnergyDamageFactor          { "DamageFormulas", "fEnergyDamageFactor",           0.15f };
			inline static REX::INI::F32<> fEnergyArmorReductionExponent{ "DamageFormulas", "fEnergyArmorReductionExponent", 0.365f };
		};

		// Power Armor v0.3 -- Jetpack, fusion-core drain, and PA
		// durability GMSTs. Multiplier defaults at 1.0; Direct defaults
		// match each vanilla GMST baseline so preserveBaselineAtNeutral
		// keeps other mods' edits until the user moves the slider off.
		class PowerArmor
		{
		public:
			// Jetpack (Multiplier mode; 1.0 = vanilla)
			inline static REX::INI::F32<> fJetpackInitialDrainMult   { "PowerArmor", "fJetpackInitialDrainMult",    1.0f };
			inline static REX::INI::F32<> fJetpackSustainedDrainMult { "PowerArmor", "fJetpackSustainedDrainMult",  1.0f };
			inline static REX::INI::F32<> fJetpackInitialThrustMult  { "PowerArmor", "fJetpackInitialThrustMult",   1.0f };
			inline static REX::INI::F32<> fJetpackSustainedThrustMult{ "PowerArmor", "fJetpackSustainedThrustMult", 1.0f };

			// Jetpack (Direct mode; vanilla defaults as neutral)
			inline static REX::INI::F32<> fJetpackMinFuelRequired    { "PowerArmor", "fJetpackMinFuelRequired",     3.0f };
			inline static REX::INI::F32<> fJetpackTimeToSustained    { "PowerArmor", "fJetpackTimeToSustained",     0.15f };

			// Fusion core drain (Multiplier mode; 1.0 = vanilla)
			inline static REX::INI::F32<> fPAPowerDrainPerAPMult     { "PowerArmor", "fPAPowerDrainPerAPMult",      1.0f };
			inline static REX::INI::F32<> fPAPowerDrainRunningMult   { "PowerArmor", "fPAPowerDrainRunningMult",    1.0f };

			// Fusion core drain (Direct mode; vanilla 0)
			inline static REX::INI::F32<> fPAPowerDrainPerJump       { "PowerArmor", "fPAPowerDrainPerJump",        0.0f };
			inline static REX::INI::F32<> fPAPowerDrainPerMeleeAttack{ "PowerArmor", "fPAPowerDrainPerMeleeAttack", 0.0f };
			inline static REX::INI::F32<> fPAPowerDrainPerImpactLand { "PowerArmor", "fPAPowerDrainPerImpactLand",  0.0f };

			// Durability (Direct mode; vanilla defaults as neutral)
			inline static REX::INI::F32<> fPAPlayerArmorDamageMultiplier{ "PowerArmor", "fPAPlayerArmorDamageMultiplier", 1.0f };
			inline static REX::INI::F32<> fPANPCArmorDamageMultiplier   { "PowerArmor", "fPANPCArmorDamageMultiplier",    3.0f };
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
