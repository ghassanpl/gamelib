#pragma once

#include "ShapeConcept.h"
#include <vector>

namespace glm
{
	template <typename T>
	struct tpolygon2
	{
		std::vector<glm::tvec2<T>> vertices;
	};
}