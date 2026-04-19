#include "PCH.h"

#include "Hooks/Unlocks.h"

#include "Settings.h"

#include <memory>
#include <vector>

namespace Hooks::Unlocks
{
	namespace
	{
		using GDLFn = RE::DifficultyLevel(RE::PlayerCharacter*);
		using GDLHook = REL::Hook<GDLFn>;

		using VoidPCFn = void(RE::PlayerCharacter*);
		using VoidPCHook = REL::Hook<VoidPCFn>;

		std::vector<std::unique_ptr<REL::HookObject>> g_hooks;

		template <class HookT, class FnT>
		void AddHook(std::uint64_t a_id, std::size_t a_offset, FnT* a_fn, const char* a_label)
		{
			auto hook = std::make_unique<HookT>(REL::ID(a_id), a_offset, a_fn);
			const bool init_ok   = hook->Init();
			const bool enable_ok = hook->Enable();
			REX::INFO("Unlocks: hook {} (id={} +0x{:X}) init={} enable={}",
				a_label, a_id, a_offset, init_ok, enable_ok);
			g_hooks.push_back(std::move(hook));
		}

		// Pick the (id, offset) for the current runtime. NG and AE share values in
		// practice (verified by our AE scan matching Baka's NG offsets 1:1 for every
		// hook). An id of 0 means "skip on this runtime" — a call site that exists
		// on NG/AE but not OG, or vice versa.
		bool SelectSite(
			std::uint64_t og_id, std::uint64_t ng_id,
			std::size_t og_off, std::size_t ng_off,
			std::uint64_t& out_id, std::size_t& out_off)
		{
			using RT = REL::Module::Runtime;
			switch (REL::Module::GetRuntimeIndex()) {
				case RT::kOG: out_id = og_id; out_off = og_off; break;
				case RT::kNG:
				case RT::kAE: out_id = ng_id; out_off = ng_off; break;
				default:      return false;
			}
			return out_id != 0;
		}

		RE::DifficultyLevel RealDifficulty(RE::PlayerCharacter* a_this)
		{
			using func_t = RE::DifficultyLevel (*)(RE::PlayerCharacter*);
			static REL::Relocation<func_t> real{ RE::ID::PlayerCharacter::GetDifficultyLevel };
			return real(a_this);
		}

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

		struct GDLSite
		{
			std::uint64_t og_id;
			std::uint64_t ng_id;  // AE == NG in practice
			std::size_t   og_off;
			std::size_t   ng_off;  // AE == NG in practice
			GDLFn*        spoof;
			const char*   label;
		};

		struct VoidPCSite
		{
			std::uint64_t og_id;
			std::uint64_t ng_id;
			std::size_t   og_off;
			std::size_t   ng_off;
			VoidPCFn*     fn;
			const char*   label;
		};

		// OG IDs/offsets: our scan of the unpacked 1.10.163 Fallout4.exe,
		// cross-referenced with IDA name-port JSON.
		// NG/AE IDs/offsets: Baka's source (REL::ID + offset), confirmed against
		// our AE 1.11.191 scan. NG and AE match exactly for every hook below.
		// An id of 0 means "skip on this runtime" (call site doesn't exist there).
		const GDLSite kGdlSites[] = {
			{ 927099,  2249425, 0x20A, 0x1AA, &Spoof_Console,          "Console / MenuOpenHandler" },

			{ 1470086, 2249427, 0x06C, 0x06C, &Spoof_SaveSelf,         "SaveSelf / QuickSaveLoadHandler" },
			{ 425422,  2223965, 0x047, 0x04D, &Spoof_SaveSelf,         "SaveSelf / PauseMenu::CheckIfSaveLoadPossible" },
			{ 1330449, 2223964, 0x0C1, 0x0C6, &Spoof_SaveSelf,         "SaveSelf / PauseMenu::InitMainList" },

			{ 712982,  2224179, 0x31E, 0x34C, &Spoof_FastTravel,       "FastTravel / PipboyMenu::PipboyMenu" },
			{ 1327120, 2224206, 0x013, 0x014, &Spoof_FastTravel,       "FastTravel / nsPipboyMenu::CheckHardcoreFastTravel" },

			{ 1158548, 2223294, 0x04E, 0x04E, &Spoof_SaveAuto,         "SaveAuto / LevelUpMenu dtor" },
			// NG/AE have a second LevelUpMenu call site OG doesn't expose.
			{ 0,       2223327, 0,     0x057, &Spoof_SaveAuto,         "SaveAuto / LevelUpMenu dtor (NG/AE extra)" },
			{ 98443,   2224974, 0x193, 0x13E, &Spoof_SaveAuto,         "SaveAuto / WorkshopMenu dtor" },
			{ 1231000, 2225457, 0x18A, 0x1A4, &Spoof_SaveAuto,         "SaveAuto / PipboyManager::OnPipboyCloseAnim" },
			{ 146861,  2232905, 0x678, 0x6A1, &Spoof_SaveAuto,         "SaveAuto / PlayerCharacter::HandlePositionPlayerRequest" },

			{ 1475119, 2220612, 0x017, 0x01B, &Spoof_CompassEnemies,   "CompassEnemies / HUDMarkerUtils::GetHostileEnemyMaxDistance" },

			{ 1301956, 2220611, 0x00B, 0x00B, &Spoof_CompassLocations, "CompassLocations / HUDMarkerUtils::GetLocationMaxDistance" },
			{ 1153736, 2220617, 0x0A2, 0x086, &Spoof_CompassLocations, "CompassLocations / CalculateCompassMarkersFunctor::UpdateLocationMarkers" },

			{ 1321341, 2193446, 0x092, 0x08E, &Spoof_NoAlchWeight,     "Weight / TESWeightForm::GetFormWeight (alch)" },
			{ 1321341, 2193446, 0x11C, 0x110, &Spoof_NoAmmoWeight,     "Weight / TESWeightForm::GetFormWeight (ammo)" },
		};

		const VoidPCSite kVoidPCSites[] = {
			{ 1397471, 2198697, 0x1B4, 0x1B4, &Replace_RequestQueueDoorAutosave,
			  "SaveAuto / TESObjectDOOR::DoorTeleportPlayerArrivalCallback" },
		};
	}

	void Install()
	{
		for (const auto& s : kGdlSites) {
			std::uint64_t id;
			std::size_t   off;
			if (SelectSite(s.og_id, s.ng_id, s.og_off, s.ng_off, id, off)) {
				AddHook<GDLHook>(id, off, s.spoof, s.label);
			}
		}
		for (const auto& s : kVoidPCSites) {
			std::uint64_t id;
			std::size_t   off;
			if (SelectSite(s.og_id, s.ng_id, s.og_off, s.ng_off, id, off)) {
				AddHook<VoidPCHook>(id, off, s.fn, s.label);
			}
		}
	}
}
