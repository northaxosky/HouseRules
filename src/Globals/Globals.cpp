#include "PCH.h"

#include "Globals/Globals.h"

#include <cmath>
#include <unordered_set>

namespace Globals
{
	namespace
	{
		// Warn-once per missing FormID so a misordered Fallout4.esm doesn't flood the log.
		std::unordered_set<std::uint32_t> g_warned_missing_formIDs;
	}

	void WriteByFormID(std::uint32_t a_formID, float a_value)
	{
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return;
		}

		for (auto* glob : dh->GetFormArray<RE::TESGlobal>()) {
			if (!glob || glob->formID != a_formID) {
				continue;
			}
			if (std::abs(glob->value - a_value) > 1e-6f) {
				glob->value = a_value;
				const char* edid = glob->GetFormEditorID();
				REX::INFO("Globals: wrote FormID {:08X} = {} (edid='{}')",
					a_formID, a_value, edid ? edid : "");
			}
			return;
		}

		if (g_warned_missing_formIDs.insert(a_formID).second) {
			REX::WARN("Globals: FormID {:08X} not found in TESGlobal array", a_formID);
		}
	}
}
