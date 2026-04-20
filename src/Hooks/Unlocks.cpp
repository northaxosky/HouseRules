#include "PCH.h"

#include "Hooks/Unlocks.h"

#include "Settings.h"

#include <cstring>
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

		struct RuntimeAddr
		{
			std::uint64_t id     = 0;  // 0 = skip on this runtime
			std::size_t   offset = 0;
		};

		// NG and AE mostly share offsets, but not always. Real-world exception:
		// FastTravel / PipboyMenu is at NG +0x347 / AE +0x34C (5-byte delta).
		// Verify any new site with tools/audit_hook_offsets.py against each runtime.
		const RuntimeAddr* SelectSite(
			const RuntimeAddr& og, const RuntimeAddr& ng, const RuntimeAddr& ae)
		{
			using RT = REL::Module::Runtime;
			const RuntimeAddr* ra = nullptr;
			switch (REL::Module::GetRuntimeIndex()) {
				case RT::kOG: ra = &og; break;
				case RT::kNG: ra = &ng; break;
				case RT::kAE: ra = &ae; break;
				default:      return nullptr;
			}
			return ra->id != 0 ? ra : nullptr;
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

		template <class Fn>
		struct Site
		{
			RuntimeAddr og;
			RuntimeAddr ng;
			RuntimeAddr ae;
			Fn*         fn;
			const char* label;
		};

		// OG offsets: scan of the unpacked 1.10.163 Fallout4.exe, named via IDA
		// port JSON. NG/AE: Baka's source cross-checked with our scans of the
		// unpacked NG/AE binaries. An id of 0 skips on that runtime (call site
		// doesn't exist there, or can't be verified yet).
		//
		// NG and AE mostly match, but the FastTravel/PipboyMenu offset differs
		// (NG 0x347 vs AE 0x34C). Run tools/audit_hook_offsets.py after any
		// change here to catch drift.
		const Site<GDLFn> kGdlSites[] = {
			{ {927099,  0x20A}, {2249425, 0x1AA}, {2249425, 0x1AA}, &Spoof_Console,          "Console / MenuOpenHandler" },

			{ {1470086, 0x06C}, {2249427, 0x06C}, {2249427, 0x06C}, &Spoof_SaveSelf,         "SaveSelf / QuickSaveLoadHandler" },
			{ {425422,  0x047}, {2223965, 0x04D}, {2223965, 0x04D}, &Spoof_SaveSelf,         "SaveSelf / PauseMenu::CheckIfSaveLoadPossible" },
			{ {1330449, 0x0C1}, {2223964, 0x0C6}, {2223964, 0x0C6}, &Spoof_SaveSelf,         "SaveSelf / PauseMenu::InitMainList" },

			{ {712982,  0x31E}, {2224179, 0x347}, {2224179, 0x34C}, &Spoof_FastTravel,       "FastTravel / PipboyMenu::PipboyMenu" },
			{ {1327120, 0x013}, {2224206, 0x014}, {2224206, 0x014}, &Spoof_FastTravel,       "FastTravel / nsPipboyMenu::CheckHardcoreFastTravel" },

			{ {1158548, 0x04E}, {2223294, 0x04E}, {2223294, 0x04E}, &Spoof_SaveAuto,         "SaveAuto / LevelUpMenu dtor" },
			// NG/AE have a second LevelUpMenu call site OG doesn't expose.
			{ {0,       0    }, {2223327, 0x057}, {2223327, 0x057}, &Spoof_SaveAuto,         "SaveAuto / LevelUpMenu dtor (NG/AE extra)" },
			{ {98443,   0x193}, {2224974, 0x13E}, {2224974, 0x13E}, &Spoof_SaveAuto,         "SaveAuto / WorkshopMenu dtor" },
			{ {1231000, 0x18A}, {2225457, 0x1A4}, {2225457, 0x1A4}, &Spoof_SaveAuto,         "SaveAuto / PipboyManager::OnPipboyCloseAnim" },
			{ {146861,  0x678}, {2232905, 0x6A1}, {2232905, 0x6A1}, &Spoof_SaveAuto,         "SaveAuto / PlayerCharacter::HandlePositionPlayerRequest" },

			{ {1475119, 0x017}, {2220612, 0x01B}, {2220612, 0x01B}, &Spoof_CompassEnemies,   "CompassEnemies / HUDMarkerUtils::GetHostileEnemyMaxDistance" },

			{ {1301956, 0x00B}, {2220611, 0x00B}, {2220611, 0x00B}, &Spoof_CompassLocations, "CompassLocations / HUDMarkerUtils::GetLocationMaxDistance" },
			{ {1153736, 0x0A2}, {2220617, 0x086}, {2220617, 0x086}, &Spoof_CompassLocations, "CompassLocations / CalculateCompassMarkersFunctor::UpdateLocationMarkers" },

			{ {1321341, 0x092}, {2193446, 0x08E}, {2193446, 0x08E}, &Spoof_NoAlchWeight,     "Weight / TESWeightForm::GetFormWeight (alch)" },
			{ {1321341, 0x11C}, {2193446, 0x110}, {2193446, 0x110}, &Spoof_NoAmmoWeight,     "Weight / TESWeightForm::GetFormWeight (ammo)" },
		};

		const Site<VoidPCFn> kVoidPCSites[] = {
			{ {1397471, 0x1B4}, {2198697, 0x1B4}, {2198697, 0x1B4}, &Replace_RequestQueueDoorAutosave,
			  "SaveAuto / TESObjectDOOR::DoorTeleportPlayerArrivalCallback" },
		};

		// --- Re-enable Survival (byte patch) -----------------------------------
		//
		// Vanilla blocks re-entering Survival mode after you leave it by calling
		// `MiscStatManager::QueryStat` inside `PauseMenu::CheckIfSaveLoadPossible`
		// to check a save-tied "has left survival" stat. USM NOPs that call; we
		// do the same, but sig-scan so one patch handles OG/NG/AE.
		//
		// Pattern: E8 ?? ?? ?? ?? 44 39 75 67  (CALL rel32 + cmp [rbp+0x67], r14d)
		// Confirmed at OG RVA 0xB7FE2D.

		std::uintptr_t g_reenable_addr      = 0;
		std::uint8_t   g_reenable_original[5]{};
		bool           g_reenable_saved     = false;
		bool           g_reenable_patched   = false;

		std::uintptr_t FindReenableSignature()
		{
			const auto mod = REL::Module::GetSingleton();
			if (!mod) {
				return 0;
			}
			const auto text = mod->segment(REL::Segment::text);
			const auto base = text.pointer<std::uint8_t>();
			const auto size = text.size();
			for (std::size_t i = 0; i + 9 <= size; ++i) {
				if (base[i] == 0xE8 &&
					base[i + 5] == 0x44 &&
					base[i + 6] == 0x39 &&
					base[i + 7] == 0x75 &&
					base[i + 8] == 0x67) {
					return reinterpret_cast<std::uintptr_t>(base + i);
				}
			}
			return 0;
		}

		void ApplyReenablePatchState()
		{
			if (g_reenable_addr == 0 || !g_reenable_saved) {
				return;
			}
			const bool want_patched =
				MCM::Settings::General::bEnabled.GetValue() &&
				MCM::Settings::Unlocks::bReenableSurvival.GetValue();
			if (want_patched == g_reenable_patched) {
				return;
			}
			constexpr std::uint8_t nops[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
			const void* src = want_patched ? static_cast<const void*>(nops)
			                               : static_cast<const void*>(g_reenable_original);
			if (REL::WriteSafe(g_reenable_addr, src, 5)) {
				g_reenable_patched = want_patched;
				REX::INFO("ReenableSurvival: {} patch at 0x{:X}",
					want_patched ? "applied" : "reverted", g_reenable_addr);
			} else {
				REX::ERROR("ReenableSurvival: WriteSafe failed at 0x{:X}", g_reenable_addr);
			}
		}

		void InstallReenableSurvival()
		{
			g_reenable_addr = FindReenableSignature();
			if (!g_reenable_addr) {
				REX::WARN("ReenableSurvival: signature not found; feature unavailable on this runtime");
				return;
			}
			std::memcpy(g_reenable_original,
				reinterpret_cast<const void*>(g_reenable_addr), 5);
			g_reenable_saved = true;
			REX::INFO("ReenableSurvival: sig found at 0x{:X}, original bytes cached",
				g_reenable_addr);
			ApplyReenablePatchState();
		}
	}

	void Install()
	{
		for (const auto& s : kGdlSites) {
			if (const auto* ra = SelectSite(s.og, s.ng, s.ae)) {
				AddHook<GDLHook>(ra->id, ra->offset, s.fn, s.label);
			}
		}
		for (const auto& s : kVoidPCSites) {
			if (const auto* ra = SelectSite(s.og, s.ng, s.ae)) {
				AddHook<VoidPCHook>(ra->id, ra->offset, s.fn, s.label);
			}
		}
		InstallReenableSurvival();
	}

	void RefreshRuntimePatches()
	{
		ApplyReenablePatchState();
	}
}
