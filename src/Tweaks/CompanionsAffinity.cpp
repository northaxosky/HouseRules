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

		// FormIDs sourced from the Companions Affinity mod's MCM JSON
		// (sourceForm "Fallout4.esm|<hex>"). Spot-checked against the
		// in-game GLOB form-array dump (CA_Event_Likes/Loves/Dislikes/Hates).
		const std::array<Target, 9> kTargets = { {
			{ "Reaction Love",        0x5611Du, &MCM::Settings::CompanionsAffinity::fLove              },
			{ "Reaction Like",        0x5611Cu, &MCM::Settings::CompanionsAffinity::fLike              },
			{ "Reaction Indifferent", 0x56120u, &MCM::Settings::CompanionsAffinity::fIndifferent       },
			{ "Reaction Dislike",     0x5611Eu, &MCM::Settings::CompanionsAffinity::fDislike           },
			{ "Reaction Hates",       0x5611Fu, &MCM::Settings::CompanionsAffinity::fHates             },
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
