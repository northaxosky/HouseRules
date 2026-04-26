#include "PCH.h"

#include "Tweaks/Difficulty.h"

#include "Settings.h"

#include <cmath>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Tweaks::Difficulty
{
	namespace
	{
		struct Target
		{
			const char* gmst;
			REX::INI::F32<>* mult;
		};

		struct Snapshot
		{
			std::string_view gmst;
			float            baseline;
		};

		std::vector<Snapshot> g_snapshots;
		bool                  g_snapshotted = false;
		bool                  g_warned_missing_collection = false;
		// Names that have already been logged as missing from the GameSettingCollection.
		std::vector<std::string_view> g_warned_missing;

		// Last-applied multipliers, keyed by GMST name. Used to detect a
		// change in the multiplier vector so we can keep summary logging quiet.
		std::unordered_map<std::string_view, float> g_last_mults;

		bool MaterialDelta(float a_prev, float a_now)
		{
			const float scale = std::max(1.0f, std::max(std::abs(a_prev), std::abs(a_now)));
			return std::abs(a_prev - a_now) > 1e-4f * scale;
		}

		// Single source of truth for the GMST <-> MCM-multiplier mapping.
		// Building this lazily (function-local static) so we don't construct
		// the array before the MCM::Settings::Difficulty statics exist.
		const std::vector<Target>& Targets()
		{
			static const std::vector<Target> kTargets = {
				// Incoming damage (damage taken by player)
				{ "fDiffMultHPToPCVE",  &MCM::Settings::Difficulty::fIncomingDamageVeryEasy  },
				{ "fDiffMultHPToPCE",   &MCM::Settings::Difficulty::fIncomingDamageEasy      },
				{ "fDiffMultHPToPCN",   &MCM::Settings::Difficulty::fIncomingDamageNormal    },
				{ "fDiffMultHPToPCH",   &MCM::Settings::Difficulty::fIncomingDamageHard      },
				{ "fDiffMultHPToPCVH",  &MCM::Settings::Difficulty::fIncomingDamageVeryHard  },
				{ "fDiffMultHPToPCTSV", &MCM::Settings::Difficulty::fIncomingDamageSurvival  },

				// Outgoing damage (damage done by player)
				{ "fDiffMultHPByPCVE",  &MCM::Settings::Difficulty::fOutgoingDamageVeryEasy  },
				{ "fDiffMultHPByPCE",   &MCM::Settings::Difficulty::fOutgoingDamageEasy      },
				{ "fDiffMultHPByPCN",   &MCM::Settings::Difficulty::fOutgoingDamageNormal    },
				{ "fDiffMultHPByPCH",   &MCM::Settings::Difficulty::fOutgoingDamageHard      },
				{ "fDiffMultHPByPCVH",  &MCM::Settings::Difficulty::fOutgoingDamageVeryHard  },
				{ "fDiffMultHPByPCTSV", &MCM::Settings::Difficulty::fOutgoingDamageSurvival  },

				// XP progression
				{ "fXPModBase",         &MCM::Settings::Difficulty::fXPBase                  },
				{ "fXPModMult",         &MCM::Settings::Difficulty::fXPMult                  },

				// Legendary chance
				{ "fDiffMultLegendaryChance_VE",  &MCM::Settings::Difficulty::fLegendaryChanceVeryEasy  },
				{ "fDiffMultLegendaryChance_E",   &MCM::Settings::Difficulty::fLegendaryChanceEasy      },
				{ "fDiffMultLegendaryChance_N",   &MCM::Settings::Difficulty::fLegendaryChanceNormal    },
				{ "fDiffMultLegendaryChance_H",   &MCM::Settings::Difficulty::fLegendaryChanceHard      },
				{ "fDiffMultLegendaryChance_VH",  &MCM::Settings::Difficulty::fLegendaryChanceVeryHard  },
				{ "fDiffMultLegendaryChance_TSV", &MCM::Settings::Difficulty::fLegendaryChanceSurvival  },

				// Legendary rarity
				{ "fDiffMultLegendaryRarity_VE",  &MCM::Settings::Difficulty::fLegendaryRarityVeryEasy  },
				{ "fDiffMultLegendaryRarity_E",   &MCM::Settings::Difficulty::fLegendaryRarityEasy      },
				{ "fDiffMultLegendaryRarity_N",   &MCM::Settings::Difficulty::fLegendaryRarityNormal    },
				{ "fDiffMultLegendaryRarity_H",   &MCM::Settings::Difficulty::fLegendaryRarityHard      },
				{ "fDiffMultLegendaryRarity_VH",  &MCM::Settings::Difficulty::fLegendaryRarityVeryHard  },
				{ "fDiffMultLegendaryRarity_TSV", &MCM::Settings::Difficulty::fLegendaryRaritySurvival },
			};
			return kTargets;
		}

		RE::Setting* LookupSetting(const char* a_name)
		{
			auto* coll = RE::GameSettingCollection::GetSingleton();
			if (!coll) {
				if (!g_warned_missing_collection) {
					REX::WARN("Difficulty: GameSettingCollection unavailable; skipping");
					g_warned_missing_collection = true;
				}
				return nullptr;
			}
			return coll->GetSetting(a_name);
		}

		bool AlreadyWarnedMissing(std::string_view a_name)
		{
			for (auto n : g_warned_missing) {
				if (n == a_name) return true;
			}
			return false;
		}

		void SnapshotAll()
		{
			const auto& targets = Targets();
			g_snapshots.reserve(targets.size());
			for (const auto& t : targets) {
				auto* s = LookupSetting(t.gmst);
				if (!s || s->GetType() != RE::Setting::SETTING_TYPE::kFloat) {
					if (!AlreadyWarnedMissing(t.gmst)) {
						REX::WARN("Difficulty: GMST '{}' missing or non-float; skipping", t.gmst);
						g_warned_missing.push_back(t.gmst);
					}
					continue;
				}
				g_snapshots.push_back({ t.gmst, s->GetFloat() });
			}
			REX::INFO("Difficulty: snapshotted {} GMST baselines", g_snapshots.size());
		}

		const Snapshot* FindSnapshot(std::string_view a_name)
		{
			for (const auto& s : g_snapshots) {
				if (s.gmst == a_name) return std::addressof(s);
			}
			return nullptr;
		}
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		// Same safe-context guarantee as Magnitudes: only call from the
		// pause-menu close / loading-menu close hooks where the engine has
		// finished its lazy init of subsystems.
		if (!g_snapshotted) {
			SnapshotAll();
			g_snapshotted = true;
		}

		const bool trace = MCM::Settings::Diagnostic::bDifficultyTrace.GetValue();

		// Detect any change in the multiplier vector to decide whether to
		// emit the once-per-edit summary line.
		bool any_changed = false;

		for (const auto& t : Targets()) {
			const auto* snap = FindSnapshot(t.gmst);
			if (!snap) {
				continue;
			}
			auto* s = LookupSetting(t.gmst);
			if (!s || s->GetType() != RE::Setting::SETTING_TYPE::kFloat) {
				continue;
			}

			const float mult = t.mult->GetValue();
			const float wanted = snap->baseline * mult;
			s->SetFloat(wanted);
			const float readback = s->GetFloat();

			auto it = g_last_mults.find(t.gmst);
			if (it == g_last_mults.end() || MaterialDelta(it->second, mult)) {
				any_changed = true;
				g_last_mults[t.gmst] = mult;
			}

			if (trace) {
				REX::INFO("  GMST {}: baseline={} mult={} wrote={} readback={}",
					t.gmst, snap->baseline, mult, wanted, readback);
			}
		}

		if (any_changed || trace) {
			REX::INFO("Difficulty: applied multipliers (in_VE={} in_E={} in_N={} in_H={} in_VH={} in_TSV={} "
				"out_VE={} out_E={} out_N={} out_H={} out_VH={} out_TSV={} "
				"xp_base={} xp_mult={} "
				"lchance_VE={} lchance_E={} lchance_N={} lchance_H={} lchance_VH={} lchance_TSV={} "
				"lrare_VE={} lrare_E={} lrare_N={} lrare_H={} lrare_VH={} lrare_TSV={})",
				MCM::Settings::Difficulty::fIncomingDamageVeryEasy.GetValue(),
				MCM::Settings::Difficulty::fIncomingDamageEasy.GetValue(),
				MCM::Settings::Difficulty::fIncomingDamageNormal.GetValue(),
				MCM::Settings::Difficulty::fIncomingDamageHard.GetValue(),
				MCM::Settings::Difficulty::fIncomingDamageVeryHard.GetValue(),
				MCM::Settings::Difficulty::fIncomingDamageSurvival.GetValue(),
				MCM::Settings::Difficulty::fOutgoingDamageVeryEasy.GetValue(),
				MCM::Settings::Difficulty::fOutgoingDamageEasy.GetValue(),
				MCM::Settings::Difficulty::fOutgoingDamageNormal.GetValue(),
				MCM::Settings::Difficulty::fOutgoingDamageHard.GetValue(),
				MCM::Settings::Difficulty::fOutgoingDamageVeryHard.GetValue(),
				MCM::Settings::Difficulty::fOutgoingDamageSurvival.GetValue(),
				MCM::Settings::Difficulty::fXPBase.GetValue(),
				MCM::Settings::Difficulty::fXPMult.GetValue(),
				MCM::Settings::Difficulty::fLegendaryChanceVeryEasy.GetValue(),
				MCM::Settings::Difficulty::fLegendaryChanceEasy.GetValue(),
				MCM::Settings::Difficulty::fLegendaryChanceNormal.GetValue(),
				MCM::Settings::Difficulty::fLegendaryChanceHard.GetValue(),
				MCM::Settings::Difficulty::fLegendaryChanceVeryHard.GetValue(),
				MCM::Settings::Difficulty::fLegendaryChanceSurvival.GetValue(),
				MCM::Settings::Difficulty::fLegendaryRarityVeryEasy.GetValue(),
				MCM::Settings::Difficulty::fLegendaryRarityEasy.GetValue(),
				MCM::Settings::Difficulty::fLegendaryRarityNormal.GetValue(),
				MCM::Settings::Difficulty::fLegendaryRarityHard.GetValue(),
				MCM::Settings::Difficulty::fLegendaryRarityVeryHard.GetValue(),
				MCM::Settings::Difficulty::fLegendaryRaritySurvival.GetValue());
		}
	}
}
