#include "PCH.h"

#include "Tweaks/CompanionsAffinity.h"

#include "Globals/Globals.h"
#include "Settings.h"

#include <array>
#include <cstdint>

namespace Tweaks::CompanionsAffinity
{
	namespace
	{
		struct Target
		{
			const char*      label;
			std::uint32_t    formID;
			REX::INI::F32<>* setting;
		};

		// Vanilla Fallout4.esm FormIDs spot-checked against the in-game GLOB dump. Values are per-event affinity deltas, not thresholds.
		const std::array<Target, 9> kTargets = { {
			{ "Loves Reaction",       0x5611Du, &MCM::Settings::CompanionsAffinity::fLove              },
			{ "Likes Reaction",       0x5611Cu, &MCM::Settings::CompanionsAffinity::fLike              },
			{ "Indifferent Reaction", 0x56120u, &MCM::Settings::CompanionsAffinity::fIndifferent       },
			{ "Dislikes Reaction",    0x5611Eu, &MCM::Settings::CompanionsAffinity::fDislike           },
			{ "Hates Reaction",       0x5611Fu, &MCM::Settings::CompanionsAffinity::fHates             },
			{ "Cooldown Immediate",   0xA1B7Cu, &MCM::Settings::CompanionsAffinity::fCooldownImmediate },
			{ "Cooldown Short",       0xA1B7Bu, &MCM::Settings::CompanionsAffinity::fCooldownShort     },
			{ "Cooldown Medium",      0xA1B7Du, &MCM::Settings::CompanionsAffinity::fCooldownMedium    },
			{ "Cooldown Long",        0xA1B7Eu, &MCM::Settings::CompanionsAffinity::fCooldownLong      },
		} };
	}

	void Apply()
	{
		if (!MCM::Settings::General::bEnabled.GetValue()) {
			return;
		}

		for (const auto& t : kTargets) {
			Globals::WriteByFormID(t.formID, t.setting->GetValue());
		}
	}
}
