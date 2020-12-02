#pragma once

#include "ShapeConcept.h"

namespace glm
{
	template <typename T>
	struct tseg2
	{
		glm::tvec2<T> p1 = { 0,0 };
		glm::tvec2<T> p2 = { 0,0 };
	};
}