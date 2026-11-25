#include "BDIObject.h"

namespace BDI
{
	void from_json(const json& j, BDIObject& a_data)
	{
		j.at("projectPath").get_to(a_data.projectPath);

		auto typeName = j.at("type").get<std::string>();
		auto enumType = BDITypeEnumTbl.from_string(typeName, false);
		if (enumType.has_value()) {
			a_data.type = enumType.value();
		} else {
			// Resilient fallback for all common variants (with/without 'k', case-insensitive)
			std::string lowerType = typeName;
			for (char& c : lowerType) {
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}
			if (lowerType.starts_with('k') && lowerType.length() > 1) {
				lowerType.erase(0, 1);
			}

			if (lowerType == "bool" || lowerType == "boolean") {
				a_data.type = BDIDataTypes::kBool;
			} else if (lowerType == "int" || lowerType == "int32" || lowerType == "integer") {
				a_data.type = BDIDataTypes::kInt;
			} else if (lowerType == "float" || lowerType == "real" || lowerType == "double") {
				a_data.type = BDIDataTypes::kFloat;
			} else if (lowerType == "event") {
				a_data.type = BDIDataTypes::kEvent;
			} else {
				throw std::invalid_argument("Invalid BDI data type: " + typeName);
			}
		}

		j.at("name").get_to(a_data.name);

		switch (a_data.type) {
		case BDIDataTypes::kBool:
			if (j.contains("value")) {
				const auto& val = j.at("value");
				if (val.is_boolean()) {
					a_data.value.b = val.get<bool>();
				} else if (val.is_number()) {
					a_data.value.b = (val.get<int>() != 0);
				} else if (val.is_string()) {
					std::string strVal = val.get<std::string>();
					a_data.value.b = (_stricmp(strVal.c_str(), "true") == 0 || strVal == "1");
				} else {
					val.get_to(a_data.value.b);
				}
			}
			break;

		case BDIDataTypes::kInt:
			if (j.contains("value")) {
				const auto& val = j.at("value");
				if (val.is_number()) {
					a_data.value.i = val.get<std::int32_t>();
				} else if (val.is_boolean()) {
					a_data.value.i = val.get<bool>() ? 1 : 0;
				} else if (val.is_string()) {
					try {
						a_data.value.i = std::stoi(val.get<std::string>());
					} catch (...) {
						a_data.value.i = 0;
					}
				} else {
					val.get_to(a_data.value.i);
				}
			}
			break;

		case BDIDataTypes::kFloat:
			if (j.contains("value")) {
				const auto& val = j.at("value");
				if (val.is_number()) {
					a_data.value.f = val.get<float>();
				} else if (val.is_string()) {
					try {
						a_data.value.f = std::stof(val.get<std::string>());
					} catch (...) {
						a_data.value.f = 0.0f;
					}
				} else {
					val.get_to(a_data.value.f);
				}
			}
			break;

		case BDIDataTypes::kEvent:
		default:
			break;
		}
	}
}
