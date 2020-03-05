#include "Navigation.h"
#pragma once

namespace gamelib::squares
{
	/// TODO: http://squidpony.github.io/SquidLib/squidlib/apidocs/squidpony/squidgrid/gui/gdx/LightingHandler.html

	template <uint64_t FLAGS, typename WALL_FUNCTION>
	inline void WallNavigationGrid::BuildWalls(WALL_FUNCTION&& wall_func)
	{
		static constexpr auto ONLY_VALID = ghassanpl::is_flag_set(FLAGS, IterationFlags::OnlyValid);
		static constexpr auto dirs = ghassanpl::is_flag_set(FLAGS, IterationFlags::Diagonals) ? AllDirections : AllCardinalDirections;

		this->ForEach<FLAGS>([this, &wall_func](ivec2 pos) {
			auto& adj = At(pos)->Blocks = {};
			dirs.for_each([this, &adj, pos, &wall_func](Direction dir) {
				const auto neighbor = pos + ToVector(dir);
				//if constexpr (ONLY_VALID) if (!IsValid(neighbor)) return;
				enum_flags<WallBlocks> wall_blocks = wall_func(pos, neighbor);
				wall_blocks.for_each([&adj, dir](WallBlocks blocks) {
					adj[(int)blocks].set(dir);
				});
			});
		});
	}

	/*
	template<uint64_t FLAGS, typename FUNC>
	inline auto WallNavigationGrid::ForEachAdjacentNeighbor(ivec2 of, FUNC&& func) const
	{
		return this->ForEachSelectedNeighbor<FLAGS>(of, At(of)->Adjacency, std::forward<FUNC>(func));
	}
	*/

	template<typename TILE_DATA>
	template<bool DIAGONALS, typename PASSABLE_FUNCTION>
	std::vector<ivec2> BaseNavigationGrid<TILE_DATA>::BreadthFirstSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func)
	{
		std::queue<ivec2> frontier;
		frontier.push(start);

		ClearData();

		Predecessor(start) = start;

		while (!frontier.empty())
		{
			auto current = frontier.front();
			frontier.pop();

			if (current == goal)
				break;

			static constexpr auto Diagonals = DIAGONALS ? ghassanpl::flag_bits(Grid<TILE_DATA>::IterationFlags::Diagonals) : 0ULL;
			this->ForEachNeighbor<ghassanpl::flag_bits(Grid<TILE_DATA>::IterationFlags::OnlyValid) | Diagonals>(current, [&](ivec2 next) {
				if (!passable_func(current, next)) return;

				if (!this->IsValid(Predecessor(next)))
				{
					frontier.push(next);
					Predecessor(next) = current;
				}
			});
		}

		return ReconstructPath(start, goal);
	}

	template<typename TILE_DATA>
	template<bool DIAGONALS, typename PASSABLE_FUNCTION, typename COST_FUNCTION>
	inline std::vector<ivec2> BaseNavigationGrid<TILE_DATA>::DijkstraSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func, double max_cost, COST_FUNCTION&& cost_function)
	{
		mSearchFrontier.clear();
		PutSearchFrontierItem(start, 0);

		ClearData();

		Predecessor(start) = start;
		Cost(start) = 0;

		while (!mSearchFrontier.empty())
		{
			auto current = GetSearchFrontierItem();

			if (current == goal)
				break;

			static constexpr auto Diagonals = DIAGONALS ? ghassanpl::flag_bits(Grid<TILE_DATA>::IterationFlags::Diagonals) : 0ULL;
			this->ForEachNeighbor<ghassanpl::flag_bits(Grid<TILE_DATA>::IterationFlags::OnlyValid) | Diagonals>(current, [&](ivec2 next) {
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

	template<typename TILE_DATA>
	template<bool DIAGONALS, typename PASSABLE_FUNCTION, typename HEURISTIC_FUNCTION, typename COST_FUNCTION>
	inline std::vector<ivec2> BaseNavigationGrid<TILE_DATA>::AStarSearch(ivec2 start, ivec2 goal, PASSABLE_FUNCTION&& passable_func, HEURISTIC_FUNCTION&& heuristic, COST_FUNCTION&& cost_function)
	{
		mSearchFrontier.clear();
		PutSearchFrontierItem(start, 0);

		ClearData();

		Predecessor(start) = start;
		Cost(start) = 0;

		while (!mSearchFrontier.empty())
		{
			auto current = GetSearchFrontierItem();

			if (current == goal)
				break;

			static constexpr auto Diagonals = DIAGONALS ? ghassanpl::flag_bits(Grid<TILE_DATA>::IterationFlags::Diagonals) : 0ULL;
			this->ForEachNeighbor<ghassanpl::flag_bits(Grid<TILE_DATA>::IterationFlags::OnlyValid) | Diagonals>(current, [&](ivec2 next) {
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
	inline void BlockNavigationGrid::CalculateFOV(ivec2 source, int max_radius, bool include_walls, IS_TRANSPARENT_FUNC&& is_transparent, SET_VISIBLE_FUNC&& set_visible)
	{
		static constexpr int mult[4][8] = {
			{ 1, 0, 0, -1, -1, 0, 0, 1 },
			{ 0, 1, -1, 0, 0, -1, 1, 0 },
			{ 0, 1, 1, 0, 0, -1, -1, 0 },
			{ 1, 0, 0, 1, -1, 0, 0, -1 },
		};

		if (max_radius < 0)
		{
			int max_radius_x = int(this->mWidth) - source.x;
			int max_radius_y = int(this->mHeight) - source.y;
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
	inline void BlockNavigationGrid::CastFOV(ivec2 center, int row, float start, float end, int radius, int r2, int xx, int xy, int yx, int yy, int id,
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
				if ((unsigned)X >= (unsigned)this->mWidth || (unsigned)Y >= (unsigned)this->mHeight)
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


	template<typename TILE_DATA>
	inline void BaseNavigationGrid<TILE_DATA>::ClearData()
	{
		for (auto& tile : this->mTiles)
		{
			tile.Cost = std::numeric_limits<double>::quiet_NaN();
			tile.Predecessor = { -1, -1 };
		}
	}

	template<typename TILE_DATA>
	std::vector<ivec2> BaseNavigationGrid<TILE_DATA>::ReconstructPath(ivec2 start, ivec2 goal) const
	{
		if (start == goal) return { start };

		std::vector<ivec2> path;

		/// We do path simplification here already
		auto last_dif = ivec2{ 0,0 };
		auto last_pos = goal;
		auto current = Predecessor(goal);
		while (current != start)
		{
			if (!this->IsValid(current))
				return path;
			auto dif = current - last_pos;
			if (dif != last_dif)
			{
				last_dif = dif;
				path.push_back(last_pos);
			}
			last_pos = current;
			current = Predecessor(current);
		}
		path.push_back(last_pos);
		path.push_back(current);

		return path;
	}

	static const auto comparer = [](const auto& p1, const auto& p2) { return p1.first > p2.first; };

	template<typename TILE_DATA>
	void BaseNavigationGrid<TILE_DATA>::PutSearchFrontierItem(ivec2 item, double priority)
	{
		mSearchFrontier.emplace_back(priority, item);
		std::push_heap(mSearchFrontier.begin(), mSearchFrontier.end(), comparer);
	}

	template<typename TILE_DATA>
	ivec2 BaseNavigationGrid<TILE_DATA>::GetSearchFrontierItem()
	{
		ivec2 best_item = mSearchFrontier.front().second;
		std::pop_heap(mSearchFrontier.begin(), mSearchFrontier.end(), comparer);
		mSearchFrontier.pop_back();
		return best_item;
	}

}