#pragma once

#include "RayCast.h"
#include <vector>

namespace gamelib
{
	template <typename T>
	std::optional<ray_cast_result<T>> sweep_rect_against_rect(trec2<T> const& r_dynamic, glm::tvec2<T> velocity, trec2<T> const& r_static)
	{
		/// Check if dynamic rectangle is actually moving - we assume rectangles are NOT in collision to start
		if (velocity.x == T{ 0 } && velocity.y == T{ 0 })
			return std::nullopt;

		/// Expand target rectangle by source dimensions
		const auto expanded_target = r_static.grown(r_dynamic.half_size());

		if (auto result = ray_cast(tray2<T>{r_dynamic.center(), velocity}, expanded_target); result && result->contact_in_distance(1.0f))
			return result;
		return {};
	}

	template <typename T>
	struct rect_collision_resolution
	{
		glm::tvec2<T> new_velocity{};
		std::array<trec2<T> const*, 4> contacts{};
	};

	template <typename T>
	bool resolve_rect_against_rect_sweep(trec2<T> const& r_dynamic, trec2<T> const& r_static, rect_collision_resolution<T>& resolution)
	{
		if (auto raycast_result = sweep_rect_against_rect(r_dynamic, resolution.new_velocity, r_static))
		{
			if (raycast_result->contact_normal.y > 0) resolution.contacts[0] = &r_static;
			if (raycast_result->contact_normal.x < 0) resolution.contacts[1] = &r_static;
			if (raycast_result->contact_normal.y < 0) resolution.contacts[2] = &r_static;
			if (raycast_result->contact_normal.x > 0) resolution.contacts[3] = &r_static;

			resolution.new_velocity += raycast_result->contact_normal * glm::abs(resolution.new_velocity) * (T{ 1 } - raycast_result->time_hit_near);
			return true;
		}

		return false;
	}

	template <typename T>
	rect_collision_resolution<T> resolve_rect_sweep_against_multiple_rects(trec2<T> const& r_dynamic, glm::tvec2<T> const& velocity, std::span<trec2<T>> rects)
	{
		std::vector<std::pair<size_t, float>> collisions;

		/// Work out collision point, add it to vector along with rect ID
		for (size_t i = 0; i < rects.size(); i++)
		{
			if (auto result = sweep_rect_against_rect(r_dynamic, velocity, rects[i]))
				collisions.push_back({ i, result->contact_time });
		}

		/// Do the sort
		std::sort(collisions.begin(), collisions.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) { return a.second < b.second; });

		/// Now resolve the collision in correct order 
		rect_collision_resolution<T> resolution{ velocity };
		for (auto collision : collisions)
			resolve_rect_against_rect_sweep(r_dynamic, velocity, rects[collision.first], resolution);
	}
}