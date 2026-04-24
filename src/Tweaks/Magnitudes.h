#pragma once

// Runtime magnitude scaling for the evidence-backed vanilla survival effects.
// On first call, snapshots EffectItem::data.magnitude for the proven ALCH /
// consumable entries we touch, then reapplies baseline * MCM multiplier on
// later refreshes. Unproven paths stay inert rather than guessing.
namespace Tweaks::Magnitudes
{
	void Apply();
}
