#include "PCH.h"

#include "Diagnostics/Logging.h"

#include "Settings.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>

namespace Diagnostics::Logging
{
	namespace
	{
		bool      g_auditEnabled = false;
		AuditMode g_auditMode    = AuditMode::Summary;

		bool      g_warnedBadLevel = false;
		bool      g_warnedBadAuditMode = false;

		std::string ToLower(std::string_view a_value)
		{
			std::string out;
			out.resize(a_value.size());
			std::transform(a_value.begin(), a_value.end(), out.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return out;
		}

		spdlog::level::level_enum ToSpdLevel(Level a_level)
		{
			switch (a_level) {
				case Level::Quiet:   return spdlog::level::warn;
				case Level::Normal:  return spdlog::level::info;
				case Level::Verbose: return spdlog::level::debug;
				case Level::Trace:   return spdlog::level::trace;
			}
			return spdlog::level::info;
		}

		Level ParseLevelIndex(std::int32_t a_value, bool& a_recognized)
		{
			a_recognized = true;
			switch (a_value) {
				case 0: return Level::Quiet;
				case 1: return Level::Normal;
				case 2: return Level::Verbose;
				case 3: return Level::Trace;
			}
			a_recognized = false;
			return Level::Normal;
		}
	}

	Level ParseLevel(std::string_view a_value, bool& a_recognized)
	{
		const auto lower = ToLower(a_value);
		a_recognized = true;
		if (lower == "quiet")   return Level::Quiet;
		if (lower == "normal")  return Level::Normal;
		if (lower == "verbose") return Level::Verbose;
		if (lower == "trace")   return Level::Trace;
		a_recognized = false;
		return Level::Normal;
	}

	AuditMode ParseAuditMode(std::string_view a_value, bool& a_recognized)
	{
		const auto lower = ToLower(a_value);
		a_recognized = true;
		if (lower == "summary") return AuditMode::Summary;
		if (lower == "full")    return AuditMode::Full;
		a_recognized = false;
		return AuditMode::Summary;
	}

	void ApplyLogLevel()
	{
		bool levelOk = false;
		auto level = ParseLevelIndex(MCM::Settings::Diagnostic::iLogLevel.GetValue(), levelOk);
		if (!levelOk) {
			const auto& levelStr = MCM::Settings::Diagnostic::sLogLevel.GetValue();
			level = ParseLevel(levelStr, levelOk);
			if (!levelOk && !g_warnedBadLevel) {
				REX::WARN("Diagnostic.iLogLevel={} and sLogLevel='{}' are not recognized; using Normal",
					MCM::Settings::Diagnostic::iLogLevel.GetValue(), levelStr);
				g_warnedBadLevel = true;
			}
		}

		if (auto* logger = spdlog::default_logger_raw()) {
			auto sl = ToSpdLevel(level);
			if (MCM::Settings::Diagnostic::bValidationAudit.GetValue() &&
				sl > spdlog::level::info) {
				// HRVERIFY lines are intentionally emitted at info. Audit mode
				// should remain visible even if the general log is Quiet.
				sl = spdlog::level::info;
			}
			logger->set_level(sl);
			logger->flush_on(sl);
		}

		g_auditEnabled = MCM::Settings::Diagnostic::bValidationAudit.GetValue();

		const auto& modeStr = MCM::Settings::Diagnostic::sValidationAuditMode.GetValue();
		bool modeOk = false;
		const auto mode = ParseAuditMode(modeStr, modeOk);
		if (!modeOk && !g_warnedBadAuditMode) {
			REX::WARN("Diagnostic.sValidationAuditMode='{}' not recognized; "
			          "using Summary (accepted: Summary, Full)", modeStr);
			g_warnedBadAuditMode = true;
		}
		g_auditMode = mode;
	}

	bool ValidationAuditEnabled()
	{
		return g_auditEnabled;
	}

	AuditMode ValidationAuditMode()
	{
		return g_auditMode;
	}
}
