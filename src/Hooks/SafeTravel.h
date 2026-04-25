#pragma once

namespace RE
{
	class MenuOpenCloseEvent;
}

// Vanilla bug: in Survival, the time-skip that happens during fast travel
// ticks the survival systems forward without the player's HP being safe. Two
// known kill paths:
//
//   1. Hunger/thirst stage advances -> Endurance debuff -> max HP drops ->
//      current HP clamped to new max -> if new max <= 0, death on resume.
//   2. Active rad damage (rad storm) keeps accumulating during the time
//      skip; rads >= max HP at resume = instant rad death.
//
// This module becomes active after the first world load completes. Fast-travel
// hooks arm it explicitly; the next LoadingMenu then temporarily shields rad
// exposure/ingestion, clears existing rads, and heals the player before the
// time skip can mark them dead. On close, it restores pre-travel rads, restores
// HP toward the pre-travel value, and removes the rad shield. Ordinary door/cell
// loads are not touched. Initial save loads are ignored so saved radiation is
// never rewritten. No immortality byte-patch, no IsDead lying.
namespace Hooks::SafeTravel
{
	void ArmFastTravel();
	void OnMenuOpenClose(const RE::MenuOpenCloseEvent& a_event);
}
