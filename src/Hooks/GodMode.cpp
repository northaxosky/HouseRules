#include "PCH.h"

#include "Hooks/GodMode.h"

#include "Settings.h"

// Byte-patch approach, mirroring what Unlimited Survival Mode does. Vanilla
// PC::IsGodMode and PC::IsImmortal share the same survival-gate:
//
//   +0x22  cmp eax, 5          ; eax = difficulty
//   +0x25  jl  +0x35           ; low-diff path (fallthrough returns 1)
//   +0x27  xor eax, eax        ; eax = 0
//   +0x29  cmp [survival_flag], al
//   +0x2F  setne al            ; al = 1 if flag is set
//   +0x32  add eax, 5          ; eax = 5 or 6  <-- patch target here
//   +0x35  cmp eax, 6
//   +0x38  je  return_0        ; god mode blocked when eax == 6
//
// We flip the `5` immediate of `add eax, 5` (at function offset +0x34) to
// `0`, so the high-diff path produces eax = 0 or 1 — never 6. Final `je`
// never fires, god mode always returns 1 when the engine's flag is set,
// regardless of Survival. Low-difficulty (diff < 5) takes the `jl` skip
// and isn't affected. Identical offset in OG/NG/AE.
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
		constexpr std::size_t kSequenceOffset = 0x32;
		constexpr std::size_t  kImmOffset = 0x34;  // offset of the 'add eax, 5' immediate
		constexpr std::uint8_t kOrigByte  = 0x05;  // vanilla: adds 5 (path can produce eax == 6)
		constexpr std::uint8_t kPatchByte = 0x00;  // patched: adds 0 (eax never 6 → never blocked)
		constexpr std::array<std::uint8_t, 6> kExpectedVanilla{ 0x83, 0xC0, kOrigByte, 0x83, 0xF8, 0x06 };
		constexpr std::array<std::uint8_t, 6> kExpectedPatched{ 0x83, 0xC0, kPatchByte, 0x83, 0xF8, 0x06 };

		std::uintptr_t g_god_patch_addr = 0;
		std::uintptr_t g_imm_patch_addr = 0;
		bool           g_patched        = false;
		bool           g_validated      = false;

		std::array<std::uint8_t, 6> ReadPatchSequence(std::uintptr_t a_patch_addr)
		{
			std::array<std::uint8_t, 6> bytes{};
			std::memcpy(bytes.data(),
				reinterpret_cast<const void*>(a_patch_addr - (kImmOffset - kSequenceOffset)),
				bytes.size());
			return bytes;
		}

		bool SequenceMatches(const std::array<std::uint8_t, 6>& a_bytes,
			const std::array<std::uint8_t, 6>& a_expected)
		{
			return a_bytes == a_expected;
		}

		bool ValidatePatchSite(std::uintptr_t a_patch_addr, const char* a_label)
		{
			const auto bytes = ReadPatchSequence(a_patch_addr);
			if (SequenceMatches(bytes, kExpectedVanilla) || SequenceMatches(bytes, kExpectedPatched)) {
				REX::INFO("{}: verified anchor bytes {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} at 0x{:X}",
					a_label, bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5],
					a_patch_addr - (kImmOffset - kSequenceOffset));
				return true;
			}

			REX::ERROR("{}: unexpected bytes {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} at 0x{:X}; refusing patch",
				a_label, bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5],
				a_patch_addr - (kImmOffset - kSequenceOffset));
			return false;
		}

		void ApplyPatchState()
		{
			if (!g_validated || !g_god_patch_addr || !g_imm_patch_addr) {
				return;
			}
			const bool want = MCM::Settings::General::bEnabled.GetValue() &&
				MCM::Settings::Unlocks::bGodMode.GetValue();
			if (want == g_patched) {
				return;
			}
			const std::uint8_t byte = want ? kPatchByte : kOrigByte;
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
		g_god_patch_addr = god_target.address() + kImmOffset;
		g_imm_patch_addr = imm_target.address() + kImmOffset;
		g_validated = ValidatePatchSite(g_god_patch_addr, "GodMode / IsGodMode") &&
			ValidatePatchSite(g_imm_patch_addr, "GodMode / IsImmortal");
		if (!g_validated) {
			return;
		}

		const auto cur_god = *reinterpret_cast<std::uint8_t*>(g_god_patch_addr);
		const auto cur_imm = *reinterpret_cast<std::uint8_t*>(g_imm_patch_addr);
		g_patched = cur_god == kPatchByte && cur_imm == kPatchByte;
		REX::INFO("GodMode: IsGodMode+0x{:X} @ 0x{:X} = 0x{:02X}, IsImmortal+0x{:X} @ 0x{:X} = 0x{:02X}",
			kImmOffset, g_god_patch_addr, cur_god,
			kImmOffset, g_imm_patch_addr, cur_imm);
		if (cur_god != kOrigByte || cur_imm != kOrigByte) {
			REX::WARN("GodMode: non-vanilla startup state at patch site; toggles will now own both bytes");
		}

		ApplyPatchState();
	}

	void RefreshRuntimePatches()
	{
		ApplyPatchState();
	}
}
