#pragma once

#include "ShapeConcept.h"

namespace gamelib
{

	struct intersection_data
	{
	};

	template <typename T>
	intersection_data calculate_intersection(trec2<T> const& rect1, trec2<T> const& rect2)
	{
		return {};
	}

}