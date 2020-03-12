#include "Camera.h"
#include "Includes/Format.h"

namespace gamelib
{

	ICamera::ICamera(ALLEGRO_DISPLAY* display)
	{
		SetFromDisplay(display);
	}
	
	void ICamera::SetFromDisplay(ALLEGRO_DISPLAY* display)
	{
		auto size = irec2::from_size(0, 0, al_get_display_width(display), al_get_display_height(display));
		SetWorldBounds(size, radians_t{ 0.0f });
		SetViewport(size);
	}

	ICamera::ICamera(screen_bounds_t const& sbounds, world_bounds_t const& wbounds, radians_t rotation) noexcept
	{
		SetWorldBounds(wbounds.get(), rotation);
		SetViewport(sbounds.get());
	}

	vec2 ICamera::ScreenSpaceToCameraSpace(vec2 screen_point) const
	{
		return GetViewport().to_rect_space(screen_point);
	}

	vec2 ICamera::ScreenSpaceToWorldSpace(vec2 screen_point) const
	{
		al_transform_coordinates(&GetInverseTransform(), &screen_point.x, &screen_point.y);
		return screen_point;
	}

	vec2 ICamera::CameraSpaceToWorldSpace(vec2 camera_point) const
	{
		return CameraSpaceToScreenSpace(ScreenSpaceToWorldSpace(camera_point));
	}

	vec2 ICamera::WorldSpaceToCameraSpace(vec2 world_point) const
	{
		return ScreenSpaceToCameraSpace(WorldSpaceToScreenSpace(world_point));
	}

	vec2 ICamera::WorldSpaceToScreenSpace(vec2 world_point) const
	{
		al_transform_coordinates(&GetTransform(), &world_point.x, &world_point.y);
		return world_point;
	}

	vec2 ICamera::CameraSpaceToScreenSpace(vec2 camera_point) const
	{
		return GetViewport().to_world_space(camera_point);
	}

	rec2 ICamera::GetWorldBounds() const
	{
		auto p1 = CameraSpaceToWorldSpace({ 0, 0 });
		auto p4 = CameraSpaceToWorldSpace({ 1, 1 });
		if (GetRotation().Value == 0)
			return rec2{ p1.x, p1.y, p4.x, p4.y };

		auto p2 = CameraSpaceToWorldSpace({ 0, 1 });
		auto p3 = CameraSpaceToWorldSpace({ 1, 0 });

		auto r = std::max(std::max(std::max(p1.x, p2.x), p3.x), p4.x);
		auto l = std::min(std::min(std::min(p1.x, p2.x), p3.x), p4.x);
		auto t = std::min(std::min(std::min(p1.y, p2.y), p3.y), p4.y);
		auto b = std::max(std::max(std::max(p1.y, p2.y), p3.y), p4.y);

		return rec2{ l, t, r, b };
	}

	void ICamera::SetWorldBounds(rec2 const& rect, radians_t rotation)
	{
		mWorldRect = rect;
		UpdatePositions();
		mTransformable.SetRotation(rotation);
	}

	void ICamera::SetWorldSize(vec2 size)
	{
		mWorldRect.set_size(size);
		UpdatePositions();
	}

	void ICamera::SetWorldCenter(vec2 pos)
	{
		mWorldRect.set_center(pos);
		UpdatePositions();
	}

	std::array<vec2, 4> ICamera::GetWorldPolygon() const
	{
		return { CameraSpaceToWorldSpace({ 0, 0 }), CameraSpaceToWorldSpace({ 0, 1 }) , CameraSpaceToWorldSpace({ 1, 1 }), CameraSpaceToWorldSpace({ 1, 0 }) };
	}

	void ICamera::UpdatePositions()
	{
		mTransformable.SetOrigin(mWorldRect.center());
		mTransformable.SetPosition(-mWorldRect.position() + vec2(mViewport.position()));
		mTransformable.SetScale(vec2{ mViewport.size() } / mWorldRect.size());
	}

	bool ICamera::InViewport(ivec2 pos) const
	{
		return GetViewport().contains(pos);
	}

	bool ICamera::InViewport(screen_pos_t pos) const
	{
		return GetViewport().contains(pos.Value);
	}

	irec2 ICamera::GetViewport() const
	{
		return mViewport;
	}

	void ICamera::SetViewport(irec2 const& viewport)
	{
		mViewport = viewport;
		UpdatePositions();
	}

	void ICamera::Pan(world_pos_t by)
	{
		mWorldRect.set_position(mWorldRect.position() - by.Value);
		UpdatePositions();
	}

	void ICamera::Pan(screen_pos_t by)
	{
		auto zero = ScreenSpaceToWorldSpace({});
		auto offs = ScreenSpaceToWorldSpace(by.Value);
		Pan(world_pos_t{ offs - zero });
	}

	void ICamera::ScreenZoom(vec2 screen_anchor, double zoom_by)
	{
		/// Inefficient but meh
		auto world_anchor = ScreenSpaceToWorldSpace(screen_anchor);

		auto aspect = mWorldRect.size();
		aspect /= aspect.x;
		mWorldRect.grow(float(zoom_by) * aspect);

		/// FUCK IT FFFFFFFFUUUUUUUUUUUUUUUUUCK
		mWorldRect.set_size(glm::max(mWorldRect.size(), vec2{ mViewport.size() }));

		UpdatePositions();
		auto new_world_anchor = ScreenSpaceToWorldSpace(screen_anchor);
		SetWorldCenter(GetWorldCenter() + (world_anchor - new_world_anchor));
	}
}