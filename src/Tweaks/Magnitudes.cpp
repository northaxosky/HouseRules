#include "PCH.h"

#include "Tweaks/Magnitudes.h"

#include "Settings.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

namespace Tweaks::Magnitudes
{
	namespace
	{
		using Scaler = std::function<float(std::size_t /*effect_index*/)>;

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

		// MGEFs (EffectSetting records)
		constexpr std::uint32_t kMgefRestoreHealthFood  = 0x0000397E;
		constexpr std::uint32_t kMgefHCDrinkWaterEffect = 0x00000889;

		struct AlchSnapshot
		{
			std::uint32_t      formID;
			std::vector<float> baselines;  // one entry per EffectItem in listOfEffects
		};
		struct MgefSnapshot
		{
			std::uint32_t formID;
			float         baseline;
		};

		std::vector<AlchSnapshot> g_alch_snapshots;
		std::vector<MgefSnapshot> g_mgef_snapshots;
		bool                      g_snapshotted = false;

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

		RE::EffectSetting* LookupMgef(std::uint32_t a_formID)
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) return nullptr;
			for (auto* mgef : dh->GetFormArray<RE::EffectSetting>()) {
				if (mgef && mgef->formID == a_formID) return mgef;
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
			snap.baselines.reserve(alch->listOfEffects.size());
			for (auto* eff : alch->listOfEffects) {
				snap.baselines.push_back(eff ? eff->data.magnitude : 0.0f);
			}
			g_alch_snapshots.push_back(std::move(snap));
		}

		void SnapshotMgef(std::uint32_t a_formID)
		{
			auto* mgef = LookupMgef(a_formID);
			if (!mgef) {
				REX::WARN("Magnitudes: MGEF {:08X} not found", a_formID);
				return;
			}
			g_mgef_snapshots.push_back({ a_formID, mgef->data.baseCost });
		}

		const AlchSnapshot* FindAlchSnapshot(std::uint32_t a_formID)
		{
			for (const auto& s : g_alch_snapshots) {
				if (s.formID == a_formID) return &s;
			}
			return nullptr;
		}

		const MgefSnapshot* FindMgefSnapshot(std::uint32_t a_formID)
		{
			for (const auto& s : g_mgef_snapshots) {
				if (s.formID == a_formID) return &s;
			}
			return nullptr;
		}

		void SnapshotAll()
		{
			SnapshotAlch(kAlchStimpak);
			SnapshotAlch(kAlchRadAway);
			SnapshotAlch(kAlchRadX);
			for (auto id : kAlchHungerStages) SnapshotAlch(id);
			for (auto id : kAlchThirstStages) SnapshotAlch(id);
			for (auto id : kAlchSleepStages)  SnapshotAlch(id);
			SnapshotMgef(kMgefRestoreHealthFood);
			SnapshotMgef(kMgefHCDrinkWaterEffect);
			REX::INFO("Magnitudes: snapshotted {} ALCH / {} MGEF baselines",
				g_alch_snapshots.size(), g_mgef_snapshots.size());
		}

		void ApplyAlch(std::uint32_t a_formID, const Scaler& a_scaler)
		{
			const auto* snap = FindAlchSnapshot(a_formID);
			if (!snap) return;
			auto* alch = LookupAlch(a_formID);
			if (!alch) return;
			const auto count = std::min<std::size_t>(alch->listOfEffects.size(), snap->baselines.size());
			for (std::size_t i = 0; i < count; ++i) {
				auto* eff = alch->listOfEffects[static_cast<std::uint32_t>(i)];
				if (!eff) continue;
				eff->data.magnitude = snap->baselines[i] * a_scaler(i);
			}
		}

		void ApplyMgef(std::uint32_t a_formID, float a_mult)
		{
			const auto* snap = FindMgefSnapshot(a_formID);
			if (!snap) return;
			auto* mgef = LookupMgef(a_formID);
			if (!mgef) return;
			mgef->data.baseCost = snap->baseline * a_mult;
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
		const float water_sat  = MCM::Settings::Magnitudes::fWaterSatiation.GetValue();
		const float hunger_pen = MCM::Settings::Magnitudes::fHungerPenalty.GetValue();
		const float thirst_pen = MCM::Settings::Magnitudes::fThirstPenalty.GetValue();
		const float sleep_pen  = MCM::Settings::Magnitudes::fSleepPenalty.GetValue();

		// Stimpak: effect #0 = RestoreHealthStimpak, #2..#7 = limb RestoreCondition
		ApplyAlch(kAlchStimpak, [stim_heal, stim_limb](std::size_t i) -> float {
			if (i == 0) return stim_heal;
			if (i >= 2 && i <= 7) return stim_limb;
			return 1.0f;  // effect #1 = RestoreRadsCompanion — leave alone
		});
		ApplyAlch(kAlchRadAway, [radaway](std::size_t) { return radaway; });
		ApplyAlch(kAlchRadX,    [radx]   (std::size_t) { return radx;    });

		for (auto id : kAlchHungerStages) {
			ApplyAlch(id, [hunger_pen](std::size_t) { return hunger_pen; });
		}
		for (auto id : kAlchThirstStages) {
			ApplyAlch(id, [thirst_pen](std::size_t) { return thirst_pen; });
		}
		for (auto id : kAlchSleepStages) {
			ApplyAlch(id, [sleep_pen](std::size_t) { return sleep_pen; });
		}

		ApplyMgef(kMgefRestoreHealthFood,  food);
		ApplyMgef(kMgefHCDrinkWaterEffect, water_sat);

		REX::INFO("Magnitudes: applied (stim_heal={} stim_limb={} radaway={} radx={} food={} water_sat={} hunger={} thirst={} sleep={})",
			stim_heal, stim_limb, radaway, radx, food, water_sat, hunger_pen, thirst_pen, sleep_pen);
	}
}
