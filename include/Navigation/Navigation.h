#pragma once

#include "../Includes/GLM.h"
#include "../Includes/EnumFlags.h"
#include "../Colors.h"
#include "Grid.h"

namespace gamelib::squares
{

	struct NavigationTile
	{
		enum class TileFlags
		{
			InSet,
			Visited,
			BlocksPassage,

			BlocksSight,
			Visible,
			WasSeen,

			Lit,
		};

		double Cost = std::numeric_limits<double>::quiet_NaN();
		ghassanpl::enum_flags<TileFlags> Flags;
		ivec2 Predecessor{ -1, -1 };
		Color LightColor = Colors::Black;
		DirectionBitmap Adjacency{ 0xFF };
	};

	struct NavigationGrid : Grid<NavigationTile>
	{
		void ClearData(ghassanpl::enum_flags<NavigationTile::TileFlags> unset_flags = ghassanpl::enum_flags<NavigationTile::TileFlags>::all());

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid, IterationFlags::Diagonals), typename PASSABLE_FUNCTION>
		void BuildAdjacency(PASSABLE_FUNCTION&& passable_func);

		/// Uses the `BlocksPassage` flag to determine whether tiles are adjacent
		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid, IterationFlags::Diagonals)>
		void BuildAdjacency();

		void BreakAdjacency(ivec2 pos, Direction dir, bool two_ways = true);
		void SetAdjacent(ivec2 pos, Direction dir, bool two_ways = true);

		void BreakAdjacency(irec2 const& room);

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::WithSelf, IterationFlags::OnlyValid), typename FUNC>
		auto ForEachAdjacentNeighbor(ivec2 of, FUNC&& func) const;

		/// Returns the REVERSED path, for ease of popping
		template <bool DIAGONALS = true, typename PASSABLE_FUNCTION>
		std::vector<ivec2> BreadthFirstSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func);

		/// Uses the `BlocksPassage` flag to determine whether tiles are adjacent
		std::vector<ivec2> BreadthFirstSearch(ivec2 start, ivec2 goal, bool diagonals = true);

		/// Returns the REVERSED path, for ease of popping
		template <bool DIAGONALS = true, typename PASSABLE_FUNCTION, typename COST_FUNCTION>
		std::vector<ivec2> DijkstraSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func, double max_cost, COST_FUNCTION&& cost_function);

		/// Uses the `BlocksPassage` flag to determine whether tiles are adjacent
		std::vector<ivec2> DijkstraSearch(ivec2 start, ivec2 goal, double max_cost, bool diagonals = true);

		/// Returns the REVERSED path, for ease of popping
		template <bool DIAGONALS = true, typename PASSABLE_FUNCTION, typename HEURISTIC_FUNCTION, typename COST_FUNCTION>
		std::vector<ivec2> AStarSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func, HEURISTIC_FUNCTION&& heuristic, COST_FUNCTION&& cost_function);

		/// Uses the `BlocksPassage` flag to determine whether tiles are adjacent
		std::vector<ivec2> AStarSearch(ivec2 start, ivec2 goal, bool diagonals = true);

		double& Cost(ivec2 pos) noexcept { return At(pos)->Cost; }
		double Cost(ivec2 pos) const noexcept { return At(pos)->Cost; }
		ivec2& Predecessor(ivec2 pos) noexcept { return At(pos)->Predecessor; }
		ivec2 Predecessor(ivec2 pos) const noexcept { return At(pos)->Predecessor; }

		bool HasCost(ivec2 pos) const noexcept { return !std::isnan(At(pos)->Cost); }

#define FLAG_METHODS(name) \
	void Set##name(ivec2 pos, bool value) noexcept { At(pos)->Flags.set_to(value, NavigationTile::TileFlags::name); } \
	bool name(ivec2 pos) const noexcept { return At(pos)->Flags.is_set(NavigationTile::TileFlags::name); } \
	void SetAll##name(bool value) { ForEach([this, value](ivec2 pos) { Set##name(pos, value); return false; }); }

		FLAG_METHODS(InSet)
		FLAG_METHODS(Visited)
		FLAG_METHODS(BlocksPassage)
		FLAG_METHODS(BlocksSight)
		FLAG_METHODS(Visible)
		FLAG_METHODS(WasSeen)
		FLAG_METHODS(Lit)

		bool Adjacent(ivec2 from, ivec2 to) const;

		template <typename FUNC>
		void SmoothPath(std::vector<ivec2>& path, FUNC&& blocks_func) const;

		template <typename IS_TRANSPARENT_FUNC, typename SET_VISIBLE_FUNC>
		void CalculateFOV(ivec2 source, int max_radius, bool include_walls, IS_TRANSPARENT_FUNC&& is_transparent, SET_VISIBLE_FUNC&& set_visible);

		void CalculateFOV(ivec2 source, int max_radius, bool include_walls);

		bool CanSee(ivec2 start, ivec2 end, bool ignore_start) const;

	private:

		template <typename IS_TRANSPARENT_FUNC, typename SET_VISIBLE_FUNC>
		void CastFOV(ivec2 center, int row, float start, float end, int radius, int r2, int xx, int xy, int yx, int yy, int id, bool light_walls,
			const IS_TRANSPARENT_FUNC& is_transparent, const SET_VISIBLE_FUNC& set_visible);

		std::vector<ivec2> ReconstructPath(ivec2 start, ivec2 goal) const;

		std::vector<std::pair<double, ivec2>> mSearchFrontier;

		void PutSearchFrontierItem(ivec2 item, double priority);

		ivec2 GetSearchFrontierItem();
	};
}

#include "Navigation.impl.h"