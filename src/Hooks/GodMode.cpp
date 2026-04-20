#include "PCH.h"

#include "Hooks/GodMode.h"

#include "Settings.h"

#include <memory>
#include <vector>

namespace Hooks::GodMode
{
	namespace
	{
		// The GodMode / ImmortalMode cheat flags live in .data, and the OG
		// AddressLib dump is functions-only, so resolve them per-runtime from
		// the module base. RVAs pulled by disassembling PC::IsGodMode on each
		// runtime (see findings/).
		bool* GodModePtr()
		{
			static bool* p = []() -> bool* {
				const auto base = REL::Module::GetSingleton()->base();
				using RT = REL::Module::Runtime;
				switch (REL::Module::GetRuntimeIndex()) {
					case RT::kOG: return reinterpret_cast<bool*>(base + 0x5EC7BB4);
					case RT::kNG: return reinterpret_cast<bool*>(base + 0x305E12D);
					case RT::kAE: return reinterpret_cast<bool*>(base + 0x32F598D);
					default:      return nullptr;
				}
			}();
			return p;
		}

		bool* ImmortalPtr()
		{
			static bool* p = []() -> bool* {
				const auto base = REL::Module::GetSingleton()->base();
				using RT = REL::Module::Runtime;
				switch (REL::Module::GetRuntimeIndex()) {
					case RT::kOG: return reinterpret_cast<bool*>(base + 0x5EC7BB5);
					case RT::kNG: return reinterpret_cast<bool*>(base + 0x305E12E);
					case RT::kAE: return reinterpret_cast<bool*>(base + 0x32F598E);
					default:      return nullptr;
				}
			}();
			return p;
		}

		bool ToggleAllowsCheatInSurvival()
		{
			return MCM::Settings::General::bEnabled.GetValue() &&
				MCM::Settings::Unlocks::bGodMode.GetValue();
		}

		// Common gate: cheat is set, and either we're not in Survival (vanilla
		// allows it outside Survival) or the MCM toggle is overriding Survival.
		bool AllowCheat(bool* a_flag)
		{
			if (!a_flag || !*a_flag) {
				return false;
			}
			if (ToggleAllowsCheatInSurvival()) {
				return true;
			}
			auto* pc = RE::PlayerCharacter::GetSingleton();
			if (!pc) {
				return true;
			}
			return pc->GetDifficultyLevel() != RE::DifficultyLevel::kTrueSurvival;
		}

		bool AllowGodMode()  { return AllowCheat(GodModePtr()); }
		bool AllowImmortal() { return AllowCheat(ImmortalPtr()); }

		// --- IsGodMode / IsImmortal wholesale replacements ---------------------

		bool IsGodMode_Impl()  { return AllowGodMode(); }
		bool IsImmortal_Impl() { return AllowImmortal(); }

		// --- MagicTarget::IsInvulnerable (vtable slot 4 via VTABLE[8]) ---------
		// Hook is on PlayerCharacter's vtable, so the caller is always the
		// player — we skip RE::fallout_cast (which needs RTTI::MagicTarget,
		// an ID missing from OG's AddressLib) and read PC directly from its
		// singleton. Keeps the two PC-specific short-circuits vanilla's
		// IsInvulnerable cares about: ghost mode and teleport-door cooldown.

		bool IsInvulnerable_Impl(RE::MagicTarget* /*a_this*/)
		{
			if (AllowGodMode()) {
				return true;
			}
			auto* pc = RE::PlayerCharacter::GetSingleton();
			if (pc && (pc->GetGhost() ||
					pc->usingTeleportDoorTimeStamp.timeStamp - RE::AITimer::fTimer() > 0.0f)) {
				return true;
			}
			return false;
		}

		// --- Actor::CheckClampDamageModifier original (chained from PC hook) --
		// OG ID 124191 / NG+AE 2231029 (Actor base, not PC override).

		float Actor_CheckClampDamageModifier(RE::Actor* a_this,
			const RE::ActorValueInfo& a_info, float a_delta)
		{
			using func_t = decltype(&Actor_CheckClampDamageModifier);
			static REL::Relocation<func_t> func{ REL::ID({ 124191, 2231029 }) };
			return func(a_this, a_info, a_delta);
		}

