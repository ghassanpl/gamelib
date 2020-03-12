#include <iostream>
#include "Navigation.h"
#include "../Includes/Assuming.h"

namespace gamelib::squares
{

	void BlockNavigationGrid::ClearData(enum_flags<BlockNavigationTile::TileFlags> unset_flags)
	{
		for (auto& tile : mTiles)
		{
			tile.Cost = std::numeric_limits<double>::quiet_NaN();
			tile.Flags.bits = tile.Flags.bits & ~unset_flags.bits;
			tile.Predecessor = { -1, -1 };
		}
	}

	/*
	/// TODO: Should we move this to Combos?
	void BlockNavigationGrid::InitFrom(TileLayer const* layer)
	{
		AssumingNotNull(layer);
		Reset(layer->Size());
		ForEach([this, layer](ivec2 pos) {
			At(pos)->Flags.set_to(layer->At(pos)->GetProperty<bool>("BlocksPassage"), NavigationTileFlags::BlocksPassage);
			At(pos)->Flags.set_to(layer->At(pos)->GetProperty<bool>("BlocksSight"), NavigationTileFlags::BlocksSight);
			return false;
		});
	}
	*/

	std::vector<ivec2> BlockNavigationGrid::BreadthFirstSearch(ivec2 start, ivec2 goal, bool diagonals)
	{
		if (diagonals)
		{
			return BaseNavigationGrid<BlockNavigationTile>::BreadthFirstSearch<true>(start, goal, [&, goal](ivec2 from, ivec2 to) {
				return (to == goal || !BlocksPassage(to)) && (!IsDiagonalNeighbor(from, to) || (!BlocksPassage({ from.x, to.y }) && BlocksPassage({ to.x, from.y })));
			});
		}
		else
		{
			return BaseNavigationGrid<BlockNavigationTile>::BreadthFirstSearch<false>(start, goal, [&, goal](ivec2 from, ivec2 to) { return (to == goal || !BlocksPassage(to)); });
		}
	}

	inline double len(ivec2 a, ivec2 b) noexcept { return (double)glm::length(vec2(a - b)); }

	std::vector<ivec2> BlockNavigationGrid::DijkstraSearch(ivec2 start, ivec2 goal, double max_cost, bool diagonals)
	{
		if (diagonals)
		{
			return BaseNavigationGrid<BlockNavigationTile>::DijkstraSearch<true>(start, goal, [&, goal](ivec2 from, ivec2 to) {
				return (to == goal || !BlocksPassage(to)) && (!IsDiagonalNeighbor(from, to) || (!BlocksPassage({ from.x, to.y }) && BlocksPassage({ to.x, from.y })));
			}, max_cost, len);
		}
		else
		{
			return BaseNavigationGrid<BlockNavigationTile>::DijkstraSearch<false>(start, goal, [&, goal](ivec2 from, ivec2 to) {
				return (to == goal || !BlocksPassage(to));
			}, max_cost, len);
		}
	}

	std::vector<ivec2> BlockNavigationGrid::AStarSearch(ivec2 start, ivec2 goal, bool diagonals)
	{
		if (diagonals)
		{
			return BaseNavigationGrid<BlockNavigationTile>::AStarSearch<true>(start, goal, [&, goal](ivec2 from, ivec2 to) {
				return (to == goal || !BlocksPassage(to)) && (!IsDiagonalNeighbor(from, to) || (!BlocksPassage({ from.x, to.y }) && BlocksPassage({ to.x, from.y })));
			}, len, len);
		}
		else
		{
			return BaseNavigationGrid<BlockNavigationTile>::AStarSearch<false>(start, goal, [&, goal](ivec2 from, ivec2 to) {
				return (to == goal || !BlocksPassage(to));
			}, len, len);
		}
	}

	void BlockNavigationGrid::CalculateFOV(ivec2 source, int max_radius, bool include_walls)
	{
		ClearData(BlockNavigationTile::TileFlags::Visible);

		this->CalculateFOV(source, max_radius, include_walls,
			[this](ivec2 pos) {
				return !BlocksSight(pos);
			},
			[this](ivec2 pos) {
				At(pos)->Flags.set(BlockNavigationTile::TileFlags::Visible, BlockNavigationTile::TileFlags::WasSeen);
			}
		);
	}

	bool BlockNavigationGrid::CanSee(ivec2 start, ivec2 end, bool ignore_start) const
	{
		return LineCast(start, end, [this](ivec2 pos) { return BlocksSight(pos); }, ignore_start);
	}

	void WallNavigationGrid::ClearData(enum_flags<WallNavigationTile::TileFlags> unset_flags)
	{
		for (auto& tile : mTiles)
		{
			tile.Cost = std::numeric_limits<double>::quiet_NaN();
			tile.Flags.bits = tile.Flags.bits & ~unset_flags.bits;
			tile.Predecessor = { -1, -1 };
			tile.Blocks = {};
		}
	}

