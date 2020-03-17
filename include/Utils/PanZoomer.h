#pragma once

#include "../Input/InputSystem.h"
#include "../Camera.h"

namespace gamelib
{
	struct PanZoomer
	{
		IInputSystem& Input;
		ICamera& Camera;
		InputID GrabInput = InvalidInput;
		InputID ZoomInput = InvalidInput;
		InputID PositionInput = InvalidInput;

		float MinThrowSpeed = 0;
		double ZoomSpeed = 100;

		bool Grabbed() const { return mGrabbed; }

		PanZoomer(IInputSystem& input, ICamera& camera, InputID grab_input, InputID zoom_input, InputID position)
			: Input(input), Camera(camera), GrabInput(grab_input), ZoomInput(zoom_input), PositionInput(position)
		{

		}
		PanZoomer(IInputSystem& input, ICamera& camera)
			: Input(input), Camera(camera)
		{

		}

		bool Update(seconds_t time)
		{
			const auto pos = GetMouseScreenPos();

			if (mGrabbed)
			{
				if (IsGrabPressed())
				{
					/// move
					const auto diff = screen_pos_t{ pos.Value - mLastMousePos.Value };
					mLastMousePos = pos;
					Camera.Pan(diff);
					if (OnDrag) OnDrag(diff);
				}
				else
				{
					/// release
					mGrabbed = false;
					if (OnRelease) OnRelease(pos);

					const auto vel = screen_pos_t{ (pos.Value - mLastMousePos.Value) / (float)time };
					if (vel->length() > MinThrowSpeed && OnThrow)
						OnThrow(vel);
				}
			}
			else
			{
				/// grab
				if (WasGrabPressed())
				{
					if (Camera.InViewport(pos.Value))
					{
						mGrabbed = true;
						mLastMousePos = pos;
						if (OnGrab) OnGrab(pos);
					}
				}
			}

			if (Camera.InViewport(pos.Value))
			{
				auto zoom = GetZoomInput();
				if (zoom != 0)
				{
					Camera.ScreenZoom(pos.Value, -zoom * ZoomSpeed);
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
				return screen_pos_t{ Input.GetMouse()->GetInputState(Input.GetMouse()->GetXAxisInput()), Input.GetMouse()->GetInputState(Input.GetMouse()->GetYAxisInput()) };
			else
				return screen_pos_t{ Input.GetAxis2D(PositionInput) };
		}

		double GetZoomInput() const
		{
			if (ZoomInput == InvalidInput)
				return Input.GetMouse()->GetInputState(Input.GetMouse()->GetVerticalWheelInput());
			else
				return Input.GetAxis(ZoomInput);
		}

		bool WasGrabPressed() const
		{
			if (GrabInput == InvalidInput)
				return Input.WasButtonPressed(MouseButton::Middle);
			else
				return Input.WasButtonPressed(GrabInput);
		}

		bool IsGrabPressed() const
		{
			if (GrabInput == InvalidInput)
				return Input.IsButtonPressed(MouseButton::Middle);
			else
				return Input.IsButtonPressed(GrabInput);
		}
	};
}