#include "PCH.h"

#include "Hooks/GodMode.h"

#include "Settings.h"

// Byte-patch approach, mirroring what Unlimited Survival Mode does. Vanilla
// PC::IsGodMode and PC::IsImmortal each contain a `jl short` at function
// offset +0x25 (identical across OG/NG/AE) which blocks tgm/tim in Survival.
// We flip that byte (0x7C → 0xEB) to turn the conditional jump into an
// unconditional one when bGodMode is ON, and restore the original when OFF.
//
// Why byte-patch over replace_func: leaves vanilla IsGodMode's return-value
// logic intact, so the console's ToggleGodModeFunction reads a faithful
// answer and prints "god mode on/off" correctly. The replace_func variant
// we tried first created a "says off but I'm actually invincible" glitch
// because our gating made IsGodMode lie about state.
namespace Hooks::GodMode
{
	namespace
	{
		constexpr std::size_t  kJlOffset = 0x25;  // offset of 'jl short' inside each function
		constexpr std::uint8_t kJlByte   = 0x7C;  // original: jl short
		constexpr std::uint8_t kJmpByte  = 0xEB;  // patched:  jmp short (same disp, always taken)

		std::uintptr_t g_god_patch_addr = 0;
		std::uintptr_t g_imm_patch_addr = 0;
		bool           g_patched        = false;

		void ApplyPatchState()
		{
			if (!g_god_patch_addr || !g_imm_patch_addr) {
				return;
			}
			const bool want = MCM::Settings::General::bEnabled.GetValue() &&
				MCM::Settings::Unlocks::bGodMode.GetValue();
			if (want == g_patched) {
				return;
			}
			const std::uint8_t byte = want ? kJmpByte : kJlByte;
			const bool ok_god = REL::WriteSafe(g_god_patch_addr, &byte, 1);
			const bool ok_imm = REL::WriteSafe(g_imm_patch_addr, &byte, 1);
			if (ok_god && ok_imm) {
				g_patched = want;
				REX::INFO("GodMode: {} patches at 0x{:X} / 0x{:X}",
					want ? "applied" : "reverted", g_god_patch_addr, g_imm_patch_addr);
			} else {
				REX::ERROR("GodMode: WriteSafe failed (god_ok={} imm_ok={})", ok_god, ok_imm);
			}
		}
	}

	void Install()
	{
		REL::Relocation<std::uintptr_t> god_target{ RE::ID::PlayerCharacter::IsGodMode };
		REL::Relocation<std::uintptr_t> imm_target{ RE::ID::PlayerCharacter::IsImmortal };
		g_god_patch_addr = god_target.address() + kJlOffset;
		g_imm_patch_addr = imm_target.address() + kJlOffset;

		// Sanity: the byte we intend to patch should be the unpatched 'jl short'.
		// If it isn't, either a different plugin already wrote here, or the
		// offset is wrong on this runtime.
		const auto cur_god = *reinterpret_cast<std::uint8_t*>(g_god_patch_addr);
		const auto cur_imm = *reinterpret_cast<std::uint8_t*>(g_imm_patch_addr);
		REX::INFO("GodMode: IsGodMode+0x{:X} @ 0x{:X} = 0x{:02X}, IsImmortal+0x{:X} @ 0x{:X} = 0x{:02X}",
			kJlOffset, g_god_patch_addr, cur_god,
			kJlOffset, g_imm_patch_addr, cur_imm);
		if (cur_god != kJlByte || cur_imm != kJlByte) {
			REX::WARN("GodMode: unexpected byte at patch site — another plugin may already be patching here");
		}

		ApplyPatchState();
	}

	void RefreshRuntimePatches()
	{
		ApplyPatchState();
	}
}
