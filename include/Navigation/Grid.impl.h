#include <set>
#include <queue>
#include "Grid.h"

namespace gamelib::squares
{
	template<typename TILE_DATA>
	template<uint64_t FLAGS, typename FUNC>
	inline auto Grid<TILE_DATA>::ForEachNeighbor(ivec2 of, FUNC&& func) const
	{
		using return_type = decltype(func(ivec2{}));
		static_assert(std::is_void_v<return_type> || std::is_convertible_v<return_type, bool>, "return type of tile callback must be either void or convertible to bool");
		static constexpr auto ONLY_VALID = ghassanpl::is_flag_set(FLAGS, IterationFlags::OnlyValid);
		
		if constexpr (std::is_void_v<return_type>)
		{
			if constexpr (ghassanpl::is_flag_set(FLAGS, IterationFlags::WithSelf))
				Apply<ONLY_VALID>(of, func);
			Apply<ONLY_VALID>({ of.x - 1, of.y }, func);
			Apply<ONLY_VALID>({ of.x + 1, of.y }, func);
			Apply<ONLY_VALID>({ of.x, of.y - 1 }, func);
			Apply<ONLY_VALID>({ of.x, of.y + 1 }, func);

			if constexpr (ghassanpl::is_flag_set(FLAGS, IterationFlags::Diagonals))
			{
				Apply<ONLY_VALID>({ of.x - 1, of.y - 1 }, func);
				Apply<ONLY_VALID>({ of.x + 1, of.y + 1 }, func);
				Apply<ONLY_VALID>({ of.x + 1, of.y - 1 }, func);
				Apply<ONLY_VALID>({ of.x - 1, of.y + 1 }, func);
			}
		}
		else
		{
			if constexpr (ghassanpl::is_flag_set(FLAGS, IterationFlags::WithSelf))
				if (auto ret = Apply<ONLY_VALID>(of, func)) return ret;
			if (auto ret = Apply<ONLY_VALID>({ of.x - 1, of.y }, func)) return ret;
			if (auto ret = Apply<ONLY_VALID>({ of.x + 1, of.y }, func)) return ret;
			if (auto ret = Apply<ONLY_VALID>({ of.x, of.y - 1 }, func)) return ret;
			if (auto ret = Apply<ONLY_VALID>({ of.x, of.y + 1 }, func)) return ret;

			if constexpr (ghassanpl::is_flag_set(FLAGS, IterationFlags::Diagonals))
			{
				if (auto ret = Apply<ONLY_VALID>({ of.x - 1, of.y - 1 }, func)) return ret;
				if (auto ret = Apply<ONLY_VALID>({ of.x + 1, of.y + 1 }, func)) return ret;
				if (auto ret = Apply<ONLY_VALID>({ of.x + 1, of.y - 1 }, func)) return ret;
				if (auto ret = Apply<ONLY_VALID>({ of.x - 1, of.y + 1 }, func)) return ret;
			}
			return return_type{};
		}
	}

	template<typename TILE_DATA>
	template<uint64_t FLAGS, typename FUNC>
	inline auto Grid<TILE_DATA>::ForEachSelectedNeighbor(ivec2 of, DirectionBitmap neighbor_bitmap, FUNC&& func) const
	{
		using return_type = decltype(func(ivec2{}));
		static_assert(std::is_void_v<return_type> || std::is_convertible_v<return_type, bool>, "return type of tile callback must be either void or convertible to bool");
		static constexpr auto ONLY_VALID = ghassanpl::is_flag_set(FLAGS, IterationFlags::OnlyValid);
		
		if constexpr (std::is_void_v<return_type>)
		{
			if constexpr (ghassanpl::is_flag_set(FLAGS, IterationFlags::WithSelf))
				Apply<ONLY_VALID>(of, func);

			neighbor_bitmap.for_each([this, of, &func](Direction d) {
				Apply<ONLY_VALID>(of + ToVector(d), func);
			});
		}
		else
		{
			if constexpr (ghassanpl::is_flag_set(FLAGS, IterationFlags::WithSelf))
				if (auto ret = Apply<ONLY_VALID>(of, func)) return ret;

			return neighbor_bitmap.for_each([this, of, &func](Direction d) {
				return Apply<ONLY_VALID>(of + ToVector(d), func);
			});
		}
	}

	template<typename TILE_DATA>
	template<uint64_t FLAGS, typename FUNC>
	inline auto Grid<TILE_DATA>::ForEachInRect(irec2 const& tile_rect, FUNC&& func) const
	{
		using return_type = decltype(func(ivec2{}));
		static_assert(std::is_void_v<return_type> || std::is_convertible_v<return_type, bool>, "return type of tile callback must be either void or convertible to bool");
		static constexpr auto ONLY_VALID = ghassanpl::is_flag_set(FLAGS, IterationFlags::OnlyValid);
		
		if constexpr (std::is_void_v<return_type>)
		{
			for (int y = tile_rect.top(); y < tile_rect.bottom(); y++)
				for (int x = tile_rect.left(); x < tile_rect.right(); x++)
					Apply<ONLY_VALID>({ x, y }, func);
		}
		else
		{
			for (int y = tile_rect.top(); y < tile_rect.bottom(); y++)
				for (int x = tile_rect.left(); x < tile_rect.right(); x++)
					if (auto ret = Apply<ONLY_VALID>({ x, y }, func)) return ret;

			return return_type{};
		}
	}

	template<typename TILE_DATA>
	template<uint64_t FLAGS, typename FUNC>
	auto Grid<TILE_DATA>::ForEachInPerimeter(irec2 const& tile_rect, FUNC&& func) const
	{
		using return_type = decltype(func(ivec2{}));
		static_assert(std::is_void_v<return_type> || std::is_convertible_v<return_type, bool>, "return type of tile callback must be either void or convertible to bool");
		static constexpr auto ONLY_VALID = ghassanpl::is_flag_set(FLAGS, IterationFlags::OnlyValid);

		if constexpr (std::is_void_v<return_type>)
		{
			for (int x = tile_rect.left(); x < tile_rect.right(); x++)
			{
				Apply<ONLY_VALID>({ x, tile_rect.top() }, func);
				Apply<ONLY_VALID>({ x, tile_rect.bottom() - 1 }, func);
			}
			for (int y = tile_rect.top() + 1; y < tile_rect.bottom() - 1; y++)
			{
				Apply<ONLY_VALID>({ tile_rect.left(), y }, func);
				Apply<ONLY_VALID>({ tile_rect.right() - 1, y }, func);
			}
		}
		else
		{
			for (int x = tile_rect.left(); x < tile_rect.right(); x++)
			{
				if (auto ret = Apply<ONLY_VALID>({ x, tile_rect.top() }, func)) return ret;
				if (auto ret = Apply<ONLY_VALID>({ x, tile_rect.bottom() - 1 }, func)) return ret;
			}
			for (int y = tile_rect.top() + 1; y < tile_rect.bottom() - 1; y++)
			{
				if (auto ret = Apply<ONLY_VALID>({ tile_rect.left(), y }, func)) return ret;
				if (auto ret = Apply<ONLY_VALID>({ tile_rect.right() - 1, y }, func)) return ret;
			}

			return return_type{};
		}
	}

	template<typename TILE_DATA>
	template<uint64_t FLAGS, typename TILE_SET, typename FUNC>
	auto Grid<TILE_DATA>::ForEachInSet(TILE_SET&& tiles, FUNC&& func) const
	{
		using return_type = decltype(func(ivec2{}));
		static_assert(std::is_void_v<return_type> || std::is_convertible_v<return_type, bool>, "return type of tile callback must be either void or convertible to bool");
		static constexpr auto ONLY_VALID = ghassanpl::is_flag_set(FLAGS, IterationFlags::OnlyValid);
		
		if constexpr (std::is_void_v<return_type>)
		{
			for (auto&& tile : tiles)
				Apply<ONLY_VALID>(tile, func);
		}
		else
		{
			for (auto&& tile : tiles)
				if (auto ret = Apply<ONLY_VALID>(tile, func)) return ret;
			return return_type{};
		}
	}

	template<typename TILE_DATA>
	template<uint64_t FLAGS, typename FUNC>
	auto Grid<TILE_DATA>::ForEach(FUNC&& func) const
	{
		//static constexpr auto ONLY_VALID = ghassanpl::is_flag_set(FLAGS, IterationFlags::OnlyValid);
		irec2 rect = { 0, 0, (int)mWidth, (int)mHeight };
		return ForEachInRect<FLAGS>(rect, std::forward<FUNC>(func));
	}

	std::set<ivec2> RasterizePolygon(std::span<vec2> points, vec2 _tile_size);

	template<typename TILE_DATA>
	template<uint64_t FLAGS, typename FUNC>
	auto Grid<TILE_DATA>::ForEachInPolygon(std::span<vec2> poly_points, vec2 tile_size, FUNC&& func) const
	{
		return ForEachInSet(RasterizePolygon(poly_points, tile_size), std::forward<FUNC>(func));
	}

	template<typename TILE_DATA>
	template<typename FUNC>
	inline bool Grid<TILE_DATA>::LineCast(ivec2 start, ivec2 end, FUNC&& blocks_func, bool ignore_start) const
	{
		int delta_x{ end.x - start.x };
		// if x1 == x2, then it does not matter what we set here
		signed char const ix((delta_x > 0) - (delta_x < 0));
		delta_x = std::abs(delta_x) << 1;

		int delta_y(end.y - start.y);
		// if y1 == y2, then it does not matter what we set here
		signed char const iy((delta_y > 0) - (delta_y < 0));
		delta_y = std::abs(delta_y) << 1;

		if (!ignore_start && blocks_func(start))
			return false;

		if (delta_x >= delta_y)
		{
			// error may go below zero
			int error(delta_y - (delta_x >> 1));

			while (start.x != end.x)
			{
				// reduce error, while taking into account the corner case of error == 0
				if ((error > 0) || (!error && (ix > 0)))
				{
					error -= delta_x;
					start.y += iy;
				}
				// else do nothing

				error += delta_y;
				start.x += ix;

				if (blocks_func(start))
					return false;
			}
		}
		else
		{
			// error may go below zero
			int error(delta_x - (delta_y >> 1));

			while (start.y != end.y)
			{
				// reduce error, while taking into account the corner case of error == 0
				if ((error > 0) || (!error && (iy > 0)))
				{
					error -= delta_y;
					start.x += ix;
				}
				// else do nothing

				error += delta_x;
				start.y += iy;

				if (blocks_func(start))
					return false;
			}
		}

		return true;
	}

	template<typename TILE_DATA>
	template<bool ONLY_VALID, typename FUNC>
	auto Grid<TILE_DATA>::Apply(ivec2 to, FUNC&& func) const
	{
		using return_type = decltype(func(ivec2{}));
		if constexpr (std::is_void_v<return_type>)
		{
			if constexpr (ONLY_VALID) if (!IsValid(to)) return;
			func(to);
		}
		else
		{
			if constexpr (ONLY_VALID) if (!IsValid(to)) return return_type{};
			return func(to);
		}
	}

	template<typename TILE_DATA>
	template<typename SHOULD_FLOOD_FUNC, typename FLOOD_FUNC>
	void Grid<TILE_DATA>::Flood(ivec2 start, SHOULD_FLOOD_FUNC&& should_flood, FLOOD_FUNC&& flood)
	{
		std::queue<ivec2> queue;
		if (!IsValid(start)) return;
		if (!should_flood(start, *At(start))) return;

		queue.push(start);
		while (!queue.empty())
		{
			auto n = queue.front();
			queue.pop();

			auto l = n, r = ivec2{ n.x + 1, n.y };

			while (IsValid(l) && should_flood(l, *At(l)))
				l.x--;
			l.x++;

			while (IsValid(r) && should_flood(r, *At(r)))
				r.x++;
			r.x--;

			for (int x = l.x; x <= r.x; x++)
			{
				ivec2 pos{ x, n.y };
				flood(pos, *At(pos));

				ivec2 up = { pos.x, pos.y - 1 }, down = { pos.x, pos.y + 1 };

				if (IsValid(up) && should_flood(up, *At(up)))
					queue.push(up);

				if (IsValid(down) && should_flood(down, *At(down)))
					queue.push(down);
			}
		}
	}

	template<typename TILE_DATA>
	void Grid<TILE_DATA>::Reset(int w, int h, TILE_DATA const& default_tile)
	{
		mTiles.clear();
		mWidth = w;
		mHeight = h;
		mTiles.resize(w * h, default_tile);
	}

	template<typename TILE_DATA>
	void Grid<TILE_DATA>::Reset(int w, int h)
	{
		mTiles.clear();
		mWidth = w;
		mHeight = h;
		mTiles.resize(w * h);
	}

	template<typename TILE_DATA>
	TILE_DATA const* Grid<TILE_DATA>::At(ivec2 pos) const noexcept
	{
		if (!IsValid(pos))
			return nullptr;
		return &mTiles[pos.x + pos.y * mWidth];
	}

	template<typename TILE_DATA>
	TILE_DATA* Grid<TILE_DATA>::At(ivec2 pos) noexcept
	{
		if (!IsValid(pos))
			return nullptr;
		return &mTiles[pos.x + pos.y * mWidth];
	}

	template<typename TILE_DATA>
	TILE_DATA const* Grid<TILE_DATA>::AtIndex(int index) const noexcept { return IsIndexValid(index) ? &mTiles[index] : nullptr; }

	template<typename TILE_DATA>
	TILE_DATA* Grid<TILE_DATA>::AtIndex(int index) noexcept { return IsIndexValid(index) ? &mTiles[index] : nullptr; }

	template<typename TILE_DATA>
	void Grid<TILE_DATA>::Resize(uvec2 new_size, const TILE_DATA& new_element)
	{
		ResizeY(new_size.y, new_element);
		ResizeX(new_size.x, new_element);
	}

	template<typename TILE_DATA>
	void Grid<TILE_DATA>::FlipHorizontal()
	{
		for (int i = 0; i < mHeight; i++)
			std::reverse(GetRowStart(i), GetRowStart(i) + mWidth);
	}

	template<typename TILE_DATA>
	void Grid<TILE_DATA>::FlipVertical()
	{
		for (int i = 0; i < mHeight / 2; i++)
			std::swap_ranges(GetRowStart(i), GetRowStart(i) + mWidth, GetRowStart(mHeight - i - 1));
	}

	template<typename TILE_DATA>
	void Grid<TILE_DATA>::Rotate180()
	{
		for (int i = 0; i < mHeight / 2; i++)
			std::swap_ranges(std::make_reverse_iterator(GetRowStart(i) + mWidth), std::make_reverse_iterator(GetRowStart(i)), GetRowStart(mHeight - i - 1));

		/// Need to reverse middle row if height is odd
		if (mHeight % 1)
			std::reverse(GetRowStart(mHeight / 2 + 1), GetRowStart(mHeight / 2 + 1) + mWidth);
	}

	template<typename TILE_DATA>
	void Grid<TILE_DATA>::ResizeY(int new_y, const TILE_DATA& new_element)
	{
		if (new_y <= 0) throw std::invalid_argument("new_y");

		const auto new_count = new_y * mWidth;
		mTiles.resize(new_count, new_element);
		mHeight = new_y;
	}

	template<typename TILE_DATA>
	void Grid<TILE_DATA>::ResizeX(int new_x, const TILE_DATA& new_element)
	{
		/// TODO: Would it be more cache-friendly to copy to a new vector and swap it with mTiles?
		/*
		std::vector<T> new_vector;
		new_vector.reserve(new_size);
		for (size_t y=0; y<mHeight; ++y)
		{
			for (size_t x=0; x<mSize.x; ++x)
			{
				new_vector.push_back(std::move(mTiles[y * mSize.x + x]));
			}
			for (size_t x=mSize.x; x<new_x; ++x)
			{
				new_vector.push_back(new_element);
			}
		}
		mTiles = std::move(new_vector);
		*/
		if (new_x <= 0) throw std::invalid_argument("new_x");

		const auto new_count = new_x * mHeight;

		if (new_x > mWidth)
		{
			mTiles.resize(new_count, new_element);

			for (int yy = 0; yy < mHeight; ++yy)
			{
				auto y = mHeight - yy - 1;
				const auto begin_range = y * mWidth;
				const auto end_range = begin_range + mWidth;
				const auto new_end_range = y * new_x + mWidth;
				std::swap_ranges(std::make_reverse_iterator(mTiles.begin() + end_range), std::make_reverse_iterator(mTiles.begin() + begin_range), std::make_reverse_iterator(mTiles.begin() + new_end_range));
			}
		}
		else
		{
			const auto dif = mWidth - new_x;
			for (int y = 1; y < mHeight; ++y)
			{
				const auto begin_range = y * mWidth;
				const auto end_range = begin_range + new_x;
				std::move(mTiles.begin() + begin_range, mTiles.begin() + end_range, mTiles.begin() + begin_range - (dif * y));
			}
			mTiles.resize(new_count);
		}

		mWidth = new_x;
	}

}