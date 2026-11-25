#include "DataHandler.h"

namespace BDI
{
	static auto MainConfig = COMPILE_PROXY("BehaviorDataInjector.ini"sv);

	DataHandler::DataHandler()
	{
		MainConfig.Bind(enableDebugLog, false);
		MainConfig.Load();

		if (enableDebugLog.get_data()) {
			spdlog::set_level(spdlog::level::debug);
			DEBUG("Debug log enabled via BehaviorDataInjector.ini.");
		}

		auto BDIFilesName = DKUtil::Config::GetAllFiles<true>("Data\\SKSE\\Plugins\\BehaviorDataInjector"sv, ".json"sv, {}, "_BDI"sv);
		for (const auto& fileName : BDIFilesName) {
			INFO("Parsing BDI Data In \"{}\"", fileName);
			try {
				std::ifstream fileStream{ std::filesystem::path(fileName) };
				if (!fileStream.is_open()) {
					ERROR("Failed to open BDI config file \"{}\"", fileName);
					continue;
				}

				auto js = json::parse(fileStream);
				if (!js.is_array()) {
					ERROR("BDI config file \"{}\" is invalid: top-level JSON must be an array.", fileName);
					continue;
				}

				for (auto& element : js) {
					try {
						auto obj = element.get<BDIObject>();
						if (obj.type == BDIDataTypes::kNone) {
							continue;
						}

						if (obj.projectPath.empty()) {
							ERROR("BDI entry in file \"{}\" has empty projectPath. Skipping.", fileName);
							continue;
						}

						if (obj.name.empty()) {
							ERROR("BDI entry in file \"{}\" has empty name. Skipping.", fileName);
							continue;
						}

						// Normalize project path: convert '/' to '\\' and uppercase
						for (char& c : obj.projectPath) {
							if (c == '/') {
								c = '\\';
							} else {
								c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
							}
						}

						// Strip any leading and trailing backslashes/slashes
						while (!obj.projectPath.empty() && (obj.projectPath.front() == '\\' || obj.projectPath.front() == '/')) {
							obj.projectPath.erase(obj.projectPath.begin());
						}
						while (!obj.projectPath.empty() && (obj.projectPath.back() == '\\' || obj.projectPath.back() == '/')) {
							obj.projectPath.pop_back();
						}

						if (obj.projectPath.empty()) {
							ERROR("BDI entry in file \"{}\" has empty normalized projectPath. Skipping.", fileName);
							continue;
						}

						objMap[obj.projectPath].AddObject(std::move(obj));
					} catch (const std::exception& ex) {
						ERROR("Failed to parse BDI entry in file \"{}\": {}", fileName, ex.what());
					}
				}
			} catch (const json::exception& ex) {
				ERROR("Caught JSON exception when parsing file \"{}\":\n {}", fileName, ex.what());
			} catch (const std::exception& ex) {
				ERROR("Caught standard exception when opening file \"{}\":\n {}", fileName, ex.what());
			}
		}
	}

	void DataHandler::BDIObjArray::AddObject(BDIObject a_obj)
	{
		switch (a_obj.type) {
		case BDIDataTypes::kBool:
		case BDIDataTypes::kInt:
		case BDIDataTypes::kFloat:
			for (const auto& existing : Objects[Type::kVariable]) {
				if (_stricmp(existing.name.c_str(), a_obj.name.c_str()) == 0) {
					DEBUG("Variable \"{}\" already registered for path \"{}\", skipping duplicate.", a_obj.name, a_obj.projectPath);
					return;
				}
			}
			INFO("Stored a BDI variable: name: \"{}\", type: \"{}\", projectPath: \"{}\".",
				a_obj.name, BDITypeEnumTbl.to_string(a_obj.type), a_obj.projectPath);
			Objects[Type::kVariable].push_back(std::move(a_obj));
			break;

		case BDIDataTypes::kEvent:
			for (const auto& existing : Objects[Type::kEvent]) {
				if (_stricmp(existing.name.c_str(), a_obj.name.c_str()) == 0) {
					DEBUG("Event \"{}\" already registered for path \"{}\", skipping duplicate.", a_obj.name, a_obj.projectPath);
					return;
				}
			}
			INFO("Stored a BDI event: name: \"{}\", type: \"{}\", projectPath: \"{}\".",
				a_obj.name, BDITypeEnumTbl.to_string(a_obj.type), a_obj.projectPath);
			Objects[Type::kEvent].push_back(std::move(a_obj));
			break;

		default:
			break;
		}
	}

	void DataHandler::BDIObjArray::InjectVariables(RE::hkbBehaviorGraph* a_graph) const
	{
		if (a_graph && a_graph->data && a_graph->data->stringData && a_graph->data->stringData->variableNames.data()) {
			const char* graphName = a_graph->name.c_str() ? a_graph->name.c_str() : "<unnamed>";
			for (const auto& obj : Objects[Type::kVariable]) {
				int result = -1;
				switch (obj.type) {
				case BDIDataTypes::kBool:
					result = a_graph->data->AddBoolVariable(RE::hkStringPtr{ obj.name }, obj.value.b);
					break;

				case BDIDataTypes::kInt:
					result = a_graph->data->AddIntVariable(RE::hkStringPtr{ obj.name }, obj.value.i);
					break;

				case BDIDataTypes::kFloat:
					result = a_graph->data->AddFloatVariable(RE::hkStringPtr{ obj.name }, obj.value.f);
					break;

				default:
					break;
				}

				if (result >= 0) {
					DEBUG("Injected a variable \"{}\" to graph \"{}\" successfully", obj.name, graphName);
				} else {
					DEBUG("Variable \"{}\" already exists in graph \"{}\" or failed to inject", obj.name, graphName);
				}
			}
		}
	}

	void DataHandler::BDIObjArray::InjectEvents(RE::hkbBehaviorGraph* a_graph) const
	{
		if (a_graph && a_graph->data && a_graph->data->stringData && a_graph->data->stringData->eventNames.data()) {
			const char* graphName = a_graph->name.c_str() ? a_graph->name.c_str() : "<unnamed>";
			for (const auto& obj : Objects[Type::kEvent]) {
				if (a_graph->data->AddEvent(RE::hkStringPtr{ obj.name }) >= 0) {
					DEBUG("Injected an event \"{}\" to graph \"{}\" successfully", obj.name, graphName);
				} else {
					DEBUG("Event \"{}\" already exists in graph \"{}\" or failed to inject", obj.name, graphName);
				}
			}
		}
	}
}
