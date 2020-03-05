#pragma once

#include "../Includes/GLM.h"
#include "../Includes/EnumFlags.h"
#include "../Colors.h"
#include "Grid.h"

namespace gamelib::squares
{
	/// TODO: Maybe split this into two classes, one that uses per-tile Blocks, one that uses per-wall ones

	struct BaseNavigationTile
	{
		double Cost = std::numeric_limits<double>::quiet_NaN();
		ivec2 Predecessor{ -1, -1 };
	};

	template <typename TILE_DATA>
	struct BaseNavigationGrid : public Grid<TILE_DATA>
	{
		void ClearData();

		/// Returns the REVERSED path, for ease of popping
		template <bool DIAGONALS = true, typename PASSABLE_FUNCTION>
		std::vector<ivec2> BreadthFirstSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func);

		/// Returns the REVERSED path, for ease of popping
		template <bool DIAGONALS = true, typename PASSABLE_FUNCTION, typename COST_FUNCTION>
		std::vector<ivec2> DijkstraSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func, double max_cost, COST_FUNCTION&& cost_function);

		/// Returns the REVERSED path, for ease of popping
		template <bool DIAGONALS = true, typename PASSABLE_FUNCTION, typename HEURISTIC_FUNCTION, typename COST_FUNCTION>
		std::vector<ivec2> AStarSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func, HEURISTIC_FUNCTION&& heuristic, COST_FUNCTION&& cost_function);

		double& Cost(ivec2 pos) noexcept { return this->At(pos)->Cost; }
		double Cost(ivec2 pos) const noexcept { return this->At(pos)->Cost; }
		ivec2& Predecessor(ivec2 pos) noexcept { return this->At(pos)->Predecessor; }
		ivec2 Predecessor(ivec2 pos) const noexcept { return this->At(pos)->Predecessor; }

		bool HasCost(ivec2 pos) const noexcept { return !std::isnan(this->At(pos)->Cost); }

	protected:

		std::vector<ivec2> ReconstructPath(ivec2 start, ivec2 goal) const;

		std::vector<std::pair<double, ivec2>> mSearchFrontier;

		void PutSearchFrontierItem(ivec2 item, double priority);

		ivec2 GetSearchFrontierItem();
	};

	struct BlockNavigationTile : BaseNavigationTile
	{
		enum class TileFlags
		{
			BlocksPassage,
			BlocksSight,

			InSet,
			Visited,
			Visible,
			WasSeen,

			Lit,
		};

		enum_flags<TileFlags> Flags;
	};

	struct BlockNavigationGrid : BaseNavigationGrid<BlockNavigationTile>
	{
		void ClearData(enum_flags<BlockNavigationTile::TileFlags> unset_flags = enum_flags<BlockNavigationTile::TileFlags>::all());

		/// Uses the `BlocksPassage` flag to determine whether tiles are adjacent
		std::vector<ivec2> BreadthFirstSearch(ivec2 start, ivec2 goal, bool diagonals = true);

		/// Uses the `BlocksPassage` flag to determine whether tiles are adjacent
		std::vector<ivec2> DijkstraSearch(ivec2 start, ivec2 goal, double max_cost, bool diagonals = true);

		/// Uses the `BlocksPassage` flag to determine whether tiles are adjacent
		std::vector<ivec2> AStarSearch(ivec2 start, ivec2 goal, bool diagonals = true);

#define FLAG_METHODS(name) \
	void Set##name(ivec2 pos, bool value) noexcept { At(pos)->Flags.set_to(value, BlockNavigationTile::TileFlags::name); } \
	bool name(ivec2 pos) const noexcept { return At(pos)->Flags.is_set(BlockNavigationTile::TileFlags::name); } \
	void SetAll##name(bool value) { ForEach([this, value](ivec2 pos) { Set##name(pos, value); return false; }); }

		FLAG_METHODS(InSet)
		FLAG_METHODS(Visited)
		FLAG_METHODS(BlocksPassage)
		FLAG_METHODS(BlocksSight)
		FLAG_METHODS(Visible)
		FLAG_METHODS(WasSeen)
		FLAG_METHODS(Lit)

		template <typename FUNC>
		void SmoothPath(std::vector<ivec2>& path, FUNC&& blocks_func) const;

		/// Uses the `BlocksSight` flag to determine whether a tile blocks sight
		void CalculateFOV(ivec2 source, int max_radius, bool include_walls);

		/// Uses the `BlocksSight` flag to determine whether a tile blocks sight
		bool CanSee(ivec2 start, ivec2 end, bool ignore_start) const;

		template <typename IS_TRANSPARENT_FUNC, typename SET_VISIBLE_FUNC>
		void CalculateFOV(ivec2 source, int max_radius, bool include_walls, IS_TRANSPARENT_FUNC&& is_transparent, SET_VISIBLE_FUNC&& set_visible);

	protected:

		template <typename IS_TRANSPARENT_FUNC, typename SET_VISIBLE_FUNC>
		void CastFOV(ivec2 center, int row, float start, float end, int radius, int r2, int xx, int xy, int yx, int yy, int id, bool light_walls,
			const IS_TRANSPARENT_FUNC& is_transparent, const SET_VISIBLE_FUNC& set_visible);

	};

	enum WallBlocks
	{
		Passage,
		Sight,

		Count
	};

	struct WallNavigationTile : BaseNavigationTile
	{
		enum class TileFlags
		{
			InSet,
			Visited,
			Visible,
			WasSeen,
			Lit,
		};

		enum_flags<TileFlags> Flags;

		std::array<DirectionBitmap, (size_t)WallBlocks::Count> Blocks{};
	};

	struct WallNavigationGrid : BaseNavigationGrid<WallNavigationTile>
	{
		void ClearData(enum_flags<WallNavigationTile::TileFlags> unset_flags = enum_flags<WallNavigationTile::TileFlags>::all());

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid, IterationFlags::Diagonals), typename WALL_FUNCTION>
		void BuildWalls(WALL_FUNCTION&& wall_func);

		/*
		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::WithSelf, IterationFlags::OnlyValid), typename FUNC>
		auto ForEachNonBlockingNeighbor(ivec2 of, FUNC&& func) const;
		*/

		bool Blocks(ivec2 from, Direction dir, WallBlocks what) const;
		bool Blocks(ivec2 from, ivec2 to, WallBlocks what) const;

		void SetBlocking(ivec2 from, Direction dir, enum_flags<WallBlocks> what);
		void SetBlocking(ivec2 from, ivec2 to, enum_flags<WallBlocks> what, bool two_ways = true);
		void SetBlocking(irec2 const& room, enum_flags<WallBlocks> what, bool two_ways = true);

		void SetNonBlocking(ivec2 from, Direction dir, enum_flags<WallBlocks> what);
		void SetNonBlocking(ivec2 from, ivec2 to, enum_flags<WallBlocks> what, bool two_ways = true);
		void SetNonBlocking(irec2 const& room, enum_flags<WallBlocks> what, bool two_ways = true);

		bool Visible(ivec2 from, ivec2 to) const { return !Blocks(from, to, WallBlocks::Sight); }
		bool Passable(ivec2 from, ivec2 to) const { return !Blocks(from, to, WallBlocks::Passage); }
	};
}

#include "Navigation.impl.h"