#pragma once

#include "Common.h"
#include "Includes/Allegro.h"
#include "Includes/EnumFlags.h"
#include "Transformable.h"

namespace gamelib
{

	enum class DrawLayer : uint64_t
	{
		First = 0,
		Last = 63
	};

	struct ALLEGRO_BITMAP;

	using screen_pos_t = named<vec2, struct ScreenSpace>;
	using camera_pos_t = named<vec2, struct CameraSpace>;
	using world_pos_t = named<vec2, struct WorldSpace>;

	using screen_bounds_t = named<rec2, struct ScreenSpace>;
	using camera_bounds_t = named<rec2, struct CameraSpace>;
	using world_bounds_t = named<rec2, struct WorldSpace>;

	struct ICamera
	{
		ICamera() noexcept = default;
		ICamera(ALLEGRO_DISPLAY* display) noexcept;
		ICamera(screen_bounds_t const& sbounds, world_bounds_t const& wbounds, double rotation = 0) noexcept;

		ghassanpl::enum_flags<DrawLayer> DrawLayers = ghassanpl::enum_flags<DrawLayer>::all();

		bool Enabled = true;

		std::string Name = "Main";

		ALLEGRO_BITMAP* Target = nullptr;

		/// TODO:
		//bool MouseInView() const;

		//vec2 LocalMousePosition() const; /// <- if MouseInView(), this is the camera space position of the mouse point over this camera
		//vec2 WorldMousePosition() const; /// <- if MouseInView(), this is the world position of the mouse point through this camera

		/// TODO: Are both these versions neccessary?

		vec2 ScreenSpaceToCameraSpace(vec2 screen_point) const;
		vec2 ScreenSpaceToWorldSpace(vec2 screen_point) const;

		vec2 CameraSpaceToWorldSpace(vec2 camera_pos) const;
		vec2 WorldSpaceToCameraSpace(vec2 camera_pos) const;

		vec2 WorldSpaceToScreenSpace(vec2 camera_pos) const;
		vec2 CameraSpaceToScreenSpace(vec2 camera_pos) const;

		screen_pos_t ToScreenSpace(camera_pos_t pos) const { return screen_pos_t{ CameraSpaceToScreenSpace(pos.get()) }; }
		screen_pos_t ToScreenSpace(world_pos_t pos) const { return screen_pos_t{ WorldSpaceToScreenSpace(pos.get()) }; }
		camera_pos_t ToCameraSpace(screen_pos_t pos) const { return camera_pos_t{ ScreenSpaceToCameraSpace(pos.get()) }; }
		camera_pos_t ToCameraSpace(world_pos_t pos) const { return camera_pos_t{ WorldSpaceToCameraSpace(pos.get()) }; }
		world_pos_t ToWorldSpace(camera_pos_t pos) const { return world_pos_t{ CameraSpaceToWorldSpace(pos.get()) }; }
		world_pos_t ToWorldSpace(screen_pos_t pos) const { return world_pos_t{ ScreenSpaceToWorldSpace(pos.get()) }; }

		rec2 GetWorldBounds() const;
		void SetWorldBounds(rec2 const& rect, float rotation);

		void SetWorldSize(vec2 size);
		void SetWorldCenter(vec2 pos);
		vec2 GetWorldCenter() const { return mWorldPosition - mWorldSize / 2.0f; }

		bool InViewport(ivec2 pos) const;
		bool InViewport(screen_pos_t pos) const;
		irec2 GetViewport() const;
		void SetViewport(irec2 const& rect);

		std::array<vec2, 4> GetWorldPolygon() const;

		auto GetTransform() const -> ALLEGRO_TRANSFORM const& { return mTransformable.GetTransform(); }
		auto GetInverseTransform() const -> ALLEGRO_TRANSFORM const& { return mTransformable.GetInverseTransform(); }

		auto GetPosition() const->vec2 { return mTransformable.GetPosition(); }
		auto SetPosition(vec2 pos) -> void { mTransformable.SetPosition(pos); }
		auto GetRotation() const -> double { return mTransformable.GetRotation(); }
		auto SetRotation(double rot) -> void { mTransformable.SetRotation(rot); }

		void ScreenZoom(vec2 screen_anchor, double zoom_by);
		void WorldZoom(vec2 world_anchor, double zoom_by);
		void Zoom(double zoom_by);
		void Pan(world_pos_t by);
		void Pan(screen_pos_t by);

	private:

		void UpdatePositions();

		vec2 mWorldPosition = {};
		vec2 mWorldSize = {};
		Transformable mTransformable;

		irec2 mViewport;
	};

}