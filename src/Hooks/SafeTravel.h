#pragma once

namespace RE
{
	class MenuOpenCloseEvent;
}

// Vanilla bug: in Survival, the time-skip that happens during fast travel
// (and any other LoadingMenu transition that advances the world clock -
// door autosaves, sleep, wait) ticks the survival systems forward without
// the player's HP being safe. Two known kill paths:
//
//   1. Hunger/thirst stage advances -> Endurance debuff -> max HP drops ->
//      current HP clamped to new max -> if new max <= 0, death on resume.
//   2. Active rad damage (rad storm) keeps accumulating during the time
//      skip; rads >= max HP at resume = instant rad death.
//
// This module becomes active after the first world load completes, then
// snapshots effective health and rads when later LoadingMenu transitions open.
// On close, it removes rads accumulated during that load-screen window, then
// restores enough health damage modifier to leave the player at 1 HP if the
// remaining Survival time-skip effects would otherwise be lethal. Initial save
// loads are ignored so saved radiation is never rewritten. No immortality
// byte-patch, no IsDead lying.
namespace Hooks::SafeTravel
{
	void OnMenuOpenClose(const RE::MenuOpenCloseEvent& a_event);
}
