#include "PCH.h"

#include "Tweaks/GameSettings.h"

#include "Diagnostics/Logging.h"
#include "Settings.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace Tweaks::GameSettings
{
	namespace
	{
		bool g_warnedMissingCollection = false;

		// Per-process state, keyed by GMST name. GMST names are globally
		// unique, so a single map is enough across modules.
		struct FloatState
		{
			float baseline = 0.0f;
			bool  hasSnapshot = false;
			float lastUser = 0.0f;
			bool  hasLast = false;
		};
		struct IntState
		{
			std::int32_t baseline = 0;
			bool         hasSnapshot = false;
			std::int32_t lastUser = 0;
			bool         hasLast = false;
		};

		std::unordered_map<std::string, FloatState> g_floatState;
		std::unordered_map<std::string, IntState>   g_intState;
		std::unordered_set<std::string>             g_warnedKeys;

		bool MaterialDelta(float a_prev, float a_now)
		{
			const float scale = std::max(1.0f, std::max(std::abs(a_prev), std::abs(a_now)));
			return std::abs(a_prev - a_now) > 1e-4f * scale;
		}

		bool MaterialDelta(std::int32_t a_prev, std::int32_t a_now)
		{
			return a_prev != a_now;
		}

		RE::Setting* LookupSetting(const char* a_name)
		{
			auto* coll = RE::GameSettingCollection::GetSingleton();
			if (!coll) {
				if (!g_warnedMissingCollection) {
					REX::WARN("GameSettings: GameSettingCollection unavailable; skipping");
					g_warnedMissingCollection = true;
				}
				return nullptr;
			}
			return coll->GetSetting(a_name);
		}

		bool WarnKey(std::string_view a_module, std::string_view a_key, std::string_view a_msg)
		{
			std::string composite;
			composite.reserve(a_module.size() + 1 + a_key.size());
			composite.append(a_module);
			composite.push_back('|');
			composite.append(a_key);
			if (g_warnedKeys.find(composite) != g_warnedKeys.end()) {
				return false;
			}
			g_warnedKeys.insert(std::move(composite));
			REX::WARN("{}: {} ({})", a_module, a_msg, a_key);
			return true;
		}

		template <typename T>
		T Clamp(T a_value, const std::optional<T>& a_min, const std::optional<T>& a_max)
		{
			if (a_min && a_value < *a_min) a_value = *a_min;
			if (a_max && a_value > *a_max) a_value = *a_max;
			return a_value;
		}

		template <typename T>
		bool NearlyEqual(T a, T b)
		{
			if constexpr (std::is_floating_point_v<T>) {
				return !MaterialDelta(a, b);
			} else {
				return a == b;
			}
		}
	}

	void WarnOnce(std::string_view a_module, std::string_view a_key, std::string_view a_message)
	{
		WarnKey(a_module, a_key, a_message);
	}

	ApplyResult Apply(std::string_view a_module, std::span<const FloatTarget> a_targets)
	{
		ApplyResult result;
		const bool  trace = MCM::Settings::Diagnostic::bGameSettingsTrace.GetValue();
		const bool  audit = Diagnostics::Logging::ValidationAuditEnabled();
		const bool  auditFull = audit &&
			Diagnostics::Logging::ValidationAuditMode() == Diagnostics::Logging::AuditMode::Full;

		std::size_t passed = 0;
		std::size_t failed = 0;

		for (const auto& t : a_targets) {
			auto* s = LookupSetting(t.gmst);
			if (!s || s->GetType() != RE::Setting::SETTING_TYPE::kFloat) {
				WarnKey(a_module, t.gmst, "GMST missing or non-float; skipping");
				++result.skipped;
				if (auditFull) {
					REX::INFO("HRVERIFY module={} target={} type=float mode={} result=SKIP reason=missing_or_wrong_type",
						a_module, t.gmst,
						t.mode == Mode::Multiplier ? "mult" : "direct");
				}
				continue;
			}

			auto& state = g_floatState[t.gmst];
			if (!state.hasSnapshot) {
				state.baseline = s->GetFloat();
				state.hasSnapshot = true;
				if (trace) {
					REX::INFO("  GameSettings[{}] snapshot {} = {}", a_module, t.gmst, state.baseline);
				}
			}

			const float user = Clamp<float>(t.setting->GetValue(), t.minClamp, t.maxClamp);
			float       wanted = state.baseline;
			switch (t.mode) {
				case Mode::Multiplier:
					wanted = state.baseline * user;
					break;
				case Mode::Direct:
					if (t.preserveBaselineAtNeutral && NearlyEqual(user, t.neutral)) {
						wanted = state.baseline;
					} else {
						wanted = user;
					}
					break;
			}

			s->SetFloat(wanted);
			++result.applied;

			const float readback = s->GetFloat();
			const bool  pass = NearlyEqual(readback, wanted);
			if (pass) ++passed; else ++failed;

			if (!state.hasLast || MaterialDelta(state.lastUser, user)) {
				++result.changed;
				state.lastUser = user;
				state.hasLast = true;
			}

			if (trace) {
				REX::INFO("  GameSettings[{}] {} mode={} baseline={} user={} wrote={} readback={}",
					a_module, t.gmst,
					t.mode == Mode::Multiplier ? "mult" : "direct",
					state.baseline, user, wanted, readback);
			}

			if (auditFull) {
				REX::INFO("HRVERIFY module={} target={} type=float mode={} baseline={:g} user={:g} expected={:g} readback={:g} result={}",
					a_module, t.gmst,
					t.mode == Mode::Multiplier ? "mult" : "direct",
					state.baseline, user, wanted, readback,
					pass ? "PASS" : "FAIL");
			}
		}

		if (result.changed > 0 || trace) {
			REX::INFO("GameSettings[{}]: applied {} target(s), {} changed, {} skipped",
				a_module, result.applied, result.changed, result.skipped);
		}

		if (audit) {
			const bool overallPass = (failed == 0) && (result.skipped == 0);
			REX::INFO("HRVERIFY_SUMMARY module={} type=float applied={} passed={} failed={} skipped={} result={}",
				a_module, result.applied, passed, failed, result.skipped,
				overallPass ? "PASS" : "FAIL");
		}
		return result;
	}

	ApplyResult Apply(std::string_view a_module, std::span<const IntTarget> a_targets)
	{
		ApplyResult result;
		const bool  trace = MCM::Settings::Diagnostic::bGameSettingsTrace.GetValue();
		const bool  audit = Diagnostics::Logging::ValidationAuditEnabled();
		const bool  auditFull = audit &&
			Diagnostics::Logging::ValidationAuditMode() == Diagnostics::Logging::AuditMode::Full;

		std::size_t passed = 0;
		std::size_t failed = 0;

		for (const auto& t : a_targets) {
			auto* s = LookupSetting(t.gmst);
			if (!s || s->GetType() != RE::Setting::SETTING_TYPE::kInt) {
				WarnKey(a_module, t.gmst, "GMST missing or non-int; skipping");
				++result.skipped;
				if (auditFull) {
					REX::INFO("HRVERIFY module={} target={} type=int mode={} result=SKIP reason=missing_or_wrong_type",
						a_module, t.gmst,
						t.mode == Mode::Multiplier ? "mult" : "direct");
				}
				continue;
			}

			auto& state = g_intState[t.gmst];
			if (!state.hasSnapshot) {
				state.baseline = s->GetInt();
				state.hasSnapshot = true;
				if (trace) {
					REX::INFO("  GameSettings[{}] snapshot {} = {}", a_module, t.gmst, state.baseline);
				}
			}

			const std::int32_t user = Clamp<std::int32_t>(t.setting->GetValue(), t.minClamp, t.maxClamp);
			std::int32_t       wanted = state.baseline;
			switch (t.mode) {
				case Mode::Multiplier: {
					// Round-to-nearest baseline*user for int targets. Multiplier
					// mode is rarely useful for ints, but supported for symmetry.
					const double scaled = static_cast<double>(state.baseline) * static_cast<double>(user);
					const double bounded = std::clamp(
						scaled,
						static_cast<double>(std::numeric_limits<std::int32_t>::min()),
						static_cast<double>(std::numeric_limits<std::int32_t>::max()));
					wanted = static_cast<std::int32_t>(std::lround(bounded));
					break;
				}
				case Mode::Direct:
					if (t.preserveBaselineAtNeutral && user == t.neutral) {
						wanted = state.baseline;
					} else {
						wanted = user;
					}
					break;
			}

			s->SetInt(wanted);
			++result.applied;

			const std::int32_t readback = s->GetInt();
			const bool         pass = (readback == wanted);
			if (pass) ++passed; else ++failed;

			if (!state.hasLast || MaterialDelta(state.lastUser, user)) {
				++result.changed;
				state.lastUser = user;
				state.hasLast = true;
			}

			if (trace) {
				REX::INFO("  GameSettings[{}] {} mode={} baseline={} user={} wrote={} readback={}",
					a_module, t.gmst,
					t.mode == Mode::Multiplier ? "mult" : "direct",
					state.baseline, user, wanted, readback);
			}

			if (auditFull) {
				REX::INFO("HRVERIFY module={} target={} type=int mode={} baseline={} user={} expected={} readback={} result={}",
					a_module, t.gmst,
					t.mode == Mode::Multiplier ? "mult" : "direct",
					state.baseline, user, wanted, readback,
					pass ? "PASS" : "FAIL");
			}
		}

		if (result.changed > 0 || trace) {
			REX::INFO("GameSettings[{}]: applied {} target(s), {} changed, {} skipped",
				a_module, result.applied, result.changed, result.skipped);
		}

		if (audit) {
			const bool overallPass = (failed == 0) && (result.skipped == 0);
			REX::INFO("HRVERIFY_SUMMARY module={} type=int applied={} passed={} failed={} skipped={} result={}",
				a_module, result.applied, passed, failed, result.skipped,
				overallPass ? "PASS" : "FAIL");
		}
		return result;
	}
}
