#include "PCH.h"

#include "Diagnostics/ActorValueProbe.h"

#include "Settings.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace Diagnostics::ActorValueProbe
{
	namespace
	{
		struct TrackedActorValue
		{
			const char*                  label;
			RE::ActorValueInfo* RE::ActorValue::* member;
		};

		constexpr std::array kTrackedActorValues{
			TrackedActorValue{ "absorbChance", &RE::ActorValue::absorbChance },
			TrackedActorValue{ "actionPoints", &RE::ActorValue::actionPoints },
			TrackedActorValue{ "aggression", &RE::ActorValue::aggression },
			TrackedActorValue{ "agility", &RE::ActorValue::agility },
			TrackedActorValue{ "aimStability", &RE::ActorValue::aimStability },
			TrackedActorValue{ "alchemy", &RE::ActorValue::alchemy },
			TrackedActorValue{ "animationMult", &RE::ActorValue::animationMult },
			TrackedActorValue{ "armorPerks", &RE::ActorValue::armorPerks },
			TrackedActorValue{ "assistance", &RE::ActorValue::assistance },
			TrackedActorValue{ "attackDamageMult", &RE::ActorValue::attackDamageMult },
			TrackedActorValue{ "blindness", &RE::ActorValue::blindness },
			TrackedActorValue{ "block", &RE::ActorValue::block },
			TrackedActorValue{ "bloodyMess", &RE::ActorValue::bloodyMess },
			TrackedActorValue{ "bowSpeedBonus", &RE::ActorValue::bowSpeedBonus },
			TrackedActorValue{ "bowStaggerBonus", &RE::ActorValue::bowStaggerBonus },
			TrackedActorValue{ "brainCondition", &RE::ActorValue::brainCondition },
			TrackedActorValue{ "carryWeight", &RE::ActorValue::carryWeight },
			TrackedActorValue{ "charisma", &RE::ActorValue::charisma },
			TrackedActorValue{ "combatHealthRegenMult", &RE::ActorValue::combatHealthRegenMult },
			TrackedActorValue{ "confidence", &RE::ActorValue::confidence },
			TrackedActorValue{ "criticalChance", &RE::ActorValue::criticalChance },
			TrackedActorValue{ "damageResistance", &RE::ActorValue::damageResistance },
			TrackedActorValue{ "deafness", &RE::ActorValue::deafness },
			TrackedActorValue{ "enchanting", &RE::ActorValue::enchanting },
			TrackedActorValue{ "endurance", &RE::ActorValue::endurance },
			TrackedActorValue{ "enduranceCondition", &RE::ActorValue::enduranceCondition },
			TrackedActorValue{ "experience", &RE::ActorValue::experience },
			TrackedActorValue{ "health", &RE::ActorValue::health },
			TrackedActorValue{ "healRateMult", &RE::ActorValue::healRateMult },
			TrackedActorValue{ "idleChatterTimeMin", &RE::ActorValue::idleChatterTimeMin },
			TrackedActorValue{ "idleChatterTimeMAx", &RE::ActorValue::idleChatterTimeMAx },
			TrackedActorValue{ "ignoreCrippledLimbs", &RE::ActorValue::ignoreCrippledLimbs },
			TrackedActorValue{ "intelligence", &RE::ActorValue::intelligence },
			TrackedActorValue{ "invisibility", &RE::ActorValue::invisibility },
			TrackedActorValue{ "karma", &RE::ActorValue::karma },
			TrackedActorValue{ "leftAttackCondition", &RE::ActorValue::leftAttackCondition },
			TrackedActorValue{ "leftItemCharge", &RE::ActorValue::leftItemCharge },
			TrackedActorValue{ "leftMobiltyCondition", &RE::ActorValue::leftMobiltyCondition },
			TrackedActorValue{ "leftWeaponSpeedMult", &RE::ActorValue::leftWeaponSpeedMult },
			TrackedActorValue{ "lockpicking", &RE::ActorValue::lockpicking },
			TrackedActorValue{ "luck", &RE::ActorValue::luck },
			TrackedActorValue{ "mass", &RE::ActorValue::mass },
			TrackedActorValue{ "meleeDamage", &RE::ActorValue::meleeDamage },
			TrackedActorValue{ "morality", &RE::ActorValue::morality },
			TrackedActorValue{ "movementNoiseMult", &RE::ActorValue::movementNoiseMult },
			TrackedActorValue{ "nightEyeBonus", &RE::ActorValue::nightEyeBonus },
			TrackedActorValue{ "perception", &RE::ActorValue::perception },
			TrackedActorValue{ "perceptionCondition", &RE::ActorValue::perceptionCondition },
			TrackedActorValue{ "pickpocket", &RE::ActorValue::pickpocket },
			TrackedActorValue{ "poisonResistance", &RE::ActorValue::poisonResistance },
			TrackedActorValue{ "powerArmorBattery", &RE::ActorValue::powerArmorBattery },
			TrackedActorValue{ "powerArmorHeadCondition", &RE::ActorValue::powerArmorHeadCondition },
			TrackedActorValue{ "powerArmorTorsoCondition", &RE::ActorValue::powerArmorTorsoCondition },
			TrackedActorValue{ "powerArmorLeftArmCondition", &RE::ActorValue::powerArmorLeftArmCondition },
			TrackedActorValue{ "powerArmorRightArmCondition", &RE::ActorValue::powerArmorRightArmCondition },
			TrackedActorValue{ "powerArmorLeftLegCondition", &RE::ActorValue::powerArmorLeftLegCondition },
			TrackedActorValue{ "powerArmorRightLegCondition", &RE::ActorValue::powerArmorRightLegCondition },
			TrackedActorValue{ "radHealthMax", &RE::ActorValue::radHealthMax },
			TrackedActorValue{ "rads", &RE::ActorValue::rads },
			TrackedActorValue{ "fatigueAPMax", &RE::ActorValue::fatigueAPMax },
			TrackedActorValue{ "fatigue", &RE::ActorValue::fatigue },
			TrackedActorValue{ "reflectDamage", &RE::ActorValue::reflectDamage },
			TrackedActorValue{ "restoreConditionRate", &RE::ActorValue::restoreConditionRate },
			TrackedActorValue{ "rightAttackCondition", &RE::ActorValue::rightAttackCondition },
			TrackedActorValue{ "rightItemCharge", &RE::ActorValue::rightItemCharge },
			TrackedActorValue{ "rightMobilityCondition", &RE::ActorValue::rightMobilityCondition },
			TrackedActorValue{ "rotationSpeedCondition", &RE::ActorValue::rotationSpeedCondition },
			TrackedActorValue{ "shieldPerks", &RE::ActorValue::shieldPerks },
			TrackedActorValue{ "shoutRecoveryMult", &RE::ActorValue::shoutRecoveryMult },
			TrackedActorValue{ "sneak", &RE::ActorValue::sneak },
			TrackedActorValue{ "speechcraft", &RE::ActorValue::speechcraft },
			TrackedActorValue{ "speedMult", &RE::ActorValue::speedMult },
			TrackedActorValue{ "stamina", &RE::ActorValue::stamina },
			TrackedActorValue{ "strength", &RE::ActorValue::strength },
			TrackedActorValue{ "suspicious", &RE::ActorValue::suspicious },
			TrackedActorValue{ "telekinesis", &RE::ActorValue::telekinesis },
			TrackedActorValue{ "unarmedDamage", &RE::ActorValue::unarmedDamage },
			TrackedActorValue{ "vansPerk", &RE::ActorValue::vansPerk },
			TrackedActorValue{ "wardPower", &RE::ActorValue::wardPower },
			TrackedActorValue{ "waitingForPlayer", &RE::ActorValue::waitingForPlayer },
			TrackedActorValue{ "waterBreathing", &RE::ActorValue::waterBreathing },
			TrackedActorValue{ "waterWalking", &RE::ActorValue::waterWalking },
			TrackedActorValue{ "weaponSpeedMult", &RE::ActorValue::weaponSpeedMult },
			TrackedActorValue{ "weapReloadSpeedMult", &RE::ActorValue::weapReloadSpeedMult },
			TrackedActorValue{ "restoreHealthRate", &RE::ActorValue::restoreHealthRate },
			TrackedActorValue{ "restoreAPRate", &RE::ActorValue::restoreAPRate },
			TrackedActorValue{ "apRateMult", &RE::ActorValue::apRateMult },
			TrackedActorValue{ "restoreRadsRate", &RE::ActorValue::restoreRadsRate },
			TrackedActorValue{ "radsRateMult", &RE::ActorValue::radsRateMult },
			TrackedActorValue{ "restoreFatigueRate", &RE::ActorValue::restoreFatigueRate },
			TrackedActorValue{ "fatigueRateMult", &RE::ActorValue::fatigueRateMult },
			TrackedActorValue{ "conditionRateMult", &RE::ActorValue::conditionRateMult },
			TrackedActorValue{ "fireResistance", &RE::ActorValue::fireResistance },
			TrackedActorValue{ "electricalResistance", &RE::ActorValue::electricalResistance },
			TrackedActorValue{ "frostResistance", &RE::ActorValue::frostResistance },
			TrackedActorValue{ "magicResistance", &RE::ActorValue::magicResistance },
			TrackedActorValue{ "radIngestionResistance", &RE::ActorValue::radIngestionResistance },
			TrackedActorValue{ "radExposureResistance", &RE::ActorValue::radExposureResistance },
			TrackedActorValue{ "energy", &RE::ActorValue::energy },
			TrackedActorValue{ "energyResistance", &RE::ActorValue::energyResistance },
			TrackedActorValue{ "paralysis", &RE::ActorValue::paralysis },
			TrackedActorValue{ "attackConditionAlt1", &RE::ActorValue::attackConditionAlt1 },
			TrackedActorValue{ "attackConditionAlt2", &RE::ActorValue::attackConditionAlt2 },
			TrackedActorValue{ "attackConditionAlt3", &RE::ActorValue::attackConditionAlt3 },
			TrackedActorValue{ "powerGenerated", &RE::ActorValue::powerGenerated },
			TrackedActorValue{ "powerRadation", &RE::ActorValue::powerRadation },
			TrackedActorValue{ "powerRequired", &RE::ActorValue::powerRequired },
			TrackedActorValue{ "resourceFood", &RE::ActorValue::resourceFood },
			TrackedActorValue{ "resourceWater", &RE::ActorValue::resourceWater },
			TrackedActorValue{ "resourceSafety", &RE::ActorValue::resourceSafety },
			TrackedActorValue{ "resourceBed", &RE::ActorValue::resourceBed },
			TrackedActorValue{ "resourceHappiness", &RE::ActorValue::resourceHappiness },
			TrackedActorValue{ "resourceArtillery", &RE::ActorValue::resourceArtillery },
			TrackedActorValue{ "workshopItemOverlap", &RE::ActorValue::workshopItemOverlap },
			TrackedActorValue{ "workshopItemClampDirection", &RE::ActorValue::workshopItemClampDirection },
			TrackedActorValue{ "workshopItemZOffset", &RE::ActorValue::workshopItemZOffset },
			TrackedActorValue{ "workshopPlayerOwned", &RE::ActorValue::workshopPlayerOwned },
			TrackedActorValue{ "workshopActorWounded", &RE::ActorValue::workshopActorWounded },
			TrackedActorValue{ "workshopStackableItem", &RE::ActorValue::workshopStackableItem },
			TrackedActorValue{ "workshopSnapPointRadius", &RE::ActorValue::workshopSnapPointRadius },
			TrackedActorValue{ "workshopAnythingIsGround", &RE::ActorValue::workshopAnythingIsGround },
			TrackedActorValue{ "workshopMaxTriangles", &RE::ActorValue::workshopMaxTriangles },
			TrackedActorValue{ "workshopMaxDraws", &RE::ActorValue::workshopMaxDraws },
			TrackedActorValue{ "workshopCurrentTriangles", &RE::ActorValue::workshopCurrentTriangles },
			TrackedActorValue{ "workshopCurrentDraws", &RE::ActorValue::workshopCurrentDraws },
			TrackedActorValue{ "workshopIgnoreSimpleIntersections", &RE::ActorValue::workshopIgnoreSimpleIntersections },
			TrackedActorValue{ "workshopAllowUnsupportedStacking", &RE::ActorValue::workshopAllowUnsupportedStacking },
			TrackedActorValue{ "workshopSnapTransmitsPower", &RE::ActorValue::workshopSnapTransmitsPower },
			TrackedActorValue{ "workshopPowerConnection", &RE::ActorValue::workshopPowerConnection },
			TrackedActorValue{ "logicOperation", &RE::ActorValue::logicOperation },
			TrackedActorValue{ "logicOutput", &RE::ActorValue::logicOutput },
			TrackedActorValue{ "ignorePlayerWhileFrenzied", &RE::ActorValue::ignorePlayerWhileFrenzied },
		};

		bool g_ranPhaseA = false;
		bool g_ranPhaseB = false;

		std::vector<std::string> SplitFilter(const std::string& a_raw)
		{
			std::vector<std::string> out;
			std::size_t start = 0;
			for (std::size_t i = 0; i <= a_raw.size(); ++i) {
				if (i == a_raw.size() || a_raw[i] == ',') {
					auto token = a_raw.substr(start, i - start);
					while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front()))) {
						token.erase(token.begin());
					}
					while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back()))) {
						token.pop_back();
					}
					if (!token.empty()) {
						out.push_back(std::move(token));
					}
					start = i + 1;
				}
			}
			return out;
		}

		bool NameMatches(std::string_view a_name, const std::vector<std::string>& a_filters)
		{
			if (a_name.empty()) {
				return false;
			}
			if (a_filters.empty()) {
				return true;
			}
			for (const auto& filter : a_filters) {
				if (filter == "*" || a_name.find(filter) != std::string_view::npos) {
					return true;
				}
			}
			return false;
		}

		const char* SafeEditorID(const RE::TESForm* a_form)
		{
			if (!a_form) {
				return "";
			}
			const auto edid = a_form->GetFormEditorID();
			return edid ? edid : "";
		}

		const char* SafeString(const char* a_value)
		{
			return a_value ? a_value : "";
		}

		std::uint32_t FormID(const RE::TESForm* a_form)
		{
			return a_form ? a_form->formID : 0u;
		}

		std::uint32_t Flags(const RE::ActorValueInfo& a_info)
		{
			return static_cast<std::uint32_t>(a_info.flags.get());
		}

		std::int32_t AVType(const RE::ActorValueInfo& a_info)
		{
			return static_cast<std::int32_t>(a_info.avType.get());
		}

		void LogAVMetadata(std::string_view a_prefix, const RE::ActorValueInfo& a_info, const char* a_label)
		{
			const auto depCount = std::min<std::uint32_t>(a_info.numDependentActorValues, 4);
			std::uint32_t deps[4]{};
			for (std::uint32_t i = 0; i < depCount; ++i) {
				deps[i] = FormID(a_info.dependentActorValues[i]);
			}

			REX::INFO("AVProbe: {} label='{}' formID={:08X} edid='{}' avType={} flags=0x{:08X} defVal={} fullIdx={} permIdx={} deps={} dep0={:08X} dep1={:08X} dep2={:08X} dep3={:08X}",
				a_prefix,
				SafeString(a_label),
				a_info.formID,
				SafeEditorID(std::addressof(a_info)),
				AVType(a_info),
				Flags(a_info),
				a_info.defaultValue,
				a_info.fullCacheIndex,
				a_info.permanentCacheIndex,
				a_info.numDependentActorValues,
				deps[0],
				deps[1],
				deps[2],
				deps[3]);
		}

		void LogPlayerSnapshot(const char* a_prefix, const RE::PlayerCharacter& a_player,
			const RE::ActorValueInfo& a_info, const char* a_label)
		{
			const auto current = a_player.GetActorValue(a_info);
			const auto permanentValue = a_player.GetPermanentActorValue(a_info);
			const auto base = a_player.GetBaseActorValue(a_info);
			const auto permanentMod = a_player.GetModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, a_info);
			const auto temporaryMod = a_player.GetModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, a_info);
			const auto damage = a_player.GetModifier(RE::ACTOR_VALUE_MODIFIER::kDamage, a_info);

			REX::INFO("AVProbe: {} label='{}' edid='{}' current={} permanent={} base={} modPermanent={} modTemporary={} damage={}",
				a_prefix,
				SafeString(a_label),
				SafeEditorID(std::addressof(a_info)),
				current,
				permanentValue,
				base,
				permanentMod,
				temporaryMod,
				damage);
		}

		bool ShouldSnapshotPlayer()
		{
			return MCM::Settings::Diagnostic::bActorValueProbePlayerSnapshot.GetValue();
		}

		RE::PlayerCharacter* GetSnapshotPlayer()
		{
			return ShouldSnapshotPlayer() ? RE::PlayerCharacter::GetSingleton() : nullptr;
		}

		bool MatchesActorValue(const char* a_label, const RE::ActorValueInfo& a_info,
			const std::vector<std::string>& a_filters)
		{
			return NameMatches(a_label, a_filters) ||
			       NameMatches(SafeEditorID(std::addressof(a_info)), a_filters);
		}

		void RunPhaseA()
		{
			const auto& rawFilter = MCM::Settings::Diagnostic::sActorValueProbeFilter.GetValue();
			const auto filters = SplitFilter(rawFilter);

			REX::INFO("AVProbe: --- begin phase=A filter='{}' members={} ---",
				rawFilter,
				kTrackedActorValues.size());

			const auto* actorValues = RE::ActorValue::GetSingleton();
			if (!actorValues) {
				REX::WARN("AVProbe: phase=A skipped; ActorValue singleton unavailable");
				REX::INFO("AVProbe: --- end phase=A matched=0 fallback=0 ---");
				return;
			}

			auto* player = GetSnapshotPlayer();
			std::unordered_set<const RE::ActorValueInfo*> seen;
			std::size_t matched = 0;

			for (std::size_t i = 0; i < kTrackedActorValues.size(); ++i) {
				const auto& tracked = kTrackedActorValues[i];
				const auto* info = actorValues->*tracked.member;
				if (!info) {
					if (NameMatches(tracked.label, filters)) {
						REX::WARN("AVProbe: AV idx={} label='{}' unavailable on this runtime", i, tracked.label);
					}
					continue;
				}

				seen.insert(info);
				if (!MatchesActorValue(tracked.label, *info, filters)) {
					continue;
				}

				++matched;
				const auto prefix = std::string{ "AV idx=" } + std::to_string(i);
				LogAVMetadata(prefix, *info, tracked.label);
				if (player) {
					LogPlayerSnapshot("AV-PC", *player, *info, tracked.label);
				}
			}

			std::size_t fallback = 0;
			auto logFallback = [&](const RE::BSTArray<RE::ActorValueInfo*>& a_array, const char* a_source) {
				for (const auto* info : a_array) {
					if (!info || seen.contains(info)) {
						continue;
					}
					seen.insert(info);
					const auto* edid = SafeEditorID(info);
					if (!NameMatches(edid, filters)) {
						continue;
					}
					++fallback;
					REX::INFO("AVProbe: AV-EXT source='{}' formID={:08X} edid='{}' avType={} flags=0x{:08X} defVal={}",
						a_source,
						info->formID,
						edid,
						AVType(*info),
						Flags(*info),
						info->defaultValue);
					if (player) {
						LogPlayerSnapshot("AV-PC", *player, *info, edid);
					}
				}
			};

			logFallback(actorValues->hardcodedActorValues, "hardcoded");
			logFallback(actorValues->conditionActorValues, "condition");

			REX::INFO("AVProbe: --- end phase=A matched={} fallback={} ---", matched, fallback);
		}

		void LogAVIFCandidate(const RE::ActorValueInfo& a_info, const RE::PlayerCharacter* a_player)
		{
			if (a_player) {
				const auto current = a_player->GetActorValue(a_info);
				const auto permanentValue = a_player->GetPermanentActorValue(a_info);
				const auto base = a_player->GetBaseActorValue(a_info);
				const auto permanentMod = a_player->GetModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, a_info);
				const auto temporaryMod = a_player->GetModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, a_info);
				const auto damage = a_player->GetModifier(RE::ACTOR_VALUE_MODIFIER::kDamage, a_info);

				REX::INFO("AVProbe: DO-CANDIDATE kind=AVIF formID={:08X} edid='{}' avType={} flags=0x{:08X} defVal={} current={} permanent={} base={} modPermanent={} modTemporary={} damage={}",
					a_info.formID,
					SafeEditorID(std::addressof(a_info)),
					AVType(a_info),
					Flags(a_info),
					a_info.defaultValue,
					current,
					permanentValue,
					base,
					permanentMod,
					temporaryMod,
					damage);
				return;
			}

			REX::INFO("AVProbe: DO-CANDIDATE kind=AVIF formID={:08X} edid='{}' avType={} flags=0x{:08X} defVal={}",
				a_info.formID,
				SafeEditorID(std::addressof(a_info)),
				AVType(a_info),
				Flags(a_info),
				a_info.defaultValue);
		}

		bool MatchesMGEF(const RE::EffectSetting& a_mgef, const std::vector<std::string>& a_filters)
		{
			return NameMatches(SafeEditorID(std::addressof(a_mgef)), a_filters) ||
			       NameMatches(SafeEditorID(a_mgef.data.primaryAV), a_filters) ||
			       NameMatches(SafeEditorID(a_mgef.data.secondaryAV), a_filters);
		}

		void RunOptionalDFOBSweep(RE::TESDataHandler& a_dataHandler, const std::vector<std::string>& a_filters,
			std::size_t& a_matched, bool& a_unavailable)
		{
			if (!MCM::Settings::Diagnostic::bActorValueProbeDFOBArray.GetValue()) {
				return;
			}

			auto& dfobs = a_dataHandler.GetFormArray<RE::BGSDefaultObject>();
			if (dfobs.empty()) {
				a_unavailable = true;
				REX::INFO("AVProbe: DO-DFOB unavailable (BGSDefaultObject form array empty; singleton path intentionally not used)");
				return;
			}

			for (auto* dfob : dfobs) {
				if (!dfob) {
					continue;
				}
				const auto* doName = dfob->formEditorID.c_str();
				const auto* formEdid = SafeEditorID(dfob->form);
				if (!NameMatches(doName, a_filters) && !NameMatches(formEdid, a_filters)) {
					continue;
				}
				++a_matched;
				REX::INFO("AVProbe: DO-DFOB doName='{}' type={} form={:08X} formEdid='{}'",
					doName,
					static_cast<std::uint32_t>(dfob->type),
					FormID(dfob->form),
					formEdid);
			}
		}

		void RunPhaseB()
		{
			const auto& rawFilter = MCM::Settings::Diagnostic::sDefaultObjectProbeFilter.GetValue();
			const auto filters = SplitFilter(rawFilter);

			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) {
				REX::WARN("AVProbe: phase=B skipped; TESDataHandler unavailable");
				return;
			}

			auto& avifs = dh->GetFormArray<RE::ActorValueInfo>();
			auto& globs = dh->GetFormArray<RE::TESGlobal>();
			auto& mgefs = dh->GetFormArray<RE::EffectSetting>();

			REX::INFO("AVProbe: --- begin phase=B filter='{}' avifArray={} globArray={} mgefArray={} ---",
				rawFilter,
				avifs.size(),
				globs.size(),
				mgefs.size());

			auto* player = GetSnapshotPlayer();
			std::size_t avifMatched = 0;
			std::size_t globMatched = 0;
			std::size_t mgefMatched = 0;
			std::size_t dfobMatched = 0;
			bool dfobUnavailable = false;

			for (auto* avif : avifs) {
				if (!avif || !NameMatches(SafeEditorID(avif), filters)) {
					continue;
				}
				++avifMatched;
				LogAVIFCandidate(*avif, player);
			}

			for (auto* glob : globs) {
				if (!glob || !NameMatches(SafeEditorID(glob), filters)) {
					continue;
				}
				++globMatched;
				REX::INFO("AVProbe: DO-CANDIDATE kind=GLOB formID={:08X} edid='{}' value={}",
					glob->formID,
					SafeEditorID(glob),
					glob->value);
			}

			for (auto* mgef : mgefs) {
				if (!mgef || !MatchesMGEF(*mgef, filters)) {
					continue;
				}
				++mgefMatched;
				const auto& data = mgef->data;
				REX::INFO("AVProbe: DO-CANDIDATE kind=MGEF formID={:08X} edid='{}' baseCost={} primaryAV='{}' secondaryAV='{}' archetype={} flags=0x{:08X}",
					mgef->formID,
					SafeEditorID(mgef),
					data.baseCost,
					SafeEditorID(data.primaryAV),
					SafeEditorID(data.secondaryAV),
					static_cast<std::int32_t>(data.archetype.get()),
					static_cast<std::uint32_t>(data.flags.get()));
			}

			RunOptionalDFOBSweep(*dh, filters, dfobMatched, dfobUnavailable);

			REX::INFO("AVProbe: --- end phase=B avifMatched={} globMatched={} mgefMatched={} dfobMatched={} dfobUnavailable={} ---",
				avifMatched,
				globMatched,
				mgefMatched,
				dfobMatched,
				dfobUnavailable ? 1 : 0);
		}

		bool ShouldRerun(const char* a_reason)
		{
			return MCM::Settings::Diagnostic::bActorValueProbeRerunOnPause.GetValue() &&
			       std::string_view{ SafeString(a_reason) } == "PauseMenu";
		}
	}

	void MaybeRun(const char* a_reason)
	{
		if (!MCM::Settings::Diagnostic::bActorValueProbe.GetValue()) {
			return;
		}

		const bool rerun = ShouldRerun(a_reason);
		if (MCM::Settings::Diagnostic::bActorValueProbePhaseA.GetValue() &&
		    (!g_ranPhaseA || rerun)) {
			RunPhaseA();
			g_ranPhaseA = true;
		}
		if (MCM::Settings::Diagnostic::bActorValueProbePhaseB.GetValue() &&
		    (!g_ranPhaseB || rerun)) {
			RunPhaseB();
			g_ranPhaseB = true;
		}
	}
}