	bool WallNavigationGrid::Blocks(ivec2 from, Direction dir, WallBlocks what) const
	{
		return At(from)->Blocks[(int)dir].is_set(what);
	}

	bool WallNavigationGrid::Blocks(ivec2 from, ivec2 to, WallBlocks what) const
	{
		Assuming(IsSurrounding(from, to));
		if (from == to) return false;
		return At(from)->Blocks[(int)ToDirection(to - from)].is_set(what);
	}

	enum_flags<WallBlocks>& WallNavigationGrid::BlocksIn(ivec2 from, ivec2 to)
	{
		Assuming(IsSurrounding(from, to));
		AssumingNotEqual(from, to);
		return At(from)->Blocks[(int)ToDirection(to - from)];
	}

	enum_flags<WallBlocks> const& WallNavigationGrid::BlocksIn(ivec2 from, ivec2 to) const
	{
		Assuming(IsSurrounding(from, to));
		AssumingNotEqual(from, to);
		return At(from)->Blocks[(int)ToDirection(to - from)];
	}

	void WallNavigationGrid::SetBlocking(ivec2 from, Direction dir, enum_flags<WallBlocks> what, bool blocking)
	{
		if (auto from_tile = At(from))
		{
			from_tile->Blocks[(int)dir].set_to(blocking, what);
		}
	}

	void WallNavigationGrid::SetBlocking(ivec2 from, ivec2 to, enum_flags<WallBlocks> what, bool blocking, bool two_ways)
	{
		Assuming(IsSurrounding(from, to));
		if (from == to) return;
		const auto dir = ToDirection(to - from);
		SetBlocking(from, dir, what, blocking);
		if (two_ways)
			SetBlocking(to, Opposite(dir), what, blocking);
	}

	void WallNavigationGrid::SetBlocking(irec2 const& room, enum_flags<WallBlocks> what, bool blocking, bool two_ways)
	{
		for (auto x = room.p1.x; x < room.p2.x; x++)
		{
			SetBlocking(ivec2{ x, room.p1.y }, Direction::Up, what, blocking);
			if (two_ways) SetBlocking(ivec2{ x, room.p1.y - 1 }, Direction::Down, what, blocking);

			SetBlocking(ivec2{ x, room.p2.y - 1 }, Direction::Down, what, blocking);
			if (two_ways) SetBlocking(ivec2{ x, room.p2.y }, Direction::Up, what, blocking);
		}
		for (auto y = room.p1.y; y < room.p2.y; y++)
		{
			SetBlocking(ivec2{ room.p1.x, y }, Direction::Left, what, blocking);
			if (two_ways) SetBlocking(ivec2{ room.p1.x - 1, y }, Direction::Right, what, blocking);

			SetBlocking(ivec2{ room.p2.x - 1, y }, Direction::Right, what, blocking);
			if (two_ways) SetBlocking(ivec2{ room.p2.x, y }, Direction::Left, what, blocking);
		}
	}

	RaycastResult WallNavigationGrid::RayCast(vec2 tile_size, vec2 start, vec2 direction, WallBlocks blocking, double max_distance)
	{
		auto tile = WorldPositionToTilePosition(start, tile_size);
		auto old_tile = tile;
		auto b = glm::lessThanEqual(direction, {});
		auto dTile = glm::mix(ivec2{ 1, 1 }, ivec2{ -1, -1 }, b);
		auto ddt = (vec2{ dTile } * tile_size) / direction;
		auto dt = (vec2(glm::mix(tile + ivec2{ 1, 1 }, tile, b)) * tile_size - start) / direction;
		double t = 0;
		if (glm::dot(direction, direction) > 0)
		{
			while (t < max_distance && IsValid(tile))
			{
				/// VISITED(tile)
				/// MARK(start + direction * t)

				old_tile = tile;

				if (dt.x < dt.y)
				{
					tile.x += dTile.x;
					auto d = dt.x;
					t += d;
					dt.x += ddt.x - d;
					dt.y -= d;
				}
				else
				{
					tile.y += dTile.y;
					auto d = dt.y;
					t += d;
					dt.x -= d;
					dt.y += ddt.y - d;
				}

				if (t <= max_distance && Blocks(old_tile, tile, blocking))
				{
					return RaycastResult{
						.Distance = t,
						.Tile = old_tile,
						.Neighbor = tile,
						.HitPosition = start + direction * (float)t,
						.Wall = ToDirection(tile - old_tile),
						.Hit = true,
					};
				}
			}

			return RaycastResult{
				.Distance = max_distance,
				.Tile = old_tile,
				.Neighbor = tile,
				.HitPosition = start + direction * (float)max_distance,
				.Wall = ToDirection(tile - old_tile),
				.Hit = false,
			};
		}

		return {};
	}
}