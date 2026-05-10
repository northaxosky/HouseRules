#pragma once

// Replicates Companions Affinity natively via direct writes to nine vanilla Fallout4.esm TESGlobals (per-reaction affinity deltas + event cooldowns).
namespace Tweaks::CompanionsAffinity
{
	void Apply();
}
