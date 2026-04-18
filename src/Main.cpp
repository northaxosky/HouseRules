#include "PCH.h"

#include "Hooks/Unlocks.h"
#include "Settings.h"

namespace
{
	class MenuSink :
		public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static MenuSink* GetSingleton()
		{
			static MenuSink instance;
			return std::addressof(instance);
		}

		RE::BSEventNotifyControl ProcessEvent(
			const RE::MenuOpenCloseEvent&                 a_event,
			RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
		{
			if (a_event.menuName == "PauseMenu" && !a_event.opening) {
				MCM::Settings::Update();
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void RegisterMenuSink()
	{
		const auto ui = RE::UI::GetSingleton();
		if (!ui) {
			REX::WARN("UI singleton unavailable; MCM live-reload disabled");
			return;
		}
		ui->RegisterSink<RE::MenuOpenCloseEvent>(MenuSink::GetSingleton());
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
			case F4SE::MessagingInterface::kGameLoaded:
				RegisterMenuSink();
				break;
			default:
				break;
		}
	}
}

F4SE_EXPORT bool F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se, { .logName = "SurvivalArchitect" });

	REX::INFO("Survival Architect loading");

	const auto messaging = F4SE::GetMessagingInterface();
	if (!messaging || !messaging->RegisterListener(MessageHandler)) {
		REX::ERROR("failed to register message listener");
		return false;
	}

	Hooks::Unlocks::Install();

	REX::INFO("Survival Architect loaded");
	return true;
}
