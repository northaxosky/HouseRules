#pragma once

namespace RE
{
	class MenuOpenCloseEvent;
}

// Vanilla bug: in Survival, the time-skip that happens during fast travel
// (and any other LoadingMenu transition that advances the world clock —
// door autosaves, sleep, wait) ticks the survival systems forward without
// the player's HP being safe. Two known kill paths:
//
//   1. Hunger/thirst stage advances -> Endurance debuff -> max HP drops ->
//      current HP clamped to new max -> if new max <= 0, death on resume.
//   2. Active rad damage (rad storm) keeps accumulating during the time
//      skip; rads >= max HP at resume = instant rad death.
//
// We can't actually predict which one triggered any given death, but the
// fix is the same: ensure the player can't return to player control with
// effective HP <= 0. This module snapshots the player's effective health
// when a LoadingMenu opens and, on close, if HP has dropped to a lethal
// value, restores enough damage modifier to leave the player at 1 HP. The
// player still feels every other AV change from the time skip; only the
// killing blow is suppressed. No immortality byte-patch, no IsDead lying.
namespace Hooks::SafeTravel
{
	void OnMenuOpenClose(const RE::MenuOpenCloseEvent& a_event);
}
