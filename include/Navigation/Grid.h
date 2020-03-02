#pragma once

#include "Squares.h"
#include <gsl/span>
#include <vector>

namespace gamelib::squares
{

	template <typename TILE_DATA>
	struct Grid
	{
		Grid() = default;
		Grid(size_t w, size_t h, TILE_DATA const& default_tile) { Reset(w, h, default_tile); }
		Grid(uvec2 size, TILE_DATA const& default_tile) : Grid(size.x, size.y, default_tile) {}
		Grid(size_t w, size_t h) { Reset(w, h); }
		Grid(uvec2 size) : Grid(size.x, size.y) {}

		void Reset(size_t w, size_t h, TILE_DATA const& default_tile);
		void Reset(size_t w, size_t h);
		void Reset(uvec2 size) { Reset(size.x, size.y); }

		/// Accessors & Queries

		TILE_DATA& operator[](size_t i) { return mTiles[i]; }
		TILE_DATA const& operator[](size_t i) const { return mTiles[i]; }

		/// https://github.com/nothings/stb/blob/master/stb_connected_components.h
		/// Need to determine whether tile is passable or not (maybe enum_flags::is_set(Definition->Flags, TileFlag::IsPassable) ?)
		bool IsReachable(ivec2 src_tile, ivec2 dest_tile);

		enum class IterationFlags
		{
			WithSelf,
			OnlyValid,
			Diagonals
		};

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::WithSelf, IterationFlags::OnlyValid), typename FUNC >
		auto ForEachNeighbor(ivec2 of, FUNC&& func) const;

		//template <typename FUNC>
		//auto ForEachSurrounding(ivec2 of, FUNC&& func, bool with_self = true, bool only_valid = true) const;

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::WithSelf, IterationFlags::OnlyValid), typename FUNC>
		auto ForEachSelectedNeighbor(ivec2 of, uint8_t neighbor_bitmap, FUNC&& func) const;

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid), typename FUNC>
		auto ForEachInRect(irec2 const& tile_rect, FUNC&& functrue) const;

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid), typename FUNC>
		auto ForEachInPerimeter(irec2 const& tile_rect, FUNC&& func) const;

		template<uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid), typename TILE_SET, typename FUNC>
		auto ForEachInSet(TILE_SET&& tiles, FUNC&& func) const;

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid), typename FUNC>
		auto ForEach(FUNC&& func) const;

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid), typename FUNC>
		auto ForEachInPolygon(gsl::span<vec2> poly_points, vec2 tile_size, FUNC&& func) const;

		/// Function: LineCast
		/// Return: Whether the line between `start` and `end` is free of blocing tiles, as determined by `blocks_func`
		template <typename FUNC>
		bool LineCast(ivec2 start, ivec2 end, FUNC&& blocks_func, bool ignore_start) const;

		bool IsValid(int x, int y) const noexcept { return x >= 0 && y >= 0 && (size_t)x < mWidth && (size_t)y < mHeight; }
		bool IsValid(ivec2 pos) const noexcept { return IsValid(pos.x, pos.y); }
		bool IsIndexValid(int index) const noexcept { return index >= 0 && index < (int)mTiles.size(); }

		TILE_DATA const* At(ivec2 pos) const noexcept;
		TILE_DATA const* At(int x, int y) const noexcept { return At(ivec2{ x, y }); }
		TILE_DATA* At(ivec2 pos) noexcept;
		TILE_DATA* At(int x, int y) noexcept { return At(ivec2{ x, y }); }
		TILE_DATA const* AtIndex(int index) const noexcept;
		TILE_DATA* AtIndex(int index) noexcept;

		TILE_DATA const& SafeAt(ivec2 pos, TILE_DATA const& outside) const noexcept { if (auto at = At(pos)) return at->Data; return outside; }
		TILE_DATA const& SafeAt(int x, int y, TILE_DATA const& outside) const noexcept { return At(ivec2{ x, y }, outside); }
		TILE_DATA& SafeAt(ivec2 pos, TILE_DATA const& outside) noexcept { if (auto at = At(pos)) return at->Data; return outside; }
		TILE_DATA& SafeAt(int x, int y, TILE_DATA const& outside) noexcept { return At(ivec2{ x, y }); }

		size_t Width() const noexcept { return mWidth; }
		size_t Height() const noexcept { return mHeight; }
		uvec2 Size() const noexcept { return { unsigned(mWidth), unsigned(mHeight) }; }
		gsl::span<TILE_DATA const> Tiles() const { return mTiles; }

		/// Searches

		//using PredecessorMap = std::unordered_map<ivec2, ivec2, ivec_hash>;
		//template <typename COST>
		//using CostMap = std::unordered_map<ivec2, COST, ivec_hash>;

		/// Modifiers

		template <bool ONLY_VALID = true, typename FUNC>
		auto Apply(ivec2 to, FUNC&& func) const;

		void Resize(uvec2 new_size, const TILE_DATA& new_element);

		template <typename SHOULD_FLOOD_FUNC /* void(Position, const T&) */, typename FLOOD_FUNC /* void(Position, T&) */>
		void Flood(ivec2 start, SHOULD_FLOOD_FUNC&& should_flood, FLOOD_FUNC&& flood);

		void FlipHorizontal();
		void FlipVertical();
		void Rotate180();

	protected:

		auto GetRowStart(size_t row) { return mTiles.begin() + row * mWidth; }
		auto GetTileIterator(int x, int y) { return mTiles.begin() + y * mWidth + x; }

		void ResizeY(size_t new_y, const TILE_DATA& new_element);

		void ResizeX(size_t new_x, const TILE_DATA& new_element);

		size_t mWidth = 0;
		size_t mHeight = 0;
		std::vector<TILE_DATA> mTiles;
	};

}
#include "Grid.impl.h"