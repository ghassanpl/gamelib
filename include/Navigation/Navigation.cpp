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
		return At(from)->Blocks[(int)what].is_set(dir);
	}

	bool WallNavigationGrid::Blocks(ivec2 from, ivec2 to, WallBlocks what) const
	{
		Assuming(IsSurrounding(from, to));
		if (from == to) return false;
		return At(from)->Blocks[(int)what].is_set(ToDirection(to - from));
	}

	void WallNavigationGrid::SetBlocking(ivec2 from, Direction dir, enum_flags<WallBlocks> what)
	{
		if (auto from_tile = At(from))
		{
			auto& adj = from_tile->Blocks;
			what.for_each([&adj, dir](WallBlocks blocks) {
				adj[(int)blocks].set(dir);
			});
		}
	}

	void WallNavigationGrid::SetBlocking(ivec2 from, ivec2 to, enum_flags<WallBlocks> what, bool two_ways)
	{
		Assuming(IsSurrounding(from, to));
		if (from == to) return;
		const auto dir = ToDirection(to - from);
		SetBlocking(from, dir, what);
		if (two_ways)
			SetBlocking(to, Opposite(dir), what);
	}

	void WallNavigationGrid::SetBlocking(irec2 const& room, enum_flags<WallBlocks> what, bool two_ways)
	{
		for (auto x = room.p1.x; x < room.p2.x; x++)
		{
			SetBlocking(ivec2{ x, room.p1.y }, Direction::Up, what);
			SetBlocking(ivec2{ x, room.p1.y - 1 }, Direction::Down, what);

			SetBlocking(ivec2{ x, room.p2.y - 1 }, Direction::Down, what);
			SetBlocking(ivec2{ x, room.p2.y }, Direction::Up, what);
		}
		for (auto y = room.p1.y; y < room.p2.y; y++)
		{
			SetBlocking(ivec2{ room.p1.x, y }, Direction::Left, what);
			SetBlocking(ivec2{ room.p1.x - 1, y }, Direction::Right, what);

			SetBlocking(ivec2{ room.p2.x - 1, y }, Direction::Right, what);
			SetBlocking(ivec2{ room.p2.x, y }, Direction::Left, what);
		}
	}

	void WallNavigationGrid::SetNonBlocking(ivec2 from, Direction dir, enum_flags<WallBlocks> what)
	{
		if (auto from_tile = At(from))
		{
			auto& adj = from_tile->Blocks;
			what.for_each([&adj, dir](WallBlocks blocks) {
				adj[(int)blocks].unset(dir);
			});
		}
	}

	void WallNavigationGrid::SetNonBlocking(ivec2 from, ivec2 to, enum_flags<WallBlocks> what, bool two_ways)
	{
		Assuming(IsSurrounding(from, to));
		if (from == to) return;
		const auto dir = ToDirection(to - from);
		SetNonBlocking(from, dir, what);
		if (two_ways)
			SetNonBlocking(to, Opposite(dir), what);
	}

	void WallNavigationGrid::SetNonBlocking(irec2 const& room, enum_flags<WallBlocks> what, bool two_ways)
	{
		for (auto x = room.p1.x; x < room.p2.x; x++)
		{
			SetNonBlocking(ivec2{ x, room.p1.y }, Direction::Up, what);
			SetNonBlocking(ivec2{ x, room.p1.y - 1 }, Direction::Down, what);

			SetNonBlocking(ivec2{ x, room.p2.y - 1 }, Direction::Down, what);
			SetNonBlocking(ivec2{ x, room.p2.y }, Direction::Up, what);
		}
		for (auto y = room.p1.y; y < room.p2.y; y++)
		{
			SetNonBlocking(ivec2{ room.p1.x, y }, Direction::Left, what);
			SetNonBlocking(ivec2{ room.p1.x - 1, y }, Direction::Right, what);

			SetNonBlocking(ivec2{ room.p2.x - 1, y }, Direction::Right, what);
			SetNonBlocking(ivec2{ room.p2.x, y }, Direction::Left, what);
		}
	}

}