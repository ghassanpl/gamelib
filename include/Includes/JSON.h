#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>

using nlohmann::json;

namespace nlohmann {
	template <>
	struct adl_serializer<std::filesystem::path> {
		static void to_json(json& j, const std::filesystem::path& p) { j = p.string(); }
		static void from_json(const json& j, std::filesystem::path& p) { p = j.get_ref<std::string const&>(); }
	};
}

#define GAMELIB_JSON
#include "../Combos/GLM+JSON.h"
#include "../Combos/EnumFlags+JSON.h"