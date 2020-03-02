#pragma once

namespace gamelib
{
	template <typename T>
	inline constexpr glm::tvec2<T> AlignAxes(const glm::tvec2<T>& size, const glm::tvec2<T>& max_size, Align align) {
		return { AlignAxis(size.x, max_size.x, Horizontal(align)), AlignAxis(size.y, max_size.y, Vertical(align)) };
	}

}