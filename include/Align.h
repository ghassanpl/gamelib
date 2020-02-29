#pragma once

namespace gamelib
{

	///************************************************************************/
	/// Align
	///************************************************************************/

	enum class Align
	{
		Left = 0,
		Center = 1,
		Right = 2,

		HorizontalMask = Left | Center | Right,

		Top = 0,
		Middle = 4,
		Bottom = 8,

		VerticalMask = Top | Middle | Bottom,

		LeftTop = Left | Top,
		CenterTop = Center | Top,
		RightTop = Right | Top,

		LeftMiddle = Left | Middle,
		CenterMiddle = Center | Middle,
		RightMiddle = Right | Middle,

		LeftBottom = Left | Bottom,
		CenterBottom = Center | Bottom,
		RightBottom = Right | Bottom,
	};

	constexpr inline Align operator|(Align first, Align second) { return Align(int(first) | int(second)); }

	constexpr inline Align GetHorizontal(Align align) { return Align{ int(align) & int(Align::HorizontalMask) }; }
	constexpr inline Align GetVertical(Align align) { return Align{ int(align) & int(Align::VerticalMask) }; }
	constexpr inline Align HorizontalToVertical(Align align) { return Align{ (int(align) & int(Align::HorizontalMask)) >> 2 }; }
	constexpr inline Align VerticalToHorizontal(Align align) { return Align{ (int(align) & int(Align::VerticalMask)) << 2 }; }

	template <typename T>
	inline constexpr T AlignAxis(const T& width, const T& max_width, Align horizontal_align) {
		switch (GetHorizontal(horizontal_align))
		{
		case Align::Center: return (max_width / 2 - width / 2);
		case Align::Right: return (max_width - width);
		default:
		case Align::Left: return 0;
		}
	}

}