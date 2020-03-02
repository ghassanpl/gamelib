#pragma once

namespace gamelib
{

	///************************************************************************/
	/// Align
	///************************************************************************/

	enum class HorizontalAlign
	{
		Left = 0,
		Center = 1,
		Right = 2,
		Justify = 3,

		Mask = Left | Center | Right
	};

	enum class VerticalAlign
	{
		Top = 0,
		Middle = 4,
		Bottom = 8,
		Justify = 12,

		Mask = Top | Middle | Bottom
	};

	enum class Align;

	constexpr inline Align operator|(HorizontalAlign first, VerticalAlign second) { return Align(int(first) | int(second)); }
	constexpr inline Align operator|(VerticalAlign first, HorizontalAlign second) { return Align(int(first) | int(second)); }

	enum class Align
	{
		LeftTop = HorizontalAlign::Left | VerticalAlign::Top,
		CenterTop = HorizontalAlign::Center | VerticalAlign::Top,
		RightTop = HorizontalAlign::Right | VerticalAlign::Top,

		LeftMiddle = HorizontalAlign::Left | VerticalAlign::Middle,
		CenterMiddle = HorizontalAlign::Center | VerticalAlign::Middle,
		RightMiddle = HorizontalAlign::Right | VerticalAlign::Middle,

		LeftBottom = HorizontalAlign::Left | VerticalAlign::Bottom,
		CenterBottom = HorizontalAlign::Center | VerticalAlign::Bottom,
		RightBottom = HorizontalAlign::Right | VerticalAlign::Bottom,
	};

	constexpr inline Align& operator|=(Align& first, HorizontalAlign second) { return first = Align(int(first) | int(second)); }
	constexpr inline Align& operator|=(Align& first, VerticalAlign second) { return first = Align(int(first) | int(second)); }

	constexpr inline const char* HorizontalNames[] = { "Left", "Center", "Right", "JustifyHorizontal" };
	constexpr inline const char* VerticalNames[] = { "Top", "Middle", "Bottom", "JustifyVertical", "Middle", "", "", "", "Bottom", "", "", "", "JustifyVertical" };
		
	constexpr inline VerticalAlign ToVertical(HorizontalAlign align) { return VerticalAlign{ (int(align) & int(HorizontalAlign::Mask)) >> 2 }; }
	constexpr inline VerticalAlign ToVertical(VerticalAlign align) { return align; }
	constexpr inline HorizontalAlign ToHorizontal(VerticalAlign align) { return HorizontalAlign{ (int(align) & int(VerticalAlign::Mask)) << 2 }; }
	constexpr inline HorizontalAlign ToHorizontal(HorizontalAlign align) { return align; }
	constexpr inline VerticalAlign Vertical(Align align) { return VerticalAlign{ (int(align) & int(VerticalAlign::Mask)) }; }
	constexpr inline HorizontalAlign Horizontal(Align align) { return HorizontalAlign{ (int(align) & int(HorizontalAlign::Mask)) }; }

	template <typename T>
	inline constexpr T AlignAxis(const T& width, const T& max_width, HorizontalAlign align) {
		switch (align)
		{
		case HorizontalAlign::Center: return (max_width / 2 - width / 2);
		case HorizontalAlign::Right: return (max_width - width);
		default:
		case HorizontalAlign::Left: return 0;
		}
	}

	template <typename T>
	inline constexpr T AlignAxis(const T& width, const T& max_width, VerticalAlign align) {
		switch (align)
		{
		case VerticalAlign::Middle: return (max_width / 2 - width / 2);
		case VerticalAlign::Bottom: return (max_width - width);
		default:
		case VerticalAlign::Top: return 0;
		}
	}

	/// Margins

	/*
	template <typename T>
	struct HorizontalMargins
	{
		T Left = {};
		T Right = {};

		constexpr HorizontalMargins(T left, T right) : Left(left), Right(right) {}

		constexpr HorizontalMargins(T container_width, T width, Alignment align)
		{
			switch (GetHorizontal(align))
			{
			case Alignment::Right:
				Left = container_width - width;
				break;
			case Alignment::Center:
				Left = container_width / 2 - detail::DivideRoundDown(width, 2);
				Right = container_width / 2 + detail::DivideRoundUp(width, 2);
				break;
			case Alignment::Left:
			default:
				Left = container_width - width;
				break;
			}
		}

		constexpr HorizontalMargins() = default;
		constexpr HorizontalMargins(const HorizontalMargins&) = default;
		constexpr HorizontalMargins(HorizontalMargins&&) noexcept = default;
		constexpr HorizontalMargins& operator=(const HorizontalMargins&) = default;
		constexpr HorizontalMargins& operator=(HorizontalMargins&&) noexcept = default;
	};
	*/
}

#define GAMELIB_ALIGN
#ifdef GAMELIB_GLM
#include "Combos/Align+GLM.h"
#endif