#pragma once

// Dev-only, read-only probe for the vanilla Survival manager Papyrus object.
// Gated by [Diagnostic] bHCManagerProbe.
namespace Diagnostics::HCManagerProbe
{
	void MaybeRun(const char* a_reason);
	void Reset();
}
