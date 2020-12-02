#pragma once

#include "ShapeConcept.h"

namespace glm
{
	template <typename T>
	struct tellipse2
	{
		glm::tvec2<T> center = { 0,0 };
		glm::tvec2<T> radii = { 0,0 };
	};
}