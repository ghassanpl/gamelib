#pragma once

#include "Squares.h"
#include <vector>

namespace gamelib::squares
{
	/// TODO: template <typename TILE_DATA> struct SizedGrid : Grid<TileData> { private: vec2 mTileSize; };

	template <typename TILE_DATA>
	struct Grid
	{
		Grid() = default;
		Grid(int w, int h, TILE_DATA const& default_tile) { Reset(w, h, default_tile); }
		Grid(ivec2 size, TILE_DATA const& default_tile) : Grid(size.x, size.y, default_tile) {}
		Grid(int w, int h) { Reset(w, h); }
		Grid(ivec2 size) : Grid(size.x, size.y) {}

		void Reset(int w, int h, TILE_DATA const& default_tile);
		void Reset(int w, int h);
		void Reset(ivec2 size) { Reset(size.x, size.y); }
		void Reset(ivec2 size, TILE_DATA const& default_tile) { Reset(size.x, size.y, default_tile); }

		/// Accessors & Queries

		TILE_DATA& operator[](int i) { return mTiles[i]; }
		TILE_DATA const& operator[](int i) const { return mTiles[i]; }

		/// https://github.com/nothings/stb/blob/master/stb_connected_components.h
		/// Need to determine whether tile is passable or not (maybe enum_flags::is_set(Definition->Flags, TileFlag::IsPassable) ?)
		bool IsReachable(ivec2 src_tile, ivec2 dest_tile);

		enum class IterationFlags
		{
			WithSelf,
			OnlyValid,
			Diagonals
		};

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::WithSelf, IterationFlags::OnlyValid), typename FUNC>
		auto ForEachNeighbor(ivec2 of, FUNC&& func) const;

		/// TODO: Change `neighbor_bitmap` to enum_flags<Direction>
		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::WithSelf, IterationFlags::OnlyValid), typename FUNC>
		auto ForEachSelectedNeighbor(ivec2 of, DirectionBitmap neighbor_bitmap, FUNC&& func) const;

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid), typename FUNC>
		auto ForEachInRect(irec2 const& tile_rect, FUNC&& functrue) const;

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid), typename FUNC>
		auto ForEachInPerimeter(irec2 const& tile_rect, FUNC&& func) const;

		template<uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid), typename TILE_SET, typename FUNC>
		auto ForEachInSet(TILE_SET&& tiles, FUNC&& func) const;

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid), typename FUNC>
		auto ForEach(FUNC&& func) const;

		template <uint64_t FLAGS = ghassanpl::flag_bits(IterationFlags::OnlyValid), typename FUNC>
		auto ForEachInPolygon(std::span<vec2> poly_points, vec2 tile_size, FUNC&& func) const;

		/// Function: LineCast
		/// Return: Whether the line between `start` and `end` is free of blocing tiles, as determined by `blocks_func`
		template <typename FUNC>
		bool LineCast(ivec2 start, ivec2 end, FUNC&& blocks_func, bool ignore_start) const;

		bool IsValid(int x, int y) const noexcept { return x >= 0 && y >= 0 && x < mWidth && y < mHeight; }
		bool IsValid(vec2 world_pos, vec2 tile_size) const noexcept { return IsValid(WorldPositionToTilePosition(world_pos, tile_size)); }
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
		TILE_DATA& SafeAt(ivec2 pos, TILE_DATA& outside) noexcept { if (auto at = At(pos)) return at->Data; return outside; }
		TILE_DATA& SafeAt(int x, int y, TILE_DATA& outside) noexcept { return At(ivec2{ x, y }); }

		int Width() const noexcept { return mWidth; }
		int Height() const noexcept { return mHeight; }
		ivec2 Size() const noexcept { return { mWidth, mHeight }; }
		irec2 Perimeter() const noexcept { return irec2::from_size({}, Size()); }
		std::span<TILE_DATA const> Tiles() const { return mTiles; }

		vec2 TilePositionToWorldPosition(ivec2 tile_pos, vec2 tile_size) const { return vec2(tile_pos) * tile_size; }
		ivec2 WorldPositionToTilePosition(vec2 world_pos, vec2 tile_size) const { return ivec2(glm::floor(world_pos / tile_size)); }
		irec2 WorldRectToTileRect(rec2 world_rect, vec2 tile_size) const { return irec2{ glm::floor(world_rect.p1 / tile_size), glm::ceil(world_rect.p2 / tile_size) }; }
		rec2 RectForTile(ivec2 pos, vec2 tile_size) const { return rec2::from_size(TilePositionToWorldPosition(pos, tile_size), tile_size); }

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

		auto GetRowStart(int row) { return mTiles.begin() + row * mWidth; }
		auto GetTileIterator(int x, int y) { return mTiles.begin() + y * mWidth + x; }

		void ResizeY(int new_y, const TILE_DATA& new_element);

		void ResizeX(int new_x, const TILE_DATA& new_element);

		int mWidth = 0;
		int mHeight = 0;
		std::vector<TILE_DATA> mTiles;
	};

}
#include "Grid.impl.h"