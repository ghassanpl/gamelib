#pragma once

#include "../Includes/GLM.h"
#include "../Includes/EnumFlags.h"
#include "../Colors.h"
#include "Grid.h"

namespace gamelib::squares
{
	struct RaycastResult
	{
		double Distance = 0;
		ivec2 Tile{ -1, -1 };
		ivec2 Neighbor{ -1, -1 };
		vec2 HitPosition{ 0,0 };
		Direction Wall = Direction::None;
		bool Hit = false;
	};

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

		/// Returns first hit
		template <typename PASSABLE_FUNCTION, typename ENTERED_TILE_FUNCTION>
		RaycastResult RayCast(vec2 tile_size, vec2 start, vec2 direction, PASSABLE_FUNCTION&& passable_func, ENTERED_TILE_FUNCTION&& entered_tile_func, double max_distance = std::numeric_limits<double>::max());

		template <typename PASSABLE_FUNCTION, typename ENTERED_TILE_FUNCTION>
		RaycastResult SegmentCast(vec2 tile_size, vec2 start, vec2 end, PASSABLE_FUNCTION&& passable_func, ENTERED_TILE_FUNCTION&& entered_tile_func);

		/// Goes through all hits unles HIT_FUNCTION returns false
		template <typename PASSABLE_FUNCTION, typename ENTERED_TILE_FUNCTION, typename HIT_FUNCTION>
		void RayCastCallback(vec2 tile_size, vec2 start, vec2 direction, PASSABLE_FUNCTION&& passable_func, ENTERED_TILE_FUNCTION&& entered_tile_func, HIT_FUNCTION&& hit_func, double max_distance = std::numeric_limits<double>::max());

		double& Cost(ivec2 pos) noexcept { return this->At(pos)->Cost; }
		double Cost(ivec2 pos) const noexcept { return this->At(pos)->Cost; }
		ivec2& Predecessor(ivec2 pos) noexcept { return this->At(pos)->Predecessor; }
		ivec2 Predecessor(ivec2 pos) const noexcept { return this->At(pos)->Predecessor; }

		bool HasCost(ivec2 pos) const noexcept { return !std::isnan(this->At(pos)->Cost); }

		inline static double DefaultCostFunction(ivec2 a, ivec2 b) noexcept { return (double)glm::length(vec2(a - b)); }

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

#undef FLAG_METHODS

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
		Sight
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

		std::array<enum_flags<WallBlocks>, 8> Blocks{};
		DirectionBitmap BlocksWall(WallBlocks blocks) const
		{
			DirectionBitmap result{};
			AllDirections.for_each([&] (Direction dir) {
				if (Blocks[(int)dir].is_set(blocks))
					result.set(dir);
			});
			return result;
		}
	};

	struct WallNavigationGrid : BaseNavigationGrid<WallNavigationTile>
	{
		void ClearData(enum_flags<WallNavigationTile::TileFlags> unset_flags = enum_flags<WallNavigationTile::TileFlags>::all());

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid, IterationFlags::Diagonals), typename WALL_FUNCTION>
		void BuildWalls(WALL_FUNCTION&& wall_func);

		using BaseNavigationGrid::BreadthFirstSearch;
		/// Uses the `BlocksPassage` flag to determine whether tiles are adjacent
		std::vector<ivec2> BreadthFirstSearch(ivec2 start, ivec2 goal, bool diagonals = true);

		using BaseNavigationGrid::DijkstraSearch;
		/// Uses the `BlocksPassage` flag to determine whether tiles are adjacent
		std::vector<ivec2> DijkstraSearch(ivec2 start, ivec2 goal, double max_cost, bool diagonals = true);

		using BaseNavigationGrid::AStarSearch;
		/// Uses the `BlocksPassage` flag to determine whether tiles are adjacent
		std::vector<ivec2> AStarSearch(ivec2 start, ivec2 goal, bool diagonals = true);

		/*
		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::WithSelf, IterationFlags::OnlyValid), typename FUNC>
		auto ForEachNonBlockingNeighbor(ivec2 of, FUNC&& func) const;
		*/

		bool Blocks(ivec2 from, Direction dir, WallBlocks what) const;
		bool Blocks(ivec2 from, ivec2 to, WallBlocks what) const;

		enum_flags<WallBlocks>& BlocksIn(ivec2 from, Direction dir) { return At(from)->Blocks[(int)dir]; }
		enum_flags<WallBlocks> const& BlocksIn(ivec2 from, Direction dir) const { return At(from)->Blocks[(int)dir]; }

		enum_flags<WallBlocks>& BlocksIn(ivec2 from, ivec2 to);
		enum_flags<WallBlocks> const& BlocksIn(ivec2 from, ivec2 to) const;

		void SetBlocking(ivec2 from, Direction dir, enum_flags<WallBlocks> what, bool blocking);
		void SetBlocking(ivec2 from, ivec2 to, enum_flags<WallBlocks> what, bool blocking, bool two_ways = true);
		void SetBlocking(irec2 const& room, enum_flags<WallBlocks> what, bool blocking, bool two_ways = true);

#define FLAG_METHODS(name) \
	void Set##name(ivec2 pos, bool value) noexcept { At(pos)->Flags.set_to(value, WallNavigationTile::TileFlags::name); } \
	bool name(ivec2 pos) const noexcept { return At(pos)->Flags.is_set(WallNavigationTile::TileFlags::name); } \
	void SetAll##name(bool value) { ForEach([this, value](ivec2 pos) { Set##name(pos, value); return false; }); }

		FLAG_METHODS(InSet)
		FLAG_METHODS(Visited)
		FLAG_METHODS(Visible)
		FLAG_METHODS(WasSeen)
		FLAG_METHODS(Lit)

#undef FLAG_METHODS

		bool BlocksSight(ivec2 from, ivec2 to) const { return Blocks(from, to, WallBlocks::Sight); }
		bool BlocksPassage(ivec2 from, ivec2 to) const { return Blocks(from, to, WallBlocks::Passage); }
		
		void SetBlocksPassage(ivec2 from, ivec2 to, bool passable, bool two_ways = true) { SetBlocking(from, to, WallBlocks::Passage, passable, two_ways); }

		RaycastResult RayCast(vec2 tile_size, vec2 start, vec2 direction, WallBlocks blocking, double max_distance);
		RaycastResult SegmentCast(vec2 tile_size, vec2 start, vec2 end, WallBlocks blocking);
	};
}

#include "Navigation.impl.h"