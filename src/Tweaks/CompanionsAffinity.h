#pragma once

// Companions Affinity v0.4 -- direct writes to the nine vanilla
// Fallout4.esm TESGlobals controlling companion reaction thresholds
// (Love / Like / Indifferent / Dislike / Hates) and reaction event
// cooldowns (Immediate / Short / Medium / Long, in game days).
//
// Replicates the Companions Affinity Nexus mod's surface natively;
// no Papyrus, no script forks. Each global is addressed by its
// stable Fallout4.esm FormID.
namespace Tweaks::CompanionsAffinity
{
	void Apply();
}
