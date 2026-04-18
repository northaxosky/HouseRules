#include "PCH.h"

#include "Hooks/Unlocks.h"
#include "Settings.h"

namespace
{
	void InitLogger()
	{
		auto path = F4SE::log::log_directory();
		if (!path) {
			return;
		}
		*path /= "SurvivalArchitect.log";

		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
		auto log = std::make_shared<spdlog::logger>("global", std::move(sink));
		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::info);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
	}

	void MessageHandler(F4SE::MessagingInterface::Message* a_msg)
	{
		if (!a_msg) {
			return;
		}

		switch (a_msg->type) {
			case F4SE::MessagingInterface::kGameDataReady:
				MCM::Settings::Update();
				break;
			default:
				break;
		}
	}
}

F4SE_EXPORT bool F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);
	InitLogger();

	spdlog::info("Survival Architect loading");

	const auto messaging = F4SE::GetMessagingInterface();
	if (!messaging || !messaging->RegisterListener(MessageHandler)) {
		spdlog::error("failed to register message listener");
		return false;
	}

	Hooks::Unlocks::Install();

	spdlog::info("Survival Architect loaded");
	return true;
}
