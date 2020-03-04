#pragma once

#include "../Common.h"
#include "../Includes/GLM.h"
#include "../Includes/EnumFlags.h"

namespace gamelib::squares
{

	inline bool IsSurrounding(ivec2 const a, ivec2 const b) { return std::abs(a.x - b.x) < 2 && std::abs(a.y - b.y) < 2; }
	inline bool IsNeighbor(ivec2 const a, ivec2 const b) { return IsSurrounding(a, b) && std::abs(a.y - b.y) != std::abs(a.x - b.x); }
	inline bool IsDiagonalNeighbor(ivec2 const a, ivec2 const b) { return IsSurrounding(a, b) && std::abs(a.y - b.y) == std::abs(a.x - b.x); }

	enum class Direction
	{
		None = -1,

		Right = 0,
		RightDown,
		Down,
		LeftDown,
		Left,
		LeftUp,
		Up,
		RightUp,

		/// Aliases
		East = 0,
		SouthEast,
		South,
		SouthWest,
		West,
		NorthWest,
		North,
		NorthEast
	};

	using DirectionBitmap = ghassanpl::enum_flags<Direction, uint8_t>;

	inline constexpr Direction operator+(Direction dir, int d) { return (Direction)((int(dir) + d) % 8); }
	inline constexpr Direction operator-(Direction dir, int d) { return (Direction)((int(dir) + (8 + (d % 8))) % 8); }

	inline constexpr Direction& operator++(Direction& dir) { return dir = dir + 1; }
	inline constexpr Direction& operator--(Direction& dir) { return dir = dir + 7; }

	inline constexpr Direction operator++(Direction& dir, int) { auto res = dir; ++dir; return res; }
	inline constexpr Direction operator--(Direction& dir, int) { auto res = dir; --dir; return res; }

	inline constexpr Direction Opposite(Direction dir) { return dir + 4; }
	inline constexpr Direction NextCardinal(Direction dir) { return dir + 2; }

	namespace {
		static constexpr const int DirectionToOffset[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
		static constexpr const int OffsetToDirection[] = { 5, 6, 7, 4, -1, 0, 3, 2, 1 };
	}

	inline constexpr int HorizontalOffset(Direction dir) { return DirectionToOffset[(int)dir]; }
	inline constexpr int VerticalOffset(Direction dir) { return DirectionToOffset[int(dir + 6)]; }

	inline constexpr bool IsCardinal(Direction dir) { return (int(dir) & 1) == 0; }
	inline constexpr bool IsDiagonal(Direction dir) { return (int(dir) & 1) != 0; }

	inline constexpr DirectionBitmap AllCardinalDirections = { Direction::Left, Direction::Right, Direction::Up, Direction::Down };
	inline constexpr DirectionBitmap AllDiagonalDirections = { Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown };
	inline constexpr DirectionBitmap AllDirections = { Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown, Direction::Left, Direction::Right, Direction::Up, Direction::Down };

	inline Direction ToDirection(radians_t angle)
	{
		return Direction(int(glm::mod(glm::degrees(angle.Value - glm::radians(45.0 / 2.0)), 360.0) / 45.0) % 8);
	}

	inline constexpr Direction ToDirection(ivec2 vec)
	{
		return (Direction)OffsetToDirection[vec.x + vec.y * 3 + 4];
	}

	inline constexpr radians_t ToRadians(Direction dir)
	{
		return radians_t{ (float)glm::radians(int(dir) * 45.0) };
	}

	inline constexpr degrees_t ToDegrees(Direction dir)
	{
		return degrees_t{ float(int(dir) * 45.0) };
	}

	inline constexpr ivec2 ToVector(Direction dir)
	{
		return ivec2{ HorizontalOffset(dir), VerticalOffset(dir) };
	}

}