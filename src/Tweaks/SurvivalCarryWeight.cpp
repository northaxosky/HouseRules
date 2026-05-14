#include "PCH.h"

#include "Tweaks/SurvivalCarryWeight.h"

#include "Settings.h"

#include <cmath>
#include <cstdint>
#include <string_view>
#include <vector>

namespace Tweaks::SurvivalCarryWeight
{
	namespace
	{
		constexpr std::string_view kSpellEditorID{ "HC_ReduceCarryWeightAbility" };

		std::vector<float> g_snapshots;
		bool               g_warnedMissing = false;

		RE::SpellItem* FindSpell()
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) {
				return nullptr;
			}
			for (auto* spell : dh->GetFormArray<RE::SpellItem>()) {
				if (!spell) continue;
				const char* edid = spell->GetFormEditorID();
				if (edid && kSpellEditorID == edid) {
					return spell;
				}
			}
			return nullptr;
		}
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		auto* spell = FindSpell();
		if (!spell) {
			if (!g_warnedMissing) {
				REX::WARN("SurvivalCarryWeight: SPEL '{}' not found in form array; vanilla F4 record missing", kSpellEditorID);
				g_warnedMissing = true;
			}
			return;
		}

		const auto count = spell->listOfEffects.size();
		if (count == 0) {
			return;
		}

		// First-apply baseline snapshot.
		if (g_snapshots.empty()) {
			g_snapshots.reserve(count);
			for (std::uint32_t i = 0; i < count; ++i) {
				auto* eff = spell->listOfEffects[i];
				g_snapshots.push_back(eff ? eff->data.magnitude : 0.0f);
			}
			REX::INFO("SurvivalCarryWeight: snapshotted {} effect magnitude(s)", count);
		}

		const bool gate = MCM::Settings::Unlocks::bNoSurvivalCarryWeight.GetValue();

		std::uint32_t changed = 0;
		for (std::uint32_t i = 0; i < count && i < g_snapshots.size(); ++i) {
			auto* eff = spell->listOfEffects[i];
			if (!eff) continue;
			const float wanted = gate ? 0.0f : g_snapshots[i];
			if (std::abs(eff->data.magnitude - wanted) > 1e-4f) {
				eff->data.magnitude = wanted;
				++changed;
			}
		}
		if (changed > 0) {
			REX::INFO("SurvivalCarryWeight: gate={} wrote {} effect magnitude(s)", gate ? "ON" : "OFF", changed);
		}
	}

	void ResetSnapshots()
	{
		g_snapshots.clear();
		g_warnedMissing = false;
	}
}
