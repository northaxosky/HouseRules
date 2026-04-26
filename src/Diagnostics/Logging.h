#pragma once

#include <string_view>

namespace Diagnostics::Logging
{
	enum class Level
	{
		Quiet,
		Normal,
		Verbose,
		Trace,
	};

	enum class AuditMode
	{
		Summary,
		Full,
	};

	// Re-read log settings from MCM::Settings::Diagnostic and push the result
	// to the spdlog default logger. Safe to call repeatedly.
	void ApplyLogLevel();

	// Cached audit toggles, refreshed inside ApplyLogLevel(). Hot paths can
	// query these without re-parsing the INI string each call.
	bool      ValidationAuditEnabled();
	AuditMode ValidationAuditMode();

	// Exposed for tests / direct callers.
	Level ParseLevel(std::string_view a_value, bool& a_recognized);
	AuditMode ParseAuditMode(std::string_view a_value, bool& a_recognized);
}