		// --- PC::CheckClampDamageModifier (vtable slot 0x131 via VTABLE[0]) ---
		// God mode clamps every negative delta: vanilla tgm is total invincibility
		// (including limbs, which use their own ActorValues without the
		// kGodModeNoDamage flag). Immortal stays surgical — formID 0x2D4 is the
		// Health AV, and tim only blocks what would be the killing blow so you
		// can still take limb damage.

		float CheckClampDamageModifier_Impl(RE::PlayerCharacter* a_this,
			const RE::ActorValueInfo& a_info, float a_delta)
		{
			if (AllowImmortal() && a_delta < 0.0f && a_info.formID == 0x000002D4 &&
				a_this->GetActorValue(a_info) + a_delta <= 0.0f) {
				return Actor_CheckClampDamageModifier(a_this, a_info, 0.0f);
			}
			if (AllowGodMode() && a_delta < 0.0f) {
				return Actor_CheckClampDamageModifier(a_this, a_info, 0.0f);
			}
			return Actor_CheckClampDamageModifier(a_this, a_info, a_delta);
		}

		std::vector<std::unique_ptr<REL::HookObject>> g_hooks;
	}

	void Install()
	{
		// Sanity check: these addresses should resolve, and the flags should
		// read as 0 at plugin load (no tgm/tim typed yet). Any non-zero value
		// here means either a wrong offset or a game-memory pre-init state
		// that needs resetting.
		if (auto* g = GodModePtr()) {
			REX::INFO("GodMode: godmode flag @ {} = {}", static_cast<const void*>(g), *g);
		}
		if (auto* i = ImmortalPtr()) {
			REX::INFO("GodMode: immortal flag @ {} = {}", static_cast<const void*>(i), *i);
		}

		// Wholesale replacements. The OG IsGodMode and IsImmortal are each
		// 0x40 bytes of real code; we write a 14-byte JMP14 + 0x31 bytes of
		// INT3 padding, which fits comfortably in 0x3F.
		{
			static REL::Relocation target{ RE::ID::PlayerCharacter::IsGodMode };
			target.replace_func(0x3F, &IsGodMode_Impl);
			REX::INFO("GodMode: replaced PC::IsGodMode at 0x{:X}", target.address());
		}
		{
			static REL::Relocation target{ RE::ID::PlayerCharacter::IsImmortal };
			target.replace_func(0x3F, &IsImmortal_Impl);
			REX::INFO("GodMode: replaced PC::IsImmortal at 0x{:X}", target.address());
		}

		// VTable hooks — PlayerCharacter::VTABLE IDs resolve on every runtime
		// and slot indices are stable across versions (class interfaces don't
		// drift).
		{
			auto h = std::make_unique<REL::HookVFT<bool(RE::MagicTarget*)>>(
				RE::PlayerCharacter::VTABLE[8], 0x4, &IsInvulnerable_Impl);
			const bool enabled = h->Enable();
			REX::INFO("GodMode: VFT hook MagicTarget::IsInvulnerable enabled={}", enabled);
			g_hooks.push_back(std::move(h));
		}
		{
			auto h = std::make_unique<REL::HookVFT<float(RE::PlayerCharacter*, const RE::ActorValueInfo&, float)>>(
				RE::PlayerCharacter::VTABLE[0], 0x131, &CheckClampDamageModifier_Impl);
			const bool enabled = h->Enable();
			REX::INFO("GodMode: VFT hook PC::CheckClampDamageModifier enabled={}", enabled);
			g_hooks.push_back(std::move(h));
		}

		// Start clean in case some other plugin or a prior session left the
		// engine's tgm/tim bool set.
		RefreshRuntimePatches();
	}

	// Called on pause-menu close after MCM settings reload. When the user
	// disables bGodMode, clear the engine's tgm/tim flags — otherwise a
	// previously-typed tgm would silently re-activate the moment the player
	// leaves Survival (and the user would never connect the dots).
	void RefreshRuntimePatches()
	{
		const bool enabled = MCM::Settings::General::bEnabled.GetValue() &&
			MCM::Settings::Unlocks::bGodMode.GetValue();
		if (enabled) {
			return;  // user controls tgm/tim themselves
		}
		if (auto* g = GodModePtr()) *g = false;
		if (auto* i = ImmortalPtr()) *i = false;
	}
}
