#include "PCH.h"

#include "Diagnostics/SurvivalObserver.h"

#include "Settings.h"

#include <array>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace Diagnostics::SurvivalObserver
{
	namespace
	{
		// Vanilla survival needs are driven by Hardcore:HC_ManagerScript's Papyrus
		// service quest. This observer only correlates native AV/events with that
		// known cadence; it does not claim ownership of the accrual loop itself.
		struct TrackedActorValue
		{
			const char*                  label;
			RE::ActorValueInfo* RE::ActorValue::* member;
		};

		constexpr std::array kTrackedActorValues{
			TrackedActorValue{ "resourceFood", &RE::ActorValue::resourceFood },
			TrackedActorValue{ "resourceWater", &RE::ActorValue::resourceWater },
			TrackedActorValue{ "fatigue", &RE::ActorValue::fatigue },
			TrackedActorValue{ "fatigueAPMax", &RE::ActorValue::fatigueAPMax },
			TrackedActorValue{ "restoreFatigueRate", &RE::ActorValue::restoreFatigueRate }
		};

		bool ObserverEnabled()
		{
			return MCM::Settings::Diagnostic::bSurvivalObserver.GetValue();
		}

		bool LifecycleLoggingEnabled()
		{
			return ObserverEnabled() &&
			       MCM::Settings::Diagnostic::bSurvivalObserverLifecycle.GetValue();
		}

		std::vector<std::string> SplitFilter(const std::string& a_raw)
		{
			std::vector<std::string> out;
			std::size_t              start = 0;
			for (std::size_t i = 0; i <= a_raw.size(); ++i) {
				if (i == a_raw.size() || a_raw[i] == ',') {
					auto token = a_raw.substr(start, i - start);
					while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front()))) token.erase(token.begin());
					while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back()))) token.pop_back();
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

		const TrackedActorValue* FindTrackedActorValue(const RE::ActorValueInfo& a_info)
		{
			const auto* actorValues = RE::ActorValue::GetSingleton();
			if (!actorValues) {
				return nullptr;
			}
			for (const auto& tracked : kTrackedActorValues) {
				if (actorValues->*tracked.member == std::addressof(a_info)) {
					return std::addressof(tracked);
				}
			}
			return nullptr;
		}

		class Observer :
			public RE::BSTEventSink<RE::ActorValueEvents::ActorValueChangedEvent>,
			public RE::BSTEventSink<RE::TESMagicEffectApplyEvent>,
			public RE::BSTEventSink<RE::HUDModeEvent>
		{
		public:
			static Observer* GetSingleton()
			{
				static Observer instance;
				return std::addressof(instance);
			}

			void Install()
			{
				if (installed) {
					return;
				}
				installed = true;
			}

			void OnF4SEMessage(std::uint32_t a_messageType)
			{
				if (!ObserverEnabled()) {
					return;
				}

				LogUnavailableSurfacesOnce();

				switch (a_messageType) {
					case F4SE::MessagingInterface::kGameLoaded:
						EnsureContextSinksRegistered();
						LogLifecycle("kGameLoaded", "context sinks registered; waiting for LoadingMenu close before binding player AV sink and correlating Papyrus-driven survival ticks");
						break;
					default:
						break;
				}
			}

			void OnMenuOpenClose(const RE::MenuOpenCloseEvent& a_event)
			{
				if (!ObserverEnabled()) {
					return;
				}

				if (a_event.menuName != "LoadingMenu" && a_event.menuName != "PauseMenu") {
					return;
				}

				EnsureContextSinksRegistered();
				LogUnavailableSurfacesOnce();
				if (LifecycleLoggingEnabled()) {
					REX::INFO("SurvivalObserver: menu='{}' opening={} enabled={} actorValues={} activeEffects={}",
						a_event.menuName,
						a_event.opening,
						ObserverEnabled(),
						MCM::Settings::Diagnostic::bSurvivalObserverActorValues.GetValue(),
						MCM::Settings::Diagnostic::bSurvivalObserverActiveEffects.GetValue());
				}

				if (a_event.opening) {
					return;
				}

				// LoadingMenu/PauseMenu close is the same safe context used elsewhere
				// in this plugin: player data and forms are warm, and we're back on the
				// main thread instead of F4SE's worker-thread load messages.
				SnapshotPlayerState(a_event.menuName == "LoadingMenu" ?
					"LoadingMenu close" :
					"PauseMenu close");
			}

			RE::BSEventNotifyControl ProcessEvent(
				const RE::ActorValueEvents::ActorValueChangedEvent& a_event,
				RE::BSTEventSource<RE::ActorValueEvents::ActorValueChangedEvent>*) override
			{
				if (!ObserverEnabled() ||
				    !MCM::Settings::Diagnostic::bSurvivalObserverActorValues.GetValue()) {
					return RE::BSEventNotifyControl::kContinue;
				}

				auto* player = RE::PlayerCharacter::GetSingleton();
				if (!player || a_event.owner != player) {
					return RE::BSEventNotifyControl::kContinue;
				}

				const auto* tracked = FindTrackedActorValue(a_event.actorValue);
				if (!tracked) {
					return RE::BSEventNotifyControl::kContinue;
				}

				REX::INFO("SurvivalObserver: ActorValueChangedEvent triggered by '{}'; re-reading tracked survival AVs",
					tracked->label);
				LogTrackedActorValues(*player, "ActorValueChangedEvent");
				return RE::BSEventNotifyControl::kContinue;
			}

			RE::BSEventNotifyControl ProcessEvent(
				const RE::TESMagicEffectApplyEvent& a_event,
				RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*) override
			{
				if (!ObserverEnabled() ||
				    !MCM::Settings::Diagnostic::bSurvivalObserverActiveEffects.GetValue()) {
					return RE::BSEventNotifyControl::kContinue;
				}

				auto* player = RE::PlayerCharacter::GetSingleton();
				if (!player || a_event.target.get() != player) {
					return RE::BSEventNotifyControl::kContinue;
				}

				// NOTE: TESForm::GetFormByID triggers the cold-mesh BSStaticTriShapeDB::Force
				// null-deref trap on OG during save load (see Magnitudes.cpp comment). Linear-scan
				// the EffectSetting form array instead — same safe pattern used by Magnitudes.
				const RE::EffectSetting* effect = nullptr;
				if (auto* dh = RE::TESDataHandler::GetSingleton()) {
					for (auto* mgef : dh->GetFormArray<RE::EffectSetting>()) {
						if (mgef && mgef->formID == a_event.magicEffectFormID) {
							effect = mgef;
							break;
						}
					}
				}
				const auto* caster = a_event.caster.get();
				const auto& filters = GetFilters();
				if (!NameMatches(SafeEditorID(effect), filters) &&
				    !NameMatches(SafeEditorID(caster), filters)) {
					return RE::BSEventNotifyControl::kContinue;
				}

				REX::INFO("SurvivalObserver: MGEF targetPlayer=1 effect='{}' caster='{}' formID={:08X}",
					SafeEditorID(effect),
					SafeEditorID(caster),
					a_event.magicEffectFormID);
				LogTrackedActorValues(*player, "TESMagicEffectApplyEvent");
				return RE::BSEventNotifyControl::kContinue;
			}

			RE::BSEventNotifyControl ProcessEvent(
				const RE::HUDModeEvent& a_event,
				RE::BSTEventSource<RE::HUDModeEvent>*) override
			{
				if (!LifecycleLoggingEnabled()) {
					return RE::BSEventNotifyControl::kContinue;
				}

				std::string joined;
				if (a_event.currentHUDModes) {
					for (const auto& mode : *a_event.currentHUDModes) {
						if (!joined.empty()) {
							joined += ",";
						}
						joined += mode.modeString.c_str();
					}
				}
				REX::INFO("SurvivalObserver: HUD modes='{}'", joined);
				return RE::BSEventNotifyControl::kContinue;
			}

		private:
			const std::vector<std::string>& GetFilters()
			{
				const auto& raw = MCM::Settings::Diagnostic::sSurvivalObserverFilter.GetValue();
				if (cachedFilterRaw != raw) {
					cachedFilterRaw = raw;
					cachedFilters = SplitFilter(raw);
				}
				return cachedFilters;
			}

			void LogUnavailableSurfacesOnce()
			{
				if (loggedUnavailableSurfaces) {
					return;
				}
				loggedUnavailableSurfaces = true;

				// CommonLibF4 exposes safe public sources for TESMagicEffectApplyEvent and
				// HUDModeEvent, but not for PlayerActiveEffectChanged. QueueSurvivalBumpDown-
				// Message also lacks a public observer surface here, so this module stops at
				// conservative context logging instead of guessing at HUD internals.
				REX::INFO("SurvivalObserver: using ActorValueChangedEvent + TESMagicEffectApplyEvent + HUDModeEvent to correlate the Papyrus HC_ManagerScript cadence (sustenance 0.1h, disease 0.333332986h, sleep 14h); skipping direct PlayerActiveEffectChanged and QueueSurvivalBumpDownMessage hooks");
			}

			void LogLifecycle(const char* a_phase, const char* a_detail) const
			{
				if (!LifecycleLoggingEnabled()) {
					return;
				}
				REX::INFO("SurvivalObserver: phase='{}' {}", a_phase, a_detail);
			}

			void EnsureContextSinksRegistered()
			{
				if (!ObserverEnabled()) {
					return;
				}

				if (!registeredMagicEffectSink) {
					if (auto* source = RE::TESMagicEffectApplyEvent::GetEventSource()) {
						source->RegisterSink(static_cast<RE::BSTEventSink<RE::TESMagicEffectApplyEvent>*>(this));
						registeredMagicEffectSink = true;
						LogLifecycle("context-sink", "registered TESMagicEffectApplyEvent sink");
					}
				}
				if (!registeredHUDModeSink) {
					if (auto* source = RE::HUDModeEvent::GetEventSource()) {
						source->RegisterSink(static_cast<RE::BSTEventSink<RE::HUDModeEvent>*>(this));
						registeredHUDModeSink = true;
						LogLifecycle("context-sink", "registered HUDModeEvent sink");
					}
				}
			}

			void TryRegisterPlayerActorValueSink()
			{
				if (!ObserverEnabled() ||
				    !MCM::Settings::Diagnostic::bSurvivalObserverActorValues.GetValue()) {
					return;
				}

				auto* player = RE::PlayerCharacter::GetSingleton();
				if (!player) {
					LogLifecycle("player-sink", "PlayerCharacter singleton unavailable");
					return;
				}
				if (observedPlayer == player) {
					return;
				}

				if (observedPlayer) {
					REX::INFO("SurvivalObserver: player singleton changed; re-binding actor-value sink");
				}

				static_cast<RE::BSTEventSource<RE::ActorValueEvents::ActorValueChangedEvent>*>(player)
					->RegisterSink(static_cast<RE::BSTEventSink<RE::ActorValueEvents::ActorValueChangedEvent>*>(this));
				observedPlayer = player;
				LogLifecycle("player-sink", "actor-value sink registered on PlayerCharacter");
			}

			void SnapshotPlayerState(const char* a_reason)
			{
				TryRegisterPlayerActorValueSink();

				auto* player = RE::PlayerCharacter::GetSingleton();
				if (!player) {
					REX::WARN("SurvivalObserver: {} skipped; PlayerCharacter singleton unavailable", a_reason);
					return;
				}

				if (LifecycleLoggingEnabled()) {
					REX::INFO("SurvivalObserver: snapshot reason='{}' difficulty={} godMode={} immortal={}",
						a_reason,
						static_cast<std::uint32_t>(player->GetDifficultyLevel()),
						player->IsGodMode(),
						player->IsImmortal());
				}

				if (MCM::Settings::Diagnostic::bSurvivalObserverActorValues.GetValue()) {
					LogTrackedActorValues(*player, a_reason);
				}
				if (MCM::Settings::Diagnostic::bSurvivalObserverActiveEffects.GetValue()) {
					LogActiveEffects(*player, a_reason);
				}
			}

			void LogTrackedActorValues(RE::PlayerCharacter& a_player, const char* a_reason) const
			{
				const auto* actorValues = RE::ActorValue::GetSingleton();
				if (!actorValues) {
					REX::WARN("SurvivalObserver: {} skipped actor values; ActorValue singleton unavailable", a_reason);
					return;
				}

				for (const auto& tracked : kTrackedActorValues) {
					const auto* info = actorValues->*tracked.member;
					if (!info) {
						REX::WARN("SurvivalObserver: {} '{}' unavailable on this runtime", a_reason, tracked.label);
						continue;
					}
					LogActorValue(a_player, *info, tracked.label, a_reason);
				}
			}

			void LogActorValue(
				const RE::PlayerCharacter& a_player,
				const RE::ActorValueInfo&  a_info,
				const char*                a_label,
				const char*                a_reason) const
			{
				const auto current = a_player.GetActorValue(a_info);
				const auto permanentValue = a_player.GetPermanentActorValue(a_info);
				const auto base = a_player.GetBaseActorValue(a_info);
				const auto permanentMod = a_player.GetModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, a_info);
				const auto temporaryMod = a_player.GetModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, a_info);
				const auto damage = a_player.GetModifier(RE::ACTOR_VALUE_MODIFIER::kDamage, a_info);

				REX::INFO("SurvivalObserver: AV reason='{}' name='{}' edid='{}' current={} permanent={} base={} modPermanent={} modTemporary={} damage={}",
					a_reason,
					a_label,
					SafeEditorID(std::addressof(a_info)),
					current,
					permanentValue,
					base,
					permanentMod,
					temporaryMod,
					damage);
			}

			void LogActiveEffects(RE::PlayerCharacter& a_player, const char* a_reason) const
			{
				auto* list = a_player.GetActiveEffectList();
				if (!list) {
					REX::WARN("SurvivalObserver: {} skipped active effects; player effect list unavailable", a_reason);
					return;
				}

				const auto& rawFilter = MCM::Settings::Diagnostic::sSurvivalObserverFilter.GetValue();
				const auto& filters   = const_cast<Observer*>(this)->GetFilters();

				std::size_t matched = 0;
				for (const auto& effectHandle : list->data) {
					const auto* effect = effectHandle.get();
					if (!effect) {
						continue;
					}

					const auto* effectItem = effect->effect;
					const auto* effectSetting = effectItem ? effectItem->effectSetting : nullptr;
					const auto* spell = effect->spell;
					const auto* source = effect->source;

					const auto spellID = SafeEditorID(spell);
					const auto settingID = SafeEditorID(effectSetting);
					const auto sourceID = SafeEditorID(source);
					if (!NameMatches(spellID, filters) &&
					    !NameMatches(settingID, filters) &&
					    !NameMatches(sourceID, filters)) {
						continue;
					}

					++matched;
					REX::INFO("SurvivalObserver: AE reason='{}' spell='{}' setting='{}' source='{}' mag={} duration={} elapsed={} flags=0x{:X}",
						a_reason,
						spellID,
						settingID,
						sourceID,
						effect->magnitude,
						effect->duration,
						effect->elapsedSeconds,
						static_cast<std::uint32_t>(effect->flags.get()));
				}

				REX::INFO("SurvivalObserver: active-effect snapshot reason='{}' matched={} filter='{}'",
					a_reason,
					matched,
					rawFilter);
			}

			RE::PlayerCharacter* observedPlayer{ nullptr };
			std::string          cachedFilterRaw;
			std::vector<std::string> cachedFilters;
			bool                 installed{ false };
			bool                 loggedUnavailableSurfaces{ false };
			bool                 registeredMagicEffectSink{ false };
			bool                 registeredHUDModeSink{ false };
		};
	}

	void Install()
	{
		Observer::GetSingleton()->Install();
	}

	void OnF4SEMessage(std::uint32_t a_messageType)
	{
		Observer::GetSingleton()->OnF4SEMessage(a_messageType);
	}

	void OnMenuOpenClose(const RE::MenuOpenCloseEvent& a_event)
	{
		Observer::GetSingleton()->OnMenuOpenClose(a_event);
	}
}
