#pragma once

// Toggles HC_ReduceCarryWeightAbility by zeroing/restoring its effect magnitudes; replaces the prior ESP condition gate. Magnitude is cached per ActiveEffect at spell-apply time, so the toggle propagates only on save reload.
namespace Tweaks::SurvivalCarryWeight
{
	void Apply();
	void ResetSnapshots();
}
