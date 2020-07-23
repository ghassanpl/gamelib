#pragma once

#define ALLEGRO_UNSTABLE 1
#include <array>
#include "GLM.h"
#include <allegro5/transformations.h>

inline static constexpr ALLEGRO_TRANSFORM TransformIdentity = {
	{ 
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	}
};

inline vec2 TransformPoint(ALLEGRO_TRANSFORM const& transform, vec2 point)
{
	al_transform_coordinates(&transform, &point.x, &point.y);
	return point;
}

inline std::array<vec2, 4> TransformRect(ALLEGRO_TRANSFORM const& transform, rec2 rectangle)
{
	return {
		TransformPoint(transform, rectangle.left_top()),
		TransformPoint(transform, rectangle.left_bottom()),
		TransformPoint(transform, rectangle.right_top()),
		TransformPoint(transform, rectangle.right_bottom())
	};
}

inline rec2 TransformRectBB(ALLEGRO_TRANSFORM const& transform, rec2 rectangle)
{
	const auto points = TransformRect(transform, rectangle);

	// Compute the bounding rectangle of the transformed points
	float left = points[0].x;
	float top = points[0].y;
	float right = points[0].x;
	float bottom = points[0].y;
	for (int i = 1; i < 4; ++i)
	{
		if (points[i].x < left)   left = points[i].x;
		else if (points[i].x > right)  right = points[i].x;
		if (points[i].y < top)    top = points[i].y;
		else if (points[i].y > bottom) bottom = points[i].y;
	}

	return rec2(left, top, right - left, bottom - top);
}

static_assert(sizeof(Color) == sizeof(ALLEGRO_COLOR));
inline ALLEGRO_COLOR const& ToAllegro(Color const &c) { return *(ALLEGRO_COLOR*)(&c); }
inline ALLEGRO_COLOR ToAllegro(Color&& c) { return *(ALLEGRO_COLOR*)(&c); }

#define GAMELIB_ALLEGRO