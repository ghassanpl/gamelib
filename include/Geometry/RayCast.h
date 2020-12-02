#pragma once

#include "Ray.h"
#include <optional>

namespace gamelib
{
	template <typename T>
	struct ray_cast_result
	{
		glm::tvec2<T> contact_point{ 0, 0 };
		glm::tvec2<T> contact_normal{ 0, 0 };
		T time_hit_near{};

		bool contact_in_direction() const noexcept
		{
			return time_hit_near >= 0.0f;
		}
		bool contact_in_distance(T max_distance) const noexcept
		{
			return time_hit_near >= 0.0f && time_hit_near < max_distance;
		}
	};

	/// Shamelessly stolen from: https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_Rectangles.cpp
	template <typename T>
	auto ray_cast(tray2<T> const& ray, trec2<T> const& rect) -> std::optional<ray_cast_result<T>>
	{
		ray_cast_result<T> result;

		/// Cache division
		//const auto invdir = T{ 1 } ;

		/// Calculate intersections with rectangle bounding axes
		auto t_near = (rect.p1 - ray.origin) / ray.direction;
		auto t_far = (rect.p2 - ray.origin) / ray.direction;

		if (std::isnan(t_far.y) || std::isnan(t_far.x)) return std::nullopt;
		if (std::isnan(t_near.y) || std::isnan(t_near.x)) return std::nullopt;

		/// Sort distances
		if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
		if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

		/// Early rejection		
		if (t_near.x > t_far.y || t_near.y > t_far.x) return std::nullopt;

		/// Closest 'time' will be the first contact
		result.time_hit_near = std::max(t_near.x, t_near.y);

		/// Furthest 'time' is contact on opposite side of target
		const auto t_hit_far = std::min(t_far.x, t_far.y);

		/// Reject if ray direction is pointing away from object
		if (t_hit_far < T{})
			return std::nullopt;

		/// Contact point of collision from parametric line equation
		result.contact_point = ray.at(result.time_hit_near);

		if (t_near.x > t_near.y)
		{
			if (ray.direction.x < 0)
				result.contact_normal = { T{1}, T{} };
			else
				result.contact_normal = { T{-1}, T{} };
		}
		else if (t_near.x < t_near.y)
		{
			if (ray.direction.y < 0)
				result.contact_normal = { T{}, T{1} };
			else
				result.contact_normal = { T{}, T{-1} };
		}

		/// Note if t_near == t_far, collision is principly in a diagonal
		/// so pointless to resolve. By returning a CN={0,0} even though its
		/// considered a hit, the resolver wont change anything.
		return result;
	}

}