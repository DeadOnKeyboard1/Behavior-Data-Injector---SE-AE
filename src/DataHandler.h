#pragma once
#include "BDIObject.h"

namespace BDI
{
	using namespace DKUtil::Alias;

	class DataHandler : public dku::model::Singleton<DataHandler>
	{
		friend dku::model::Singleton<DataHandler>;

		struct BDIObjArray
		{
			enum Type
			{
				kVariable = 0,
				kEvent,
				kTotal
			};

			void InjectVariables(RE::hkbBehaviorGraph* a_graph) const;
			void InjectEvents(RE::hkbBehaviorGraph* a_graph) const;

			friend class DataHandler;

		private:
			void AddObject(BDIObject a_obj);

			std::vector<BDIObject> Objects[Type::kTotal];
		};

		DataHandler();

	public:
		[[nodiscard]] static bool IsSubpathMatch(std::string_view a_fullPath, std::string_view a_prefix) noexcept
		{
			while (!a_fullPath.empty() && (a_fullPath.front() == '\\' || a_fullPath.front() == '/')) {
				a_fullPath.remove_prefix(1);
			}
			while (!a_prefix.empty() && (a_prefix.front() == '\\' || a_prefix.front() == '/')) {
				a_prefix.remove_prefix(1);
			}
			while (!a_fullPath.empty() && (a_fullPath.back() == '\\' || a_fullPath.back() == '/')) {
				a_fullPath.remove_suffix(1);
			}
			while (!a_prefix.empty() && (a_prefix.back() == '\\' || a_prefix.back() == '/')) {
				a_prefix.remove_suffix(1);
			}

			if (a_prefix.empty()) {
				return false;
			}

			if (a_fullPath.length() < a_prefix.length()) {
				return false;
			}

			for (std::size_t i = 0; i < a_prefix.length(); ++i) {
				char c1 = (a_fullPath[i] == '/') ? '\\' : static_cast<char>(std::toupper(static_cast<unsigned char>(a_fullPath[i])));
				char c2 = (a_prefix[i] == '/') ? '\\' : static_cast<char>(std::toupper(static_cast<unsigned char>(a_prefix[i])));
				if (c1 != c2) {
					return false;
				}
			}

			if (a_fullPath.length() == a_prefix.length()) {
				return true;
			}

			const char nextChar = a_fullPath[a_prefix.length()];
			return nextChar == '\\' || nextChar == '/';
		}

		[[nodiscard]] std::vector<const BDIObjArray*> GetMatchingArrays(std::string_view a_fullPath) const noexcept
		{
			std::vector<const BDIObjArray*> result;
			result.reserve(objMap.size() > 8 ? 8 : objMap.size());
			for (const auto& [prefix, objArray] : objMap) {
				if (IsSubpathMatch(a_fullPath, prefix)) {
					result.push_back(&objArray);
				}
			}
			return result;
		}

		// clang-format off
		[[nodiscard]] auto get_subpaths(std::string_view a_fullPath) const noexcept
		{
			return objMap
				| std::views::filter([=](const auto& pair) {
					return IsSubpathMatch(a_fullPath, pair.first);
				})
				| std::views::values;
		}
		// clang-format on

		std::unordered_map<std::string, BDIObjArray> objMap;

		Boolean enableDebugLog{ "EnableDebugLog", "Debug" };
	};
}
