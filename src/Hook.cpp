#include "Hook.h"
#include "DataHandler.h"

namespace BDI
{
	hkbSymbolIdMap* CreateSymbolIdMapHook::CreateSymbolIdMap(RE::hkbBehaviorGraph* a_masterGraph, const RE::BSScrapArray<RE::hkbBehaviorGraph*>& a_graphArr, const char* a_projectPath, RE::hkbCharacter* a_character, hkbSymbolLinker* eventLinker, hkbSymbolLinker* variableLinker)
	{
		DEBUG("{} Fired!", __FUNCTION__);

		// a_projectPath is supplied by the engine. Keep injection strictly optional:
		// if any input required by BDI is absent, call the original function untouched.
		if (a_masterGraph &&
			a_character &&
			a_projectPath &&
			*a_projectPath != '\0' &&
			a_masterGraph->data &&
			!a_masterGraph->eventIDMap &&
			!a_masterGraph->variableIDMap) {
			auto dataHandler = DataHandler::GetSingleton();
			auto matchingArrays = dataHandler->GetMatchingArrays(a_projectPath);
			if (!matchingArrays.empty()) {
				const char* projectName = a_masterGraph->name.c_str() ? a_masterGraph->name.c_str() : "<unnamed>";
				DEBUG("Injecting Data for project: \"{}\" in path \"{}\"", projectName, a_projectPath);
				for (const auto* objArray : matchingArrays) {
					if (objArray) {
						objArray->InjectVariables(a_masterGraph);
						for (auto graph : a_graphArr) {
							if (graph) {
								objArray->InjectEvents(graph);
							}
						}
					}
				}
			}
		}

		return _CreateSymbolIdMap(a_masterGraph, a_graphArr, a_projectPath, a_character, eventLinker, variableLinker);
	}
}
