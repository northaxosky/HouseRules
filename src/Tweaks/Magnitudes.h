#pragma once

// Runtime magnitude scaling for vanilla FO4 survival forms. On first call
// snapshots the vanilla EffectItem::data.magnitude / EffectSetting::data.baseCost
// values for a fixed set of AlchemyItems and MagicEffects (stimpak, radaway,
// radx, food heal MGEF, hunger/thirst/sleep stage ALCHs, drink-water MGEF).
// On every subsequent call — including refreshes triggered by pause-menu
// close — writes snapshot * MCM multiplier. Idempotent; never reads the
// already-scaled current value.
namespace Tweaks::Magnitudes
{
	void Apply();
}
