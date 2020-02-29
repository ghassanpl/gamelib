#include "Camera.h"
#include "Includes/Format.h"

namespace gamelib
{

	ICamera::ICamera(ALLEGRO_DISPLAY* display) noexcept
	{
		auto size = irec2::from_size(0, 0, al_get_display_width(display), al_get_display_height(display));
		SetWorldBounds(size, 0);
		SetViewport(size);
	}

	ICamera::ICamera(screen_bounds_t const& sbounds, world_bounds_t const& wbounds, double rotation) noexcept
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
		return CameraSpaceToWorldSpace(ScreenSpaceToCameraSpace(screen_point));
	}

	vec2 ICamera::CameraSpaceToWorldSpace(vec2 camera_point) const
	{
		auto world_point = camera_point * mWorldSize + vec2(mViewport.position());
		al_transform_coordinates(&GetInverseTransform(), &world_point.x, &world_point.y);
		return world_point;
	}

	vec2 ICamera::WorldSpaceToCameraSpace(vec2 world_point) const
	{
		auto camera_point = (world_point) / mWorldSize;
		al_transform_coordinates(&GetTransform(), &camera_point.x, &camera_point.y);
		return camera_point - vec2(mViewport.position());
	}

	vec2 ICamera::WorldSpaceToScreenSpace(vec2 world_point) const
	{
		return CameraSpaceToScreenSpace(WorldSpaceToCameraSpace(world_point));
	}

	vec2 ICamera::CameraSpaceToScreenSpace(vec2 camera_point) const
	{
		return GetViewport().to_world_space(camera_point);
	}

	rec2 ICamera::GetWorldBounds() const
	{
		auto p1 = CameraSpaceToWorldSpace({ 0, 0 });
		auto p4 = CameraSpaceToWorldSpace({ 1, 1 });
		if (GetRotation() == 0)
			return rec2{ p1.x, p1.y, p4.x, p4.y };

		auto p2 = CameraSpaceToWorldSpace({ 0, 1 });
		auto p3 = CameraSpaceToWorldSpace({ 1, 0 });

		auto r = std::max(std::max(std::max(p1.x, p2.x), p3.x), p4.x);
		auto l = std::min(std::min(std::min(p1.x, p2.x), p3.x), p4.x);
		auto t = std::min(std::min(std::min(p1.y, p2.y), p3.y), p4.y);
		auto b = std::max(std::max(std::max(p1.y, p2.y), p3.y), p4.y);

		return rec2{ l, t, r, b };
	}

	void ICamera::SetWorldBounds(rec2 const& rect, float rotation)
	{
		mWorldPosition = rect.position();
		mWorldSize = rect.size();
		UpdatePositions();
		mTransformable.SetRotation(rotation);
	}

	void ICamera::SetWorldSize(vec2 size)
	{
		mWorldSize = size;
		UpdatePositions();
	}

	void ICamera::SetWorldCenter(vec2 pos)
	{
		mWorldPosition = pos - mWorldSize / 2.0f;
		UpdatePositions();
	}

	std::array<vec2, 4> ICamera::GetWorldPolygon() const
	{
		return { CameraSpaceToWorldSpace({ 0, 0 }), CameraSpaceToWorldSpace({ 0, 1 }) , CameraSpaceToWorldSpace({ 1, 1 }), CameraSpaceToWorldSpace({ 1, 0 }) };
	}

	void ICamera::UpdatePositions()
	{
		mTransformable.SetOrigin(mWorldPosition + mWorldSize / 2.0f);
		mTransformable.SetPosition(-mWorldPosition + vec2(mViewport.position()));
		mTransformable.SetScale(vec2{ mViewport.size() } / mWorldSize);
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
		SetPosition(GetPosition() + by.Value);
	}

	void ICamera::Pan(screen_pos_t by)
	{
		Pan(world_pos_t{ ToCameraSpace(by).Value * mWorldSize });
	}

	void ICamera::ScreenZoom(vec2 screen_anchor, double zoom_by)
	{
		auto factor = float(zoom_by / mWorldSize.x);
		//mWorldPosition += (GetWorldCenter() - ScreenSpaceToWorldSpace(screen_anchor)) * factor;
		auto resize = mWorldSize * factor;
		mWorldPosition -= resize / 2.0f;
		mWorldSize += resize;
		UpdatePositions();
		fmt::print("World Size: {}\n", GetWorldBounds());
	}
}