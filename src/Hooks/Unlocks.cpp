#include "PCH.h"

#include "Hooks/Unlocks.h"

#include "Settings.h"

#include <memory>
#include <vector>

namespace Hooks::Unlocks
{
	namespace
	{
		using HookType = REL::Hook<RE::DifficultyLevel(RE::PlayerCharacter*)>;

		// Hooks live for the plugin's lifetime — owned here so they aren't
		// destroyed after Install() returns.
		std::vector<std::unique_ptr<HookType>> g_hooks;

		RE::DifficultyLevel RealDifficulty(RE::PlayerCharacter* a_this)
		{
			using func_t = RE::DifficultyLevel (*)(RE::PlayerCharacter*);
			static REL::Relocation<func_t> real{ RE::ID::PlayerCharacter::GetDifficultyLevel };
			return real(a_this);
		}

		// When the matching MCM toggle is on, return kVeryEasy so the game's
		// Survival gate treats the current session as non-Survival for this
		// particular call site; otherwise fall through to the real difficulty.
		bool ShouldSpoof(bool a_toggle)
		{
			return MCM::Settings::General::bEnabled.GetValue() && a_toggle;
		}

		RE::DifficultyLevel Spoof_Console(RE::PlayerCharacter* a_this)
		{
			return ShouldSpoof(MCM::Settings::Unlocks::bConsole.GetValue())
				? RE::DifficultyLevel::kVeryEasy
				: RealDifficulty(a_this);
		}

		RE::DifficultyLevel Spoof_SaveSelf(RE::PlayerCharacter* a_this)
		{
			return ShouldSpoof(MCM::Settings::Unlocks::bSaveSelf.GetValue())
				? RE::DifficultyLevel::kVeryEasy
				: RealDifficulty(a_this);
		}

		RE::DifficultyLevel Spoof_FastTravel(RE::PlayerCharacter* a_this)
		{
			return ShouldSpoof(MCM::Settings::Unlocks::bFastTravel.GetValue())
				? RE::DifficultyLevel::kVeryEasy
				: RealDifficulty(a_this);
		}

		RE::DifficultyLevel Spoof_SaveAuto(RE::PlayerCharacter* a_this)
		{
			return ShouldSpoof(MCM::Settings::Unlocks::bSaveAuto.GetValue())
				? RE::DifficultyLevel::kVeryEasy
				: RealDifficulty(a_this);
		}

		RE::DifficultyLevel Spoof_CompassEnemies(RE::PlayerCharacter* a_this)
		{
			return ShouldSpoof(MCM::Settings::Unlocks::bCompassEnemies.GetValue())
				? RE::DifficultyLevel::kVeryEasy
				: RealDifficulty(a_this);
		}

		RE::DifficultyLevel Spoof_CompassLocations(RE::PlayerCharacter* a_this)
		{
			return ShouldSpoof(MCM::Settings::Unlocks::bCompassLocations.GetValue())
				? RE::DifficultyLevel::kVeryEasy
				: RealDifficulty(a_this);
		}

		RE::DifficultyLevel Spoof_NoAlchWeight(RE::PlayerCharacter* a_this)
		{
			return ShouldSpoof(MCM::Settings::Unlocks::bNoAlchWeight.GetValue())
				? RE::DifficultyLevel::kVeryEasy
				: RealDifficulty(a_this);
		}

		RE::DifficultyLevel Spoof_NoAmmoWeight(RE::PlayerCharacter* a_this)
		{
			return ShouldSpoof(MCM::Settings::Unlocks::bNoAmmoWeight.GetValue())
				? RE::DifficultyLevel::kVeryEasy
				: RealDifficulty(a_this);
		}

		void InstallOG()
		{
			struct Site
			{
				std::uint64_t fn_id;
				std::size_t   offset;
				RE::DifficultyLevel (*spoof)(RE::PlayerCharacter*);
				const char* label;
			};

			// OG 1.10.163 hook sites. See .local/findings/og-hook-sites.md.
			const Site sites[] = {
				{ 927099,  0x20A, &Spoof_Console,          "Console / MenuOpenHandler" },

				{ 1470086, 0x06C, &Spoof_SaveSelf,         "SaveSelf / QuickSaveLoadHandler" },
				{ 425422,  0x047, &Spoof_SaveSelf,         "SaveSelf / PauseMenu::CheckIfSaveLoadPossible" },

				{ 712982,  0x31E, &Spoof_FastTravel,       "FastTravel / PipboyMenu::PipboyMenu" },
				{ 1327120, 0x013, &Spoof_FastTravel,       "FastTravel / nsPipboyMenu::CheckHardcoreFastTravel" },
				{ 146861,  0x678, &Spoof_FastTravel,       "FastTravel / PlayerCharacter::HandlePositionPlayerRequest" },

				{ 463507,  0x02B, &Spoof_SaveAuto,         "SaveAuto / PauseMenu::SaveSettings_Derived" },
				{ 1158548, 0x04E, &Spoof_SaveAuto,         "SaveAuto / LevelUpMenu dtor" },
				{ 98443,   0x193, &Spoof_SaveAuto,         "SaveAuto / WorkshopMenu dtor" },
				{ 1231000, 0x18A, &Spoof_SaveAuto,         "SaveAuto / PipboyManager::OnPipboyCloseAnim" },

				{ 1475119, 0x017, &Spoof_CompassEnemies,   "CompassEnemies / HUDMarkerUtils::GetHostileEnemyMaxDistance" },

				{ 1301956, 0x00B, &Spoof_CompassLocations, "CompassLocations / HUDMarkerUtils::GetLocationMaxDistance" },
				{ 1153736, 0x0A2, &Spoof_CompassLocations, "CompassLocations / CalculateCompassMarkersFunctor::UpdateLocationMarkers" },

				// Both sites target TESWeightForm::GetFormWeight; we can't distinguish alch vs ammo
				// without inspecting the form, so each site is tied to one toggle. First site → alch,
				// second → ammo. Refine once we inspect the TESForm arg.
				{ 1321341, 0x092, &Spoof_NoAlchWeight,     "Weight / TESWeightForm::GetFormWeight (alch)" },
				{ 1321341, 0x11C, &Spoof_NoAmmoWeight,     "Weight / TESWeightForm::GetFormWeight (ammo)" },
			};

			for (const auto& s : sites) {
				auto hook = std::make_unique<HookType>(REL::ID(s.fn_id), s.offset, s.spoof);
				const bool ok = hook->Init();
				REX::INFO("Unlocks: hook {} (id={} +0x{:X}) init={}", s.label, s.fn_id, s.offset, ok);
				g_hooks.push_back(std::move(hook));
			}
		}
	}

	void Install()
	{
		if (!REL::Module::IsRuntimeOG()) {
			REX::WARN("Unlocks: non-OG runtime; hooks disabled for v0.1");
			return;
		}
		InstallOG();
	}
}
