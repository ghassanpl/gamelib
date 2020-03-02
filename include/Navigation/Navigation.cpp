#include "Navigation.h"

namespace gamelib::squares
{

	void NavigationGrid::ClearData(ghassanpl::enum_flags<NavigationTile::TileFlags> unset_flags)
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
	void NavigationGrid::InitFrom(TileLayer const* layer)
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

	std::vector<ivec2> NavigationGrid::BreadthFirstSearch(ivec2 start, ivec2 goal, bool diagonals)
	{
		if (diagonals)
		{
			return BreadthFirstSearch<true>(start, goal, [&, goal](ivec2 from, ivec2 to) {
				return (to == goal || !BlocksPassage(to)) && (!IsDiagonalNeighbor(from, to) || (!BlocksPassage({ from.x, to.y }) && BlocksPassage({ to.x, from.y })));
				});
		}
		else
		{
			return BreadthFirstSearch<false>(start, goal, [&, goal](ivec2 from, ivec2 to) { return (to == goal || !BlocksPassage(to)); });
		}
	}

	inline double len(ivec2 a, ivec2 b) noexcept { return (double)glm::length(vec2(a - b)); }

	std::vector<ivec2> NavigationGrid::DijkstraSearch(ivec2 start, ivec2 goal, double max_cost, bool diagonals)
	{
		if (diagonals)
		{
			return DijkstraSearch<true>(start, goal, [&, goal](ivec2 from, ivec2 to) {
				return (to == goal || !BlocksPassage(to)) && (!IsDiagonalNeighbor(from, to) || (!BlocksPassage({ from.x, to.y }) && BlocksPassage({ to.x, from.y })));
				}, max_cost, len);
		}
		else
		{
			return DijkstraSearch<false>(start, goal, [&, goal](ivec2 from, ivec2 to) {
				return (to == goal || !BlocksPassage(to));
				}, max_cost, len);
		}
	}

	std::vector<ivec2> NavigationGrid::AStarSearch(ivec2 start, ivec2 goal, bool diagonals)
	{
		if (diagonals)
		{
			return AStarSearch<true>(start, goal, [&, goal](ivec2 from, ivec2 to) {
				return (to == goal || !BlocksPassage(to)) && (!IsDiagonalNeighbor(from, to) || (!BlocksPassage({ from.x, to.y }) && BlocksPassage({ to.x, from.y })));
				}, len, len);
		}
		else
		{
			return AStarSearch<false>(start, goal, [&, goal](ivec2 from, ivec2 to) {
				return (to == goal || !BlocksPassage(to));
				}, len, len);
		}
	}

	void NavigationGrid::CalculateFOV(ivec2 source, int max_radius, bool include_walls)
	{
		CalculateFOV(source, max_radius, include_walls, 
			[this](ivec2 pos) {
				return !BlocksSight(pos);
			},
			[this](ivec2 pos) {
				At(pos)->Flags.set(NavigationTile::TileFlags::Visible, NavigationTile::TileFlags::WasSeen);
			}
		);
	}

	std::vector<ivec2> NavigationGrid::ReconstructPath(ivec2 start, ivec2 goal) const
	{
		if (start == goal) return { start };

		std::vector<ivec2> path;

		/// We do path simplification here already
		auto last_dif = ivec2{ 0,0 };
		auto last_pos = goal;
		auto current = Predecessor(goal);
		while (current != start)
		{
			if (!IsValid(current))
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

	void NavigationGrid::PutSearchFrontierItem(ivec2 item, double priority)
	{
		mSearchFrontier.emplace_back(priority, item);
		std::push_heap(mSearchFrontier.begin(), mSearchFrontier.end(), comparer);
	}

	ivec2 NavigationGrid::GetSearchFrontierItem()
	{
		ivec2 best_item = mSearchFrontier.front().second;
		std::pop_heap(mSearchFrontier.begin(), mSearchFrontier.end(), comparer);
		mSearchFrontier.pop_back();
		return best_item;
	}

	bool NavigationGrid::CanSee(ivec2 start, ivec2 end, bool ignore_start) const
	{
		return LineCast(start, end, [this](ivec2 pos) { return BlocksSight(pos); }, ignore_start);
	}

}