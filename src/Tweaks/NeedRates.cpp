#include "PCH.h"

#include "Tweaks/NeedRates.h"

namespace Tweaks::NeedRates
{
	// Deferred to v0.2. Vanilla Fallout 4 exposes no records for hunger/thirst/sleep
	// rate — the mechanism is engine-hardcoded. See .local/findings/vanilla-survival-is-hardcoded.md.
	// Re-enable once engine-hook infrastructure (from Hooks::Unlocks) is in place.
	void Apply() {}
}
