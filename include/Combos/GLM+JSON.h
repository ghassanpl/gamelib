#pragma once

namespace gamelib
{
	template <typename T>
	inline void to_json(json& j, const trec2<T>& p) { j = json{ p.p1.x, p.p1.y, p.p2.x, p.p2.y }; }
	template <typename T>
	inline void from_json(const json& j, trec2<T>& p) { p = { j[0], j[1], j[2], j[3] }; }
}

namespace glm
{
	template <typename T>
	inline void to_json(json& j, const vec<2, T>& p) { j = json{ p.x, p.y }; }
	template <typename T>
	inline void from_json(const json& j, vec<2, T>& p) { p = { j[0], j[1] }; }

	inline void to_json(json& j, const vec4& p) { j = json{ p.x, p.y, p.z, p.w }; }
	inline void from_json(const json& j, vec4& p) { p = { j[0], j[1], j[2], j[3] }; }
}