#pragma once
#include "DKUtil/Config.hpp"
#include "nlohmann/json.hpp"

namespace BDI
{
	using json = nlohmann::json;

	struct BDIObject
	{
		friend void from_json(const json& j, BDIObject& a_data);
		friend class DataHandler;

		enum class DataTypes : std::uint32_t
		{
			kNone = 0,
			kBool = 1,
			kInt = 2,
			kFloat = 3,
			kEvent = 4
		};

		DataTypes GetType() const noexcept { return type; }
		const std::string& GetProjectPath() const noexcept { return projectPath; }
		const std::string& GetName() const noexcept { return name; }
		const RE::hkbVariableValue& GetValue() const noexcept { return value; }

	private:
		std::string projectPath;

		DataTypes type = DataTypes::kNone;

		std::string name;

		RE::hkbVariableValue value{};
	};
	using BDIDataTypes = BDIObject::DataTypes;

	inline DKUtil::model::enumeration<BDIDataTypes, std::uint32_t> BDITypeEnumTbl;

	void from_json(const json& j, BDIObject& a_data);
}
