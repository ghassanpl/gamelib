#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>

using nlohmann::json;

namespace nlohmann {
	template <>
	struct adl_serializer<path> {
		static void to_json(json& j, const path& p) { j = p.string(); }
		static void from_json(const json& j, path& p) { p = j.get_ref<std::string const&>(); }
	};
}

#define GAMELIB_JSON
#ifdef GAMELIB_GLM
#include "../Combos/GLM+JSON.h"
#endif
#ifdef GAMELIB_ENUM_FLAGS
#include "../Combos/EnumFlags+JSON.h"
#endif