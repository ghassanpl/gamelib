#pragma once

#include "Circle.h"
#include "../Random.h"

namespace gamelib
{
	template <typename T, typename RNG>
	glm::tvec2<T> random_point(glm::tvec2<T> pt, RNG&& rng)
	{
		return { random::RealRange(rng, T{}, pt.x), random::RealRange(rng, T{}, pt.y) };
	}

	template <typename T, typename RNG>
	auto random_point(tcircle2<T> const& s, RNG&& rng)
	{
		using std::sin; using std::cos;
		const auto t = random::Radians<T>(rng);
		const auto u = random::Percentage<T>(rng) + random::Percentage<T>(rng);
		const auto r = (u > 1.0) ? 2.0 - u : u;
		return s.center + glm::tvec2<T>(r * cos(t), r * sin(t));
	}

	template <typename T, typename RNG>
	auto random_point(trec2<T> const& s, RNG&& rng)
	{
		return s.position() + random_point(s.size(), rng);
	}
}