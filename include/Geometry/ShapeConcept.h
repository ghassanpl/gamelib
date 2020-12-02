#pragma once

#include "../Includes/GLM.h"
#include <type_traits>
#include <concepts>
#include <iosfwd>

namespace gamelib
{
	enum class ShapeRelationship
	{
		Separate,       /// no intersection
		Inside,         /// one object completely inside the other
		Touches,        /// only one common point or line
		Intersects,
		Overlaps,
		Parallel,       /// for rays/segments
		Identical,
	};

	template <typename T, typename VALUE_TYPE>
	struct tshape_helper
	{
		using value_type = VALUE_TYPE;
		using vec2 = glm::tvec2<value_type>;

		vec2 edge_point(value_type edge_position) const
		{
			return static_cast<T const*>(this)->edge_point_alpha(edge_position / static_cast<T const*>(this)->edge_length());
		}

		bool contains(vec2 point, value_type edge_epsilon = value_type{ 0 }) const
		{
			if constexpr (std::is_floating_point_v<value_type>)
				return static_cast<T const*>(this)->classify(point, edge_epsilon) != point_relationship::outside;
			else
				return static_cast<T const*>(this)->classify(point) != point_relationship::outside;
		}

		value_type x() const noexcept { const vec2 pos = static_cast<T const*>(this)->position(); return pos.x; }
		value_type y() const noexcept { const vec2 pos = static_cast<T const*>(this)->position(); return pos.y; }

		T& set_position(value_type x, value_type y) { static_cast<T*>(this)->set_position(vec2{ x,y }); return *static_cast<T*>(this); }

		T at_position(vec2 pos) const 
		{
			auto copy = static_cast<T const*>(this);
			copy.set_position(pos);
			return copy;
		}

		T at_position(value_type x, value_type y) const
		{
			return this->at_position({ x, y });
		}

		T valid() const
		{
			auto copy = static_cast<T const*>(this);
			copy.make_valid();
			return copy;
		}
	};

	template <typename T>
	concept shape = requires (T const& shape, T& shape_mutable, glm::vec2 pt, double a) {
		{ shape.classify(pt) } -> std::same_as<point_relationship>;
		{ shape.contains(pt) } -> std::convertible_to<bool>;
		{ shape.calculate_area() } -> std::floating_point;
		{ shape.edge_length() } -> std::floating_point;
		{ shape.edge_point_alpha(a) } -> std::convertible_to<glm::vec2>;
		{ shape.bounding_box() } noexcept -> std::convertible_to<rec2>;
		{ shape.projected(pt) } -> std::convertible_to<glm::vec2>;
		{ shape.is_valid() } noexcept -> std::convertible_to<bool>;
		{ shape.position() } noexcept -> std::convertible_to<glm::vec2>;
		{ shape_mutable.set_position(pt) } -> std::same_as<T&>;
		{ shape_mutable.make_valid() } -> std::same_as<T&>;
	};


}