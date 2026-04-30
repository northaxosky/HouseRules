#pragma once

// Dev-only, read-only probe for ActorValue / form-array surfaces that are not
// RE::GameSettingCollection GMSTs. Gated by [Diagnostic] bActorValueProbe.
namespace Diagnostics::ActorValueProbe
{
	void MaybeRun(const char* a_reason);
}
