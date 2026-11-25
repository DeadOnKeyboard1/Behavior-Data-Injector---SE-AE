#pragma once

namespace BDI
{
	class hkbSymbolIdMap;
	class hkbSymbolLinker;

	class CreateSymbolIdMapHook
	{
	public:
		[[nodiscard]] static bool InstallHook()
		{
			static bool installed = false;
			if (installed) {
				return true;
			}

			try {
				const auto runtime = REL::Module::get().version();

				// Original universal BDI relocation:
				//   Skyrim SE family: Address Library ID 62640, call offset +0x22A
				//   Skyrim AE family: Address Library ID 63585, call offset +0x237
				//
				// No hard executable-version whitelist is used here.
				REL::Relocation<std::uintptr_t> createSymbolIdMapBase{ REL::RelocationID(62640, 63585) };
				const auto hookAddress =
					createSymbolIdMapBase.address() + REL::Relocate(0x22A, 0x237);

				// Never blindly patch an unexpected instruction on any runtime.
				const auto opcode = *reinterpret_cast<const std::uint8_t*>(hookAddress);
				if (opcode != 0xE8) {
					WARN(
						"{}: unsupported hook layout on Skyrim {}. Expected CALL opcode 0xE8 at 0x{:X}, found 0x{:02X}. Hook was not installed.",
						__FUNCTION__,
						runtime.string(),
						hookAddress,
						opcode);
					return false;
				}

				// Skyrim 1.7.104.0: Validate CALL opcode and log whether this is a direct
				// call or a chained hook from another animation plugin (such as AMR/OAR).
				if (runtime == REL::Version{ 1, 7, 104, 0 }) {
					REL::Relocation<std::uintptr_t> expectedCreateSymbolIdMap{ REL::ID(63845) };

					std::int32_t displacement{};
					std::memcpy(
						std::addressof(displacement),
						reinterpret_cast<const void*>(hookAddress + 1),
						sizeof(displacement));

					const auto actualTarget =
						static_cast<std::uintptr_t>(
							static_cast<std::intptr_t>(hookAddress + 5) +
							static_cast<std::intptr_t>(displacement));

					if (actualTarget == expectedCreateSymbolIdMap.address()) {
						INFO(
							"Skyrim 1.7.104.0 direct hook target confirmed: patch 0x{:X}, target 0x{:X} (Address Library ID 63845).",
							hookAddress,
							actualTarget);
					} else {
						INFO(
							"Skyrim 1.7.104.0 chained hook detected: patch 0x{:X}, existing target 0x{:X} (previous trampoline), expected base ID 63845 at 0x{:X}. Chaining hook successfully.",
							hookAddress,
							actualTarget,
							expectedCreateSymbolIdMap.address());
					}
				} else {
					INFO(
						"Runtime {} passed the universal BDI call-site check at 0x{:X}.",
						runtime.string(),
						hookAddress);
				}

				SKSE::AllocTrampoline(1 << 4);
				auto& trampoline = SKSE::GetTrampoline();
				_CreateSymbolIdMap = trampoline.write_call<5>(hookAddress, CreateSymbolIdMap);

				INFO("{} Done on Skyrim {}.", __FUNCTION__, runtime.string());
				installed = true;
				return true;
			} catch (const std::exception& ex) {
				ERROR("{}: Exception caught during hook installation: {}", __FUNCTION__, ex.what());
				return false;
			}
		}

	private:
		static hkbSymbolIdMap* CreateSymbolIdMap(
			RE::hkbBehaviorGraph* a_masterGraph,
			const RE::BSScrapArray<RE::hkbBehaviorGraph*>& a_graphArr,
			const char* a_projectPath,
			RE::hkbCharacter* a_character,
			hkbSymbolLinker* eventLinker,
			hkbSymbolLinker* variableLinker);

		static inline REL::Relocation<decltype(CreateSymbolIdMap)> _CreateSymbolIdMap;
	};
}
