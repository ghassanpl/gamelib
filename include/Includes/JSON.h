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
#ifdef GAMELIB_GLM
#include "../Combos/GLM+JSON.h"
#endif
#ifdef GAMELIB_ENUM_FLAGS
#include "../Combos/EnumFlags+JSON.h"
#endif