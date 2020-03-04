#pragma once

#include "Squares.h"
#include "../Random.h"
#include <vector>
#include <set>

namespace gamelib::squares
{

	template <typename T>
	using ValidNeighborsFunc = std::vector<ivec2>(ivec2 room);
	template <typename T>
	using ConnectRoomFunc = void(ivec2 room, ivec2 parent_room);

	template <typename RANDOM, typename VALID_NEIGHBORS_FUNC, typename CONNECT_ROOM_FUNC>
	void GenerateMaze(ivec2 start_room, RANDOM& rng, VALID_NEIGHBORS_FUNC&& valid_neighbors, CONNECT_ROOM_FUNC&& connect_room)
	{
		std::vector<std::pair<ivec2, ivec2>> wall_list;
		std::set<ivec2> visited;

		visited.insert(start_room);
		for (auto neighbor : valid_neighbors(start_room))
			wall_list.push_back({ start_room, neighbor });

		while (!wall_list.empty())
		{
			auto it = random::Iterator(rng, wall_list);
			auto [parent, child] = *it;
			wall_list.erase(it);

			if (visited.count(parent) != visited.count(child))
			{
				visited.insert(child);
				connect_room(child, parent);
				for (auto neighbor : valid_neighbors(child))
					wall_list.push_back({ child, neighbor });
			}
		}
	}


}