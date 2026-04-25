#include "PCH.h"

#include "Hooks/SafeTravel.h"

namespace Hooks::SafeTravel
{
	namespace
	{
		bool  g_have_snapshot   = false;
		bool  g_world_ready     = false;
		float g_snap_health     = 0.0f;
		float g_snap_max_health = 0.0f;
		float g_snap_rads       = 0.0f;

		// Minimum effective HP the player should have on resume. 1.0 is the
		// "barely alive" floor the engine treats as not-yet-dead.
		constexpr float kSafeFloor = 1.0f;

		void Snapshot()
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
		}

		void RestoreLoadScreenRads(RE::PlayerCharacter& a_player, const RE::ActorValue& a_actorValues)
		{
			if (!a_actorValues.rads) {
				return;
			}

			const float current_rads = a_player.GetActorValue(*a_actorValues.rads);
			if (current_rads <= g_snap_rads) {
				return;
			}

			const float delta = current_rads - g_snap_rads;
			a_player.RestoreActorValue(*a_actorValues.rads, delta);
			const float readback = a_player.GetActorValue(*a_actorValues.rads);
			REX::INFO("SafeTravel: restored load-screen rads; rads {} -> {} (pre-load was {})",
				current_rads, readback, g_snap_rads);
		}

		void RestoreIfLethal()
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

			RestoreLoadScreenRads(*pc, *avs);

			const float current = pc->GetActorValue(*avs->health);
			if (current >= kSafeFloor) {
				return;
			}

			// Lift effective HP back to kSafeFloor. RestoreActorValue is the
			// engine's own "heal" path -- same one Papyrus's `RestoreActorValue`
			// resolves to -- so it walks through the proper damage/permanent
			// modifier stack rather than us guessing the sign convention.
			const float delta = kSafeFloor - current;
			pc->RestoreActorValue(*avs->health, delta);

			REX::INFO(
				"SafeTravel: clamped lethal load-screen damage; HP {} -> {} (pre-load was {}, max {})",
				current, kSafeFloor, g_snap_health, g_snap_max_health);
		}
	}

	void OnMenuOpenClose(const RE::MenuOpenCloseEvent& a_event)
	{
		if (a_event.menuName == "MainMenu" && a_event.opening) {
			g_world_ready = false;
			g_have_snapshot = false;
			return;
		}

		if (a_event.menuName != "LoadingMenu") {
			return;
		}
		if (a_event.opening) {
			Snapshot();
		} else {
			if (!g_world_ready) {
				g_world_ready = true;
				g_have_snapshot = false;
				return;
			}
			RestoreIfLethal();
		}
	}
}
