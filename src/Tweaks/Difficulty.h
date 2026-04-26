#pragma once

// Runtime scaling for vanilla difficulty GMSTs (incoming/outgoing damage,
// XP base/mult, legendary chance/rarity). Snapshots the engine's baseline
// float on first safe Apply(), then writes baseline * MCM multiplier on each
// later refresh -- so live MCM edits are reversible within the session and
// don't compound. Skips legacy SV GMSTs; current Survival is TSV.
namespace Tweaks::Difficulty
{
	void Apply();
}
