#pragma once

#include <cstdint>

namespace RE
{
	class MenuOpenCloseEvent;
}

namespace Diagnostics::SurvivalObserver
{
	void Install();
	void OnF4SEMessage(std::uint32_t a_messageType);
	void OnMenuOpenClose(const RE::MenuOpenCloseEvent& a_event);
}
