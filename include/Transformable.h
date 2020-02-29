#pragma once

#include "Includes/Allegro.h"

namespace gamelib
{

	struct Transformable
	{

		auto GetPosition() const -> vec2 { return mPosition; }
		auto SetPosition(vec2 pos) -> void { mPosition = pos; mTransformUpdated = false; }
		auto GetOrigin() const -> vec2 { return mOrigin; }
		auto SetOrigin(vec2 pos) -> void { mOrigin = pos; mTransformUpdated = false; }
		auto GetScale() const -> vec2 { return mScale; }
		auto SetScale(vec2 pos) -> void { mScale = pos; mTransformUpdated = false; }
		auto GetRotation() const -> float { return mRotation; }
		auto SetRotation(float rot) -> void { mRotation = rot; mTransformUpdated = false; }

		auto GetTransform() const->ALLEGRO_TRANSFORM const&;
		auto GetInverseTransform() const->ALLEGRO_TRANSFORM const&;

	private:

		void UpdateTransform() const;

		vec2 mPosition = {};
		vec2 mOrigin = {};
		float mRotation = 0;
		vec2 mScale = { 1, 1 };
		mutable ALLEGRO_TRANSFORM mTransform = {};
		mutable ALLEGRO_TRANSFORM mInverseTransform = {};
		mutable bool mTransformUpdated = false;
		mutable bool mInverseTransformUpdated = false;

	};

}