#include "PCH.h"

#include "Tweaks/PlayerRefresh.h"

#include "Settings.h"

namespace Tweaks::PlayerRefresh
{
	namespace
	{
		using ResetDerivedAV_t = void (*)(RE::Actor*);

		// Actor::ResetDerivedActorValues: OG 1443411, NG/AE shared 2231022.
		REL::Relocation<ResetDerivedAV_t>& ResolveFn()
		{
			static REL::Relocation<ResetDerivedAV_t> fn{ REL::ID({ 1443411, 2231022 }) };
			return fn;
		}
	}

	void ResetDerivedActorValues()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}
		ResolveFn()(player);
	}
}
