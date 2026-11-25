#include "LoadGame.h"

SKSEPluginInfo(
	.Version = Plugin::Version,
	.Name = Plugin::NAME,
	.Author = Plugin::AUTHOR,

	// CommonLibSSE-NG multi-runtime builds support the pre-1.6.629 and
	// post-1.6.629 structure layouts in one DLL. Explicitly declare that
	// compatibility instead of setting the old "post-629 only" flag.
	.StructCompatibility = SKSE::StructCompatibility::Independent,

	// No executable-version whitelist: resolve supported SE/AE runtimes
	// through the matching Address Library database.
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
#endif

	REL::Module::reset();
	SKSE::Init(a_skse);

	const auto runtime = REL::Module::get().version();
	DKUtil::Logger::Init(Plugin::NAME, runtime.string());

	// BDI's native hook has only SE/AE relocation data. CommonLib's official
	// prebuilt library itself also contains VR support, so reject VR explicitly
	// before registering any callback or touching the BDI hook.
	if (REL::Module::IsVR()) {
		WARN("Skyrim VR is not supported by this Behavior Data Injector build.");
		return false;
	}

	INFO("{} v{} loaded on Skyrim {}", Plugin::NAME, Plugin::Version, runtime.string());

	if (runtime == REL::Version{ 1, 7, 104, 0 }) {
		INFO("Skyrim 1.7.104.0 detected: enabling the additionally verified hook-target check.");
	} else {
		INFO("Runtime {} detected: using the universal SE/AE Address Library path.", runtime.string());
	}

	auto g_message = SKSE::GetMessagingInterface();
	if (!g_message) {
		WARN("Messaging Interface Not Found!");
		return false;
	}

	if (!g_message->RegisterListener(BDI::EventCallback)) {
		WARN("Could not register the SKSE messaging listener.");
		return false;
	}

	return true;
}
