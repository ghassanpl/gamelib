#pragma once

#include "ShapeConcept.h"

namespace gamelib
{

	template <typename T>
	struct tray2
	{
		glm::tvec2<T> origin;
		glm::tvec2<T> direction;

		glm::tvec2<T> at(T alpha) const
		{
			return origin + direction * alpha;
		}
	};

}