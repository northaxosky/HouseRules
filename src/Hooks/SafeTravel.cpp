#include "PCH.h"

#include "Hooks/SafeTravel.h"

#include <algorithm>
#include <chrono>

namespace Hooks::SafeTravel
{
	namespace
	{
		using Clock = std::chrono::steady_clock;

		bool              g_have_snapshot           = false;
		bool              g_fast_travel_armed       = false;
		bool              g_rad_exposure_shielded   = false;
		bool              g_rad_ingestion_shielded  = false;
		bool              g_world_ready             = false;
		Clock::time_point g_fast_travel_armed_at{};
		float             g_snap_health             = 0.0f;
		float             g_snap_max_health         = 0.0f;
		float             g_snap_rads               = 0.0f;

		// Minimum effective HP the player should have on resume. 1.0 is the
		// "barely alive" floor the engine treats as not-yet-dead.
		constexpr float kSafeFloor = 1.0f;
		constexpr float kRadShield = 1000000.0f;
		constexpr auto  kArmTimeout = std::chrono::seconds(15);

		void ClearArm()
		{
			g_fast_travel_armed = false;
			g_fast_travel_armed_at = {};
		}

		bool ConsumeFreshArm()
		{
			if (!g_fast_travel_armed) {
				return false;
			}

			g_fast_travel_armed = false;
			const auto age = Clock::now() - g_fast_travel_armed_at;
			if (age > kArmTimeout) {
				const auto age_ms = std::chrono::duration_cast<std::chrono::milliseconds>(age).count();
				REX::INFO("SafeTravel: ignored stale fast-travel arm (age={}ms)", age_ms);
				return false;
			}
			return true;
		}

		void ApplyRadiationShield(RE::PlayerCharacter& a_player, const RE::ActorValue& a_actorValues)
		{
			if (a_actorValues.radExposureResistance) {
				const float before = a_player.GetActorValue(*a_actorValues.radExposureResistance);
				a_player.ModActorValue(
					RE::ACTOR_VALUE_MODIFIER::kTemporary,
					*a_actorValues.radExposureResistance,
					kRadShield);
				g_rad_exposure_shielded = true;
				REX::INFO("SafeTravel: temporarily boosted rad exposure resistance; {} -> {}",
					before, a_player.GetActorValue(*a_actorValues.radExposureResistance));
			}

			if (a_actorValues.radIngestionResistance) {
				const float before = a_player.GetActorValue(*a_actorValues.radIngestionResistance);
				a_player.ModActorValue(
					RE::ACTOR_VALUE_MODIFIER::kTemporary,
					*a_actorValues.radIngestionResistance,
					kRadShield);
				g_rad_ingestion_shielded = true;
				REX::INFO("SafeTravel: temporarily boosted rad ingestion resistance; {} -> {}",
					before, a_player.GetActorValue(*a_actorValues.radIngestionResistance));
			}
		}

		void RemoveRadiationShield(RE::PlayerCharacter& a_player, const RE::ActorValue& a_actorValues)
		{
			if (g_rad_exposure_shielded && a_actorValues.radExposureResistance) {
				const float before = a_player.GetActorValue(*a_actorValues.radExposureResistance);
				a_player.ModActorValue(
					RE::ACTOR_VALUE_MODIFIER::kTemporary,
					*a_actorValues.radExposureResistance,
					-kRadShield);
				REX::INFO("SafeTravel: removed rad exposure resistance shield; {} -> {}",
					before, a_player.GetActorValue(*a_actorValues.radExposureResistance));
			}
			g_rad_exposure_shielded = false;

			if (g_rad_ingestion_shielded && a_actorValues.radIngestionResistance) {
				const float before = a_player.GetActorValue(*a_actorValues.radIngestionResistance);
				a_player.ModActorValue(
					RE::ACTOR_VALUE_MODIFIER::kTemporary,
					*a_actorValues.radIngestionResistance,
					-kRadShield);
				REX::INFO("SafeTravel: removed rad ingestion resistance shield; {} -> {}",
					before, a_player.GetActorValue(*a_actorValues.radIngestionResistance));
			}
			g_rad_ingestion_shielded = false;
		}

		void SnapshotAndProtect()
		{
			if (!g_world_ready) {
				return;
			}

			auto* pc  = RE::PlayerCharacter::GetSingleton();
			auto* avs = RE::ActorValue::GetSingleton();
			if (!pc || !avs || !avs->health) {
				return;
			}

			g_snap_health     = pc->GetActorValue(*avs->health);
			g_snap_max_health = pc->GetPermanentActorValue(*avs->health);
			g_snap_rads       = avs->rads ? pc->GetActorValue(*avs->rads) : 0.0f;
			g_have_snapshot   = true;

			ApplyRadiationShield(*pc, *avs);

			if (avs->rads && g_snap_rads > 0.0f) {
				pc->RestoreActorValue(*avs->rads, g_snap_rads);
				REX::INFO("SafeTravel: temporarily cleared rads for load-screen travel; rads {} -> {}",
					g_snap_rads, pc->GetActorValue(*avs->rads));
			}

			if (g_snap_max_health > g_snap_health) {
				pc->RestoreActorValue(*avs->health, g_snap_max_health - g_snap_health);
				REX::INFO("SafeTravel: temporarily restored HP for load-screen travel; HP {} -> {}",
					g_snap_health, pc->GetActorValue(*avs->health));
			}
		}

		void RestoreSnapshotRads(RE::PlayerCharacter& a_player, const RE::ActorValue& a_actorValues)
		{
			if (!a_actorValues.rads) {
				return;
			}

			const float current_rads = a_player.GetActorValue(*a_actorValues.rads);
			if (current_rads > g_snap_rads) {
				a_player.RestoreActorValue(*a_actorValues.rads, current_rads - g_snap_rads);
			} else if (current_rads < g_snap_rads) {
				a_player.ModActorValue(
					RE::ACTOR_VALUE_MODIFIER::kDamage,
					*a_actorValues.rads,
					g_snap_rads - current_rads);
			} else {
				return;
			}

			const float readback = a_player.GetActorValue(*a_actorValues.rads);
			REX::INFO("SafeTravel: restored pre-travel rads; rads {} -> {} (pre-travel was {})",
				current_rads, readback, g_snap_rads);
		}

		void RestoreSnapshotHealth(RE::PlayerCharacter& a_player, const RE::ActorValue& a_actorValues)
		{
			const float current = a_player.GetActorValue(*a_actorValues.health);
			const float target = std::max(kSafeFloor, g_snap_health);

			if (current > target) {
				a_player.ModActorValue(
					RE::ACTOR_VALUE_MODIFIER::kDamage,
					*a_actorValues.health,
					current - target);
				REX::INFO("SafeTravel: removed temporary load-screen HP; HP {} -> {} (pre-travel was {})",
					current, a_player.GetActorValue(*a_actorValues.health), g_snap_health);
				return;
			}

			if (current >= target) {
				return;
			}

			a_player.RestoreActorValue(*a_actorValues.health, target - current);
			REX::INFO(
				"SafeTravel: restored load-screen HP loss; HP {} -> {} (pre-travel was {}, max {})",
				current, a_player.GetActorValue(*a_actorValues.health), g_snap_health, g_snap_max_health);
		}

		void RestoreAfterTravel()
		{
			if (!g_have_snapshot) {
				return;
			}
			g_have_snapshot = false;

			auto* pc  = RE::PlayerCharacter::GetSingleton();
			auto* avs = RE::ActorValue::GetSingleton();
			if (!pc || !avs || !avs->health) {
				return;
			}

			RestoreSnapshotRads(*pc, *avs);
			RestoreSnapshotHealth(*pc, *avs);
			RemoveRadiationShield(*pc, *avs);
		}
	}

	void ArmFastTravel()
	{
		if (!g_world_ready) {
			return;
		}
		if (!g_fast_travel_armed) {
			REX::INFO("SafeTravel: armed for next fast-travel load screen");
		}
		g_fast_travel_armed = true;
		g_fast_travel_armed_at = Clock::now();
	}

	void OnMenuOpenClose(const RE::MenuOpenCloseEvent& a_event)
	{
		if (a_event.menuName == "MainMenu" && a_event.opening) {
			g_world_ready = false;
			g_have_snapshot = false;
			ClearArm();
			return;
		}

		if (a_event.menuName != "LoadingMenu") {
			return;
		}
		if (a_event.opening) {
			if (ConsumeFreshArm()) {
				SnapshotAndProtect();
			}
		} else {
			if (!g_world_ready) {
				g_world_ready = true;
				g_have_snapshot = false;
				ClearArm();
				return;
			}
			RestoreAfterTravel();
		}
	}
}
