#pragma once

#include "ShapeConcept.h"
#include "../Random.h"

namespace gamelib
{
	template <typename T>
	struct tcircle2 : tshape_helper<tcircle2<T>, T>
	{
		using value_type = T;
		using vec2 = glm::tvec2<value_type>;

		vec2 center = { 0,0 };
		T radius = {};

		tcircle2() noexcept = default;
		tcircle2(vec2 c, T r) noexcept : center(c), radius(r) {}
		tcircle2(T r, vec2 c) noexcept : center(c), radius(r) {}

		vec2 position() const noexcept { return center; }
		tcircle2& set_position(vec2 pos) noexcept { center = pos; return *this; }

		T radius_squared() const noexcept { return this->radius * this->radius; }

		bool is_valid() const noexcept
		{ 
			using std::isnan;
			return this->radius >= T{} && !isnan(this->radius) && glm::isnan(this->center) == glm::bvec2{false, false};
		}

		tcircle2& make_valid()
		{
			using std::abs;
			if (this->radius < 0)
				this->radius = abs(this->radius);
			return *this;
		}

		point_relationship classify(vec2 point, T edge_epsilon = T{ 0 }) const
		{
			const auto d = this->center - point;
			const auto a = glm::dot(d, d);
			const auto r2 = this->radius_squared();
			if (epsilonEqual(a, r2, edge_epsilon))
				return point_relationship::on_edge;
			else if (a < r2)
				return point_relationship::inside;
			else
				return point_relationship::outside;
		}

		T calculate_area() const noexcept { return glm::pi<T>() * this->radius_squared(); }

		T edge_length() const noexcept { return glm::two_pi<T>() * this->radius; }

		vec2 edge_point_alpha(progress_t edge_progress) const
		{
			using std::sin;
			using std::cos;
			const auto a = edge_progress * glm::two_pi<T>();
			return this->center + vec2(cos(a), sin(a)) * this->radius;
		}

		rec2 bounding_box() const noexcept
		{
			return rec2::from_size(center.x - radius, center.y - radius, radius * 2, radius * 2);
		}

		vec2 projected(vec2 pt) const { return this->center + glm::normalize(pt - this->center) * this->radius; }
	};

	static_assert(shape<tcircle2<float>>);
}