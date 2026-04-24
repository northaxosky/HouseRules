#include "PCH.h"

#include "Tweaks/Magnitudes.h"

#include "Settings.h"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace Tweaks::Magnitudes
{
	namespace
	{
		using EffectScaler = std::function<float(std::uint32_t /*effect_setting_form_id*/)>;

		// Vanilla FormIDs (Fallout4.esm). Provenance: findings/vanilla-survival-stages.md.
		// AlchemyItem consumables
		constexpr std::uint32_t kAlchStimpak = 0x00023736;
		constexpr std::uint32_t kAlchRadAway = 0x00023742;
		constexpr std::uint32_t kAlchRadX    = 0x00024057;

		// Hunger stages (Peckish -> Starving)
		constexpr std::array<std::uint32_t, 5> kAlchHungerStages = {
			0x00000856, 0x00000857, 0x00000858, 0x00000859, 0x0000085A
		};
		// Thirst stages (Thirsty -> SeverelyDehydrated)
		constexpr std::array<std::uint32_t, 5> kAlchThirstStages = {
			0x00000871, 0x00000872, 0x00000873, 0x00000874, 0x00000875
		};
		// Sleep stages (Tired -> Incapacitated). Note: not contiguous FormIDs.
		constexpr std::array<std::uint32_t, 5> kAlchSleepStages = {
			0x00000818, 0x0000081A, 0x0000081C, 0x0000081E, 0x00000820
		};

		// Vanilla's HC_ReduceCarryWeightAbility is a SPEL with an effect whose MGEF
		// has primaryAV == CarryWeight. We discover it at runtime rather than
		// hardcoding a FormID (previous FormID 0x00249A37 was wrong and the lookup
		// failed). This approach also catches any mod-added abilities that reduce
		// CarryWeight via the same archetype.

		// MGEFs (EffectSetting records) recovered from the archived survival dumps.
		constexpr std::uint32_t kEffectRestoreHealthFood          = 0x0000397E;
		constexpr std::uint32_t kEffectRestoreHealthStimpak       = 0x0021DDB8;
		constexpr std::uint32_t kEffectRestoreConditionHead       = 0x0005C529;
		constexpr std::uint32_t kEffectRestoreConditionLeftArm    = 0x0005C52C;
		constexpr std::uint32_t kEffectRestoreConditionLeftLeg    = 0x0005C52D;
		constexpr std::uint32_t kEffectRestoreConditionRightArm   = 0x0005C52A;
		constexpr std::uint32_t kEffectRestoreConditionRightLeg   = 0x0005C52B;
		constexpr std::uint32_t kEffectRestoreConditionTorso      = 0x0005C52E;
		constexpr std::uint32_t kEffectRestoreRadsChem            = 0x00023738;
		constexpr std::uint32_t kEffectFortifyResistRadsRadX      = 0x00246B11;

		struct EffectItemSnapshot
		{
			std::uint32_t effectSettingFormID;
			float         baseline;
		};

		struct AlchSnapshot
		{
			std::uint32_t                 formID;
			std::vector<EffectItemSnapshot> effects;
		};

		struct MatchedAlchemyEffectSnapshot
		{
			std::uint32_t ownerFormID;
			std::uint32_t effectIndex;
			float         baseline;
		};

		struct SpellEffectSnapshot
		{
			std::uint32_t spellFormID;
			std::uint32_t effectIndex;
			std::uint32_t effectSettingFormID;
			float         baseline;
		};

		std::vector<AlchSnapshot>               g_alch_snapshots;
		std::vector<MatchedAlchemyEffectSnapshot> g_food_heal_snapshots;
		std::vector<SpellEffectSnapshot>        g_carry_weight_snapshots;
		bool                                    g_snapshotted = false;

		void SnapshotCarryWeightSpells();
		void ApplyCarryWeightSpells(bool a_unlock);

		// Intentionally not using TESDataHandler::LookupForm — that path walks
		// through the engine's form-by-ID hash in a way that triggers a lazy
		// BSStaticTriShapeDB preload on OG, null-derefing at Fallout4.exe+0x405A
		// whenever it runs before meshes are warm. Linear-scanning the typed
		// form arrays (same path our Diagnostics dump uses successfully) stays
		// clear of that trap. ~hundreds of entries per type — negligible on the
		// once-per-save-load cadence this runs at.
		RE::AlchemyItem* LookupAlch(std::uint32_t a_formID)
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) return nullptr;
			for (auto* alch : dh->GetFormArray<RE::AlchemyItem>()) {
				if (alch && alch->formID == a_formID) return alch;
			}
			return nullptr;
		}

		void SnapshotAlch(std::uint32_t a_formID)
		{
			auto* alch = LookupAlch(a_formID);
			if (!alch) {
				REX::WARN("Magnitudes: ALCH {:08X} not found", a_formID);
				return;
			}
			AlchSnapshot snap{ a_formID, {} };
			snap.effects.reserve(alch->listOfEffects.size());
			for (auto* eff : alch->listOfEffects) {
				snap.effects.push_back({
					eff && eff->effectSetting ? eff->effectSetting->formID : 0,
					eff ? eff->data.magnitude : 0.0f
				});
			}
			g_alch_snapshots.push_back(std::move(snap));
		}

		void SnapshotAlchemyEffectsBySetting(std::uint32_t a_effectSettingFormID)
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) {
				return;
			}

			for (auto* alch : dh->GetFormArray<RE::AlchemyItem>()) {
				if (!alch) {
					continue;
				}
				for (std::uint32_t i = 0; i < alch->listOfEffects.size(); ++i) {
					const auto* eff = alch->listOfEffects[i];
					if (!eff || !eff->effectSetting || eff->effectSetting->formID != a_effectSettingFormID) {
						continue;
					}
					g_food_heal_snapshots.push_back({
						alch->formID,
						i,
						eff->data.magnitude
					});
				}
			}
		}

		void SnapshotAll()
		{
			SnapshotAlch(kAlchStimpak);
			SnapshotAlch(kAlchRadAway);
			SnapshotAlch(kAlchRadX);
			for (auto id : kAlchHungerStages) SnapshotAlch(id);
			for (auto id : kAlchThirstStages) SnapshotAlch(id);
			for (auto id : kAlchSleepStages)  SnapshotAlch(id);
			SnapshotAlchemyEffectsBySetting(kEffectRestoreHealthFood);
			SnapshotCarryWeightSpells();
			REX::INFO("Magnitudes: snapshotted {} ALCH baselines and {} RestoreHealthFood entries",
				g_alch_snapshots.size(), g_food_heal_snapshots.size());
		}

		const AlchSnapshot* FindAlchSnapshot(std::uint32_t a_formID)
		{
			for (const auto& s : g_alch_snapshots) {
				if (s.formID == a_formID) {
					return std::addressof(s);
				}
			}
			return nullptr;
		}

		void ApplyAlch(std::uint32_t a_formID, const EffectScaler& a_scaler)
		{
			const auto* snap = FindAlchSnapshot(a_formID);
			if (!snap) return;
			auto* alch = LookupAlch(a_formID);
			if (!alch) return;
			const auto count = std::min<std::size_t>(alch->listOfEffects.size(), snap->effects.size());
			for (std::size_t i = 0; i < count; ++i) {
				auto* eff = alch->listOfEffects[static_cast<std::uint32_t>(i)];
				if (!eff) continue;
				const auto& entry = snap->effects[i];
				const auto  currentSettingFormID = eff->effectSetting ? eff->effectSetting->formID : 0;
				if (entry.effectSettingFormID != currentSettingFormID) {
					REX::WARN("Magnitudes: ALCH {:08X}[{}] effect changed from {:08X} to {:08X}; skipping",
						a_formID,
						i,
						entry.effectSettingFormID,
						currentSettingFormID);
					continue;
				}
				const float wanted = entry.baseline * a_scaler(currentSettingFormID);
				eff->data.magnitude = wanted;
				const auto  readback = eff->data.magnitude;
				const int   archetype = eff->effectSetting
					  ? static_cast<int>(eff->effectSetting->data.archetype.get())
					  : -1;
				const char* mgef_edid = eff->effectSetting ? eff->effectSetting->GetFormEditorID() : "";
				REX::INFO("  ALCH {:08X}[{}]: wrote={} readback={} archetype={} mgef='{}'",
					a_formID, i, wanted, readback, archetype, mgef_edid ? mgef_edid : "");
			}
		}

		void ApplyFoodHeal(float a_mult)
		{
			for (const auto& snap : g_food_heal_snapshots) {
				auto* alch = LookupAlch(snap.ownerFormID);
				if (!alch || snap.effectIndex >= alch->listOfEffects.size()) {
					continue;
				}

				auto* eff = alch->listOfEffects[snap.effectIndex];
				if (!eff || !eff->effectSetting || eff->effectSetting->formID != kEffectRestoreHealthFood) {
					REX::WARN("Magnitudes: food-heal entry {:08X}[{}] changed effect identity; skipping",
						snap.ownerFormID,
						snap.effectIndex);
					continue;
				}

				const float wanted = snap.baseline * a_mult;
				eff->data.magnitude = wanted;
			}
		}
		RE::SpellItem* LookupSpell(std::uint32_t a_formID)
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) return nullptr;
			for (auto* spel : dh->GetFormArray<RE::SpellItem>()) {
				if (spel && spel->formID == a_formID) return spel;
			}
			return nullptr;
		}

		bool EffectTargetsCarryWeight(const RE::EffectItem* a_eff)
		{
			if (!a_eff || !a_eff->effectSetting) return false;
			const auto* mgef = a_eff->effectSetting;
			const auto* av = mgef->data.primaryAV;
			if (!av) return false;
			const char* edid = av->GetFormEditorID();
			return edid && std::string_view{ edid } == "CarryWeight";
		}

		void SnapshotCarryWeightSpells()
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) {
				REX::WARN("Magnitudes: TESDataHandler unavailable; carry-weight unlock inert");
				return;
			}
			for (auto* spel : dh->GetFormArray<RE::SpellItem>()) {
				if (!spel) continue;
				for (std::uint32_t i = 0; i < spel->listOfEffects.size(); ++i) {
					const auto* eff = spel->listOfEffects[i];
					if (!EffectTargetsCarryWeight(eff)) continue;
					const auto* mgef = eff->effectSetting;
					const bool  detrimental = mgef
						? (mgef->data.flags.underlying() & static_cast<std::uint32_t>(RE::EffectSetting::EffectSettingData::Flag::kDetrimental)) != 0
						: false;
					const float magnitude = eff->data.magnitude;
					// Only neutralize true reducers: effects with negative magnitude OR
					// MGEFs flagged kDetrimental (which subtract even with positive magnitude).
					// Positive bonuses (Strong Back, power armor, workshop, etc.) are left alone.
					const bool reducer = (magnitude < 0.0f) || detrimental;
					const char* spelEdid = spel->GetFormEditorID();
					const char* mgefEdid = mgef ? mgef->GetFormEditorID() : "";
					REX::INFO("Magnitudes: CarryWeight SPEL {:08X} edid='{}'[{}] MGEF {:08X} edid='{}' magnitude={} detrimental={} -> {}",
						spel->formID,
						spelEdid ? spelEdid : "",
						i,
						mgef ? mgef->formID : 0,
						mgefEdid ? mgefEdid : "",
						magnitude,
						detrimental,
						reducer ? "neutralize" : "skip");
					if (!reducer) continue;
					g_carry_weight_snapshots.push_back({
						spel->formID,
						i,
						mgef ? mgef->formID : 0,
						magnitude
					});
				}
			}
			if (g_carry_weight_snapshots.empty()) {
				REX::WARN("Magnitudes: no carry-weight reducers found (checked negative magnitude + kDetrimental flag); unlock will be inert");
			} else {
				REX::INFO("Magnitudes: snapshotted {} CarryWeight reducer(s)",
					g_carry_weight_snapshots.size());
			}
		}

		void ApplyCarryWeightSpells(bool a_unlock)
		{
			if (g_carry_weight_snapshots.empty()) {
				return;
			}
			std::size_t applied = 0;
			for (const auto& snap : g_carry_weight_snapshots) {
				auto* spel = LookupSpell(snap.spellFormID);
				if (!spel || snap.effectIndex >= spel->listOfEffects.size()) continue;
				auto* eff = spel->listOfEffects[snap.effectIndex];
				if (!eff) continue;
				const auto currentSettingFormID = eff->effectSetting ? eff->effectSetting->formID : 0;
				if (snap.effectSettingFormID != currentSettingFormID) {
					REX::WARN("Magnitudes: carry-weight SPEL {:08X}[{}] effect changed {:08X} -> {:08X}; skipping",
						snap.spellFormID, snap.effectIndex,
						snap.effectSettingFormID, currentSettingFormID);
					continue;
				}
				eff->data.magnitude = a_unlock ? 0.0f : snap.baseline;
				++applied;
			}
			REX::INFO("Magnitudes: carry-weight {} ({} effect(s))",
				a_unlock ? "neutralized" : "restored",
				applied);
		}
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		// Must only be called from a safe context (pause-menu close inside a
		// loaded save). On OG, kGameDataReady runs inside Main_InitGameData
		// on the InitGameDataThread while mesh/UI subsystems are still warming
		// up; TESDataHandler::LookupForm and UI::GetMenu both trigger lazy
		// preloads there and null-deref in BSStaticTriShapeDB::Force (crashes
		// 2851eb7 and its MainMenu-gated follow-up). Hook the pause-menu sink
		// instead of Settings::Update.

		if (!g_snapshotted) {
			SnapshotAll();
			g_snapshotted = true;
		}

		const float stim_heal  = MCM::Settings::Magnitudes::fStimpakHeal.GetValue();
		const float stim_limb  = MCM::Settings::Magnitudes::fStimpakLimbRepair.GetValue();
		const float radaway    = MCM::Settings::Magnitudes::fRadAwayStrength.GetValue();
		const float radx       = MCM::Settings::Magnitudes::fRadXPotency.GetValue();
		const float food       = MCM::Settings::Magnitudes::fFoodHeal.GetValue();
		const float hunger_pen = MCM::Settings::Magnitudes::fHungerPenalty.GetValue();
		const float thirst_pen = MCM::Settings::Magnitudes::fThirstPenalty.GetValue();
		const float sleep_pen  = MCM::Settings::Magnitudes::fSleepPenalty.GetValue();

		ApplyAlch(kAlchStimpak, [stim_heal, stim_limb](std::uint32_t a_effectSettingFormID) -> float {
			switch (a_effectSettingFormID) {
				case kEffectRestoreHealthStimpak:
					return stim_heal;
				case kEffectRestoreConditionHead:
				case kEffectRestoreConditionLeftArm:
				case kEffectRestoreConditionLeftLeg:
				case kEffectRestoreConditionRightArm:
				case kEffectRestoreConditionRightLeg:
				case kEffectRestoreConditionTorso:
					return stim_limb;
				default:
					return 1.0f;
			}
		});
		ApplyAlch(kAlchRadAway, [radaway](std::uint32_t a_effectSettingFormID) {
			return a_effectSettingFormID == kEffectRestoreRadsChem ? radaway : 1.0f;
		});
		ApplyAlch(kAlchRadX, [radx](std::uint32_t a_effectSettingFormID) {
			return a_effectSettingFormID == kEffectFortifyResistRadsRadX ? radx : 1.0f;
		});

		for (auto id : kAlchHungerStages) {
			ApplyAlch(id, [hunger_pen](std::uint32_t) { return hunger_pen; });
		}
		for (auto id : kAlchThirstStages) {
			ApplyAlch(id, [thirst_pen](std::uint32_t) { return thirst_pen; });
		}
		for (auto id : kAlchSleepStages) {
			ApplyAlch(id, [sleep_pen](std::uint32_t) { return sleep_pen; });
		}

		ApplyFoodHeal(food);
		ApplyCarryWeightSpells(
			MCM::Settings::Unlocks::bNoSurvivalCarryWeight.GetValue());

		REX::INFO("Magnitudes: applied (stim_heal={} stim_limb={} radaway={} radx={} food={} hunger={} thirst={} sleep={})",
			stim_heal, stim_limb, radaway, radx, food, hunger_pen, thirst_pen, sleep_pen);
	}
}
