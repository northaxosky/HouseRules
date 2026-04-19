#include "PCH.h"

#include "Hooks/Unlocks.h"

#include "Settings.h"

#include <memory>
#include <vector>

namespace Hooks::Unlocks
{
	namespace
	{
		using GDLHook = REL::Hook<RE::DifficultyLevel(RE::PlayerCharacter*)>;
		using VoidPCHook = REL::Hook<void(RE::PlayerCharacter*)>;

		// Hooks live for the plugin's lifetime — store as base pointers so we
		// can hold different signatures in the same container.
		std::vector<std::unique_ptr<REL::HookObject>> g_hooks;

		template <class Hook, class Fn>
		void AddHook(std::uint64_t a_fn_id, std::size_t a_offset, Fn* a_fn, const char* a_label)
		{
			auto hook = std::make_unique<Hook>(REL::ID(a_fn_id), a_offset, a_fn);
			const bool init_ok   = hook->Init();
			const bool enable_ok = hook->Enable();
			REX::INFO("Unlocks: hook {} (id={} +0x{:X}) init={} enable={}",
				a_label, a_fn_id, a_offset, init_ok, enable_ok);
			g_hooks.push_back(std::move(hook));
		}

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

		// RequestQueueDoorAutosave replacement. In vanilla, this fn unconditionally
		// queues a door-autosave unless in Survival. We intercept the call and only
		// queue when the user wants autosaves (bSaveAuto ON) or the game isn't in
		// Survival at all.
		void Replace_RequestQueueDoorAutosave(RE::PlayerCharacter* a_this)
		{
			if (!a_this) {
				return;
			}
			if (RealDifficulty(a_this) != RE::DifficultyLevel::kTrueSurvival) {
				a_this->doorAutosaveQueued = true;
				return;
			}
			if (MCM::Settings::General::bEnabled.GetValue() &&
				MCM::Settings::Unlocks::bSaveAuto.GetValue()) {
				a_this->doorAutosaveQueued = true;
			}
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
				{ 1330449, 0x0C1, &Spoof_SaveSelf,         "SaveSelf / PauseMenu::InitMainList" },

				{ 712982,  0x31E, &Spoof_FastTravel,       "FastTravel / PipboyMenu::PipboyMenu" },
				{ 1327120, 0x013, &Spoof_FastTravel,       "FastTravel / nsPipboyMenu::CheckHardcoreFastTravel" },

				{ 1158548, 0x04E, &Spoof_SaveAuto,         "SaveAuto / LevelUpMenu dtor" },
				{ 98443,   0x193, &Spoof_SaveAuto,         "SaveAuto / WorkshopMenu dtor" },
				{ 1231000, 0x18A, &Spoof_SaveAuto,         "SaveAuto / PipboyManager::OnPipboyCloseAnim" },
				{ 146861,  0x678, &Spoof_SaveAuto,         "SaveAuto / PlayerCharacter::HandlePositionPlayerRequest (autosave on arrival)" },

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
				AddHook<GDLHook>(s.fn_id, s.offset, s.spoof, s.label);
			}

			// Door-transition autosave: replace the call to RequestQueueDoorAutosave
			// from inside TESObjectDOOR::DoorTeleportPlayerArrivalCallback.
			AddHook<VoidPCHook>(
				1397471, 0x1B4, &Replace_RequestQueueDoorAutosave,
				"SaveAuto / TESObjectDOOR::DoorTeleportPlayerArrivalCallback");
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
