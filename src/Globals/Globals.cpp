#include "PCH.h"

#include "Globals/Globals.h"

#include <cmath>
#include <string_view>
#include <unordered_set>

namespace Globals
{
	namespace
	{
		// Warn once per missing EDID so a user running DLL-only (no ESP)
		// doesn't flood the log on every MCM apply.
		std::unordered_set<std::string> g_warned_missing;

		RE::TESGlobal* FindByEditorID(std::string_view a_edid)
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) {
				return nullptr;
			}
			for (auto* glob : dh->GetFormArray<RE::TESGlobal>()) {
				if (!glob) continue;
				const char* edid = glob->GetFormEditorID();
				if (edid && a_edid == edid) {
					return glob;
				}
			}
			return nullptr;
		}
	}

	void Write(const char* a_editorID, float a_value)
	{
		if (!a_editorID || !*a_editorID) {
			return;
		}

		auto* glob = FindByEditorID(a_editorID);
		if (!glob) {
			if (g_warned_missing.insert(a_editorID).second) {
				REX::WARN("Globals: '{}' not found -- HouseRules.esp not installed?", a_editorID);
			}
			return;
		}

		if (std::abs(glob->value - a_value) > 1e-6f) {
			glob->value = a_value;
			REX::INFO("Globals: wrote '{}' = {} (FormID {:08X})",
				a_editorID, a_value, glob->formID);
		}
	}
}
