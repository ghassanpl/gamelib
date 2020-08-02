#pragma once

#include "../Input/InputSystem.h"
#include "../Camera.h"

namespace gamelib
{
	struct PanZoomer
	{
		std::shared_ptr<IInputSystem> Input;
		std::shared_ptr<ICamera> Camera;
		InputID GrabInput = InvalidInput;
		InputID ZoomInput = InvalidInput;
		InputID PositionInput = InvalidInput;

		float MinThrowSpeed = 0;
		double ZoomSpeed = 100;

		bool Grabbed() const { return mGrabbed; }

		PanZoomer(std::shared_ptr<IInputSystem> input, std::shared_ptr<ICamera> camera, InputID grab_input, InputID zoom_input, InputID position)
			: Input(std::move(input)), Camera(std::move(camera)), GrabInput(grab_input), ZoomInput(zoom_input), PositionInput(position)
		{

		}
		PanZoomer(std::shared_ptr<IInputSystem> input, std::shared_ptr<ICamera> camera)
			: Input(std::move(input)), Camera(std::move(camera))
		{

		}

		bool Update(seconds_t time)
		{
			const auto mouse_pos = GetMouseScreenPos();

			if (mGrabbed)
			{
				if (IsGrabPressed())
				{
					/// move
					const auto diff = screen_pos_t{ mouse_pos.Value - mLastMousePos.Value };
					mLastMousePos = mouse_pos;
					Camera->Pan(diff);
					if (OnDrag) OnDrag(diff);
				}
				else
				{
					/// release
					mGrabbed = false;
					if (OnRelease) OnRelease(mouse_pos);

					const auto vel = screen_pos_t{ (mouse_pos.Value - mLastMousePos.Value) / (float)time };
					if (vel->length() > MinThrowSpeed && OnThrow)
						OnThrow(vel);
				}
			}
			else
			{
				/// grab
				if (WasGrabPressed())
				{
					if (Camera->ViewportContains(mouse_pos.Value))
					{
						mGrabbed = true;
						mLastMousePos = mouse_pos;
						if (OnGrab) OnGrab(mouse_pos);
					}
				}
			}

			if (Camera->ViewportContains(mouse_pos.Value))
			{
				auto zoom = GetZoomInput();
				if (zoom != 0)
				{
					Camera->ScreenZoom(mouse_pos.Value, -zoom * ZoomSpeed);
					return true;
				}
			}

			return mGrabbed;
		}

		std::function<void(screen_pos_t)> OnGrab;
		std::function<void(screen_pos_t)> OnRelease;
		std::function<void(screen_pos_t)> OnDrag;
		std::function<void(screen_pos_t)> OnThrow;

	private:

		bool mGrabbed = false;
		screen_pos_t mLastMousePos;

		screen_pos_t GetMouseScreenPos() const
		{
			if (PositionInput == InvalidInput)
				return screen_pos_t{ Input->Mouse()->InputValue(Input->Mouse()->XAxisInputID()), Input->Mouse()->InputValue(Input->Mouse()->YAxisInputID()) };
			else
				return screen_pos_t{ Input->Axis2DValue(PositionInput) };
		}

		double GetZoomInput() const
		{
			if (ZoomInput == InvalidInput)
				return Input->Mouse()->InputValue(Input->Mouse()->VerticalWheelInputID());
			else
				return Input->AxisValue(ZoomInput);
		}

		bool WasGrabPressed() const
		{
			if (GrabInput == InvalidInput)
				return Input->WasButtonPressed(MouseButton::Middle);
			else
				return Input->WasButtonPressed(GrabInput);
		}

		bool IsGrabPressed() const
		{
			if (GrabInput == InvalidInput)
				return Input->IsButtonPressed(MouseButton::Middle);
			else
				return Input->IsButtonPressed(GrabInput);
		}
	};
}