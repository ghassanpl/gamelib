#include "Navigation.h"
#pragma once

namespace gamelib::squares
{
	/// TODO: http://squidpony.github.io/SquidLib/squidlib/apidocs/squidpony/squidgrid/gui/gdx/LightingHandler.html

	inline bool NavigationGrid::Adjacent(ivec2 const from, ivec2 const to) const
	{
		if (from == to) return true;
		if (!IsSurrounding(from, to)) return false;
		return At(from)->Adjacency.is_set(ToDirection(to - from));
	}

	template <uint64_t FLAGS, typename PASSABLE_FUNCTION>
	inline void NavigationGrid::BuildAdjacency(PASSABLE_FUNCTION&& passable_func)
	{
		static constexpr auto ONLY_VALID = ghassanpl::is_flag_set(FLAGS, IterationFlags::OnlyValid);
		static constexpr auto dirs = ghassanpl::is_flag_set(FLAGS, IterationFlags::Diagonals) ? AllDirections : AllCardinalDirections;

		ForEach<FLAGS>([this, &passable_func](ivec2 pos) {
			auto& adj = At(pos)->Adjacency;
			adj.bits = 0;
			dirs.for_each([this, &adj, pos, &passable_func](Direction dir) {
				const auto neighbor = pos + ToVector(dir);
				if constexpr (ONLY_VALID) if (!IsValid(neighbor)) return;
				if (passable_func(pos, neighbor))
					adj.set(dir);
			});
		});
	}

	template<uint64_t FLAGS>
	inline void NavigationGrid::BuildAdjacency()
	{
		static constexpr auto ONLY_VALID = ghassanpl::is_flag_set(FLAGS, IterationFlags::OnlyValid);
		BuildAdjacency<FLAGS>([this](ivec2 from, ivec2 to) {
			if constexpr (ONLY_VALID) if (!IsValid(to)) return false;
			return !BlocksPassage(to) || (!IsDiagonalNeighbor(from, to) || (!BlocksPassage({ from.x, to.y }) && BlocksPassage({ to.x, from.y })));
		});
	}

	template<uint64_t FLAGS, typename FUNC>
	inline auto NavigationGrid::ForEachAdjacentNeighbor(ivec2 of, FUNC&& func) const
	{
		return ForEachSelectedNeighbor<FLAGS>(of, At(of)->Adjacency, std::forward<FUNC>(func));
	}

	template<bool DIAGONALS, typename PASSABLE_FUNCTION>
	std::vector<ivec2> NavigationGrid::BreadthFirstSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func)
	{
		std::queue<ivec2> frontier;
		frontier.push(start);

		ClearData({});

		Predecessor(start) = start;

		while (!frontier.empty())
		{
			auto current = frontier.front();
			frontier.pop();

			if (current == goal)
				break;

			static constexpr auto Diagonals = DIAGONALS ? ghassanpl::flag_bits(IterationFlags::Diagonals) : 0ULL;
			ForEachNeighbor<ghassanpl::flag_bits(IterationFlags::OnlyValid) | Diagonals>(current, [&](ivec2 next) {
				if (!passable_func(current, next)) return;

				if (!IsValid(Predecessor(next)))
				{
					frontier.push(next);
					Predecessor(next) = current;
				}
			});
		}

		return ReconstructPath(start, goal);
	}

	template<bool DIAGONALS, typename PASSABLE_FUNCTION, typename COST_FUNCTION>
	inline std::vector<ivec2> NavigationGrid::DijkstraSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func, double max_cost, COST_FUNCTION&& cost_function)
	{
		mSearchFrontier.clear();
		PutSearchFrontierItem(start, 0);

		ClearData({});

		Predecessor(start) = start;
		Cost(start) = 0;

		while (!mSearchFrontier.empty())
		{
			auto current = GetSearchFrontierItem();

			if (current == goal)
				break;

			static constexpr auto Diagonals = DIAGONALS ? ghassanpl::flag_bits(IterationFlags::Diagonals) : 0ULL;
			ForEachNeighbor<ghassanpl::flag_bits(IterationFlags::OnlyValid) | Diagonals>(current, [&](ivec2 next) {
				if (!passable_func(current, next)) return;

				auto new_cost = Cost(current) + cost_function(current, next);
				if ((!HasCost(next) || new_cost < Cost(next)) && new_cost <= max_cost)
				{
					Cost(next) = new_cost;
					Predecessor(next) = current;
					PutSearchFrontierItem(next, new_cost);
				}
			});
		}

		return ReconstructPath(start, goal);
	}

	template<bool DIAGONALS, typename PASSABLE_FUNCTION, typename HEURISTIC_FUNCTION, typename COST_FUNCTION>
	inline std::vector<ivec2> NavigationGrid::AStarSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func, HEURISTIC_FUNCTION&& heuristic, COST_FUNCTION&& cost_function)
	{
		mSearchFrontier.clear();
		PutSearchFrontierItem(start, 0);

		ClearData({});

		Predecessor(start) = start;
		Cost(start) = 0;

		while (!mSearchFrontier.empty())
		{
			auto current = GetSearchFrontierItem();

			if (current == goal)
				break;

			static constexpr auto Diagonals = DIAGONALS ? ghassanpl::flag_bits(IterationFlags::Diagonals) : 0ULL;
			ForEachNeighbor<ghassanpl::flag_bits(IterationFlags::OnlyValid) | Diagonals>(current, [&](ivec2 next) {
				if (!passable_func(current, next)) return;

				auto new_cost = Cost(current) + cost_function(current, next);
				if (!HasCost(next) || new_cost < Cost(next))
				{
					Cost(next) = new_cost;
					auto priority = new_cost + heuristic(next, goal);
					PutSearchFrontierItem(next, priority);
					Predecessor(next) = current;
				}
			});
		}

		return ReconstructPath(start, goal);
	}

	template<typename IS_TRANSPARENT_FUNC, typename SET_VISIBLE_FUNC>
	inline void NavigationGrid::CalculateFOV(ivec2 source, int max_radius, bool include_walls, IS_TRANSPARENT_FUNC&& is_transparent, SET_VISIBLE_FUNC&& set_visible)
	{
		ClearData(NavigationTile::TileFlags::Visible);

		static constexpr int mult[4][8] = {
			{ 1, 0, 0, -1, -1, 0, 0, 1 },
			{ 0, 1, -1, 0, 0, -1, 1, 0 },
			{ 0, 1, 1, 0, 0, -1, -1, 0 },
			{ 1, 0, 0, 1, -1, 0, 0, -1 },
		};

		if (max_radius < 0)
		{
			int max_radius_x = int(mWidth) - source.x;
			int max_radius_y = int(mHeight) - source.y;
			max_radius_x = std::max(max_radius_x, source.x);
			max_radius_y = std::max(max_radius_y, source.y);
			max_radius = (int)(std::sqrt(max_radius_x * max_radius_x + max_radius_y * max_radius_y)) + 1;
		}

		int r2 = max_radius * max_radius;

		for (int oct = 0; oct < 8; oct++)
			CastFOV(source, 1, 1.0, 0.0, max_radius, r2, mult[0][oct], mult[1][oct], mult[2][oct], mult[3][oct], 0, include_walls, is_transparent, set_visible);
		set_visible(source);
	}

	template<typename IS_TRANSPARENT_FUNC, typename SET_VISIBLE_FUNC>
	inline void NavigationGrid::CastFOV(ivec2 center, int row, float start, float end, int radius, int r2, int xx, int xy, int yx, int yy, int id,
		bool light_walls, const IS_TRANSPARENT_FUNC& is_transparent, const SET_VISIBLE_FUNC& set_visible)
	{
		float new_start = 0.0f;
		if (start < end) return;

		for (int j = row; j < radius + 1; j++)
		{
			int dx = -j - 1;
			int dy = -j;
			bool blocked = false;
			while (dx <= 0)
			{
				dx++;

				int X = center.x + dx * xx + dy * xy;
				int Y = center.y + dx * yx + dy * yy;
				if ((unsigned)X >= (unsigned)mWidth || (unsigned)Y >= (unsigned)mHeight)
					continue;

				const auto l_slope = (dx - 0.5f) / (dy + 0.5f);
				const auto r_slope = (dx + 0.5f) / (dy - 0.5f);
				if (start < r_slope) continue;
				else if (end > l_slope) break;

				const auto is_trans = is_transparent({ X, Y });

				if (dx * dx + dy * dy <= r2 && (light_walls || is_trans))
				{
					set_visible({ X, Y });
				}

				if (blocked)
				{
					if (!is_trans)
					{
						new_start = r_slope;
						continue;
					}
					else
					{
						blocked = false;
						start = new_start;
					}
				}
				else
				{
					if (!is_trans && j < radius)
					{
						blocked = true;
						CastFOV(center, j + 1, start, l_slope, radius, r2, xx, xy, yx, yy, id + 1, light_walls, is_transparent, set_visible);
						new_start = r_slope;
					}
				}
			}
			if (blocked) break;
		}
	}

}