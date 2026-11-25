#include "LoadGame.h"
#include "ConsoleCommands.h"
#include "DataHandler.h"
#include "Hook.h"

namespace BDI
{
	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (!msg || msg->type != SKSE::MessagingInterface::kPostLoad) {
			return;
		}

		auto dataHandler = DataHandler::GetSingleton();
		if (dataHandler->enableDebugLog.get_data()) {
			spdlog::set_level(spdlog::level::debug);
			DEBUG("Enable Debug Log!");
		}

		ConsoleCommands::Register();

		if (!CreateSymbolIdMapHook::InstallHook()) {
			WARN("Behavior Data Injector hook was not installed. Injection is disabled for this session.");
		}
	}
}
