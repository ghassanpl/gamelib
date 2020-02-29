#include "Transformable.h"

namespace gamelib
{

	auto Transformable::GetTransform() const -> ALLEGRO_TRANSFORM const&
	{
		UpdateTransform();
		return mTransform;
	}

	auto Transformable::GetInverseTransform() const -> ALLEGRO_TRANSFORM const&
	{
		UpdateTransform();
		if (!mInverseTransformUpdated)
		{
			al_copy_transform(&mInverseTransform, &mTransform);
			al_invert_transform(&mInverseTransform);
			mInverseTransformUpdated = true;
		}
		return mInverseTransform;
	}

	void Transformable::UpdateTransform() const
	{
		if (!mTransformUpdated)
		{
			al_identity_transform(&mTransform);
			al_translate_transform(&mTransform, -mOrigin.x, -mOrigin.y);
			al_scale_transform(&mTransform, mScale.x, mScale.y);
			al_rotate_transform(&mTransform, mRotation);
			al_translate_transform(&mTransform, mOrigin.x + mPosition.x, mOrigin.y + mPosition.y);
			mTransformUpdated = true;
			mInverseTransformUpdated = false;
		}
	}

}