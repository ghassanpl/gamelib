#include "InputDevice.h"
#include "InputSystem.h"

#pragma warning(error: 4061)
#pragma warning(error: 4062)

namespace gamelib
{

	std::string_view IInputDevice::GetName() const { return GetStringProperty(StringProperty::Name); }

	OutputProperties IInputDevice::GetOutputProperties(DeviceOutputID input) const
	{
		ParentSystem.ErrorReporter.NewError("Input device has no output properties").Value("Device", GetName()).Perform();
		std::terminate();
	}

	bool IKeyboardDevice::IsValidNavigation(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept:
		case UINavigationInput::Cancel:
		case UINavigationInput::Left:
		case UINavigationInput::Right:
		case UINavigationInput::Up:
		case UINavigationInput::Down:
		case UINavigationInput::Home:
		case UINavigationInput::End:
		case UINavigationInput::PageUp:
		case UINavigationInput::PageDown:
			return true;
		case UINavigationInput::Back:
		case UINavigationInput::Forward:
		case UINavigationInput::Menu:
		case UINavigationInput::View:
		case UINavigationInput::PageLeft:
		case UINavigationInput::PageRight:
		case UINavigationInput::ScrollUp:
		case UINavigationInput::ScrollDown:
		case UINavigationInput::ScrollLeft:
		case UINavigationInput::ScrollRight:
			return false;
		}
		return false;
	}

	bool IKeyboardDevice::GetNavigation(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return GetInputStateDigital((DeviceInputID)KeyboardKey::Enter);
		case UINavigationInput::Cancel: return GetInputStateDigital((DeviceInputID)KeyboardKey::Escape);
		case UINavigationInput::Left: return GetInputStateDigital((DeviceInputID)KeyboardKey::Left);
		case UINavigationInput::Right: return GetInputStateDigital((DeviceInputID)KeyboardKey::Right);
		case UINavigationInput::Up: return GetInputStateDigital((DeviceInputID)KeyboardKey::Up);
		case UINavigationInput::Down: return GetInputStateDigital((DeviceInputID)KeyboardKey::Down);
		case UINavigationInput::Home: return GetInputStateDigital((DeviceInputID)KeyboardKey::Home);
		case UINavigationInput::End: return GetInputStateDigital((DeviceInputID)KeyboardKey::End);
		case UINavigationInput::PageUp: return GetInputStateDigital((DeviceInputID)KeyboardKey::PgUp);
		case UINavigationInput::PageDown: return GetInputStateDigital((DeviceInputID)KeyboardKey::PgDn);
		case UINavigationInput::Back:
		case UINavigationInput::Forward:
		case UINavigationInput::Menu:
		case UINavigationInput::View:
		case UINavigationInput::PageLeft:
		case UINavigationInput::PageRight:
		case UINavigationInput::ScrollUp:
		case UINavigationInput::ScrollDown:
		case UINavigationInput::ScrollLeft:
		case UINavigationInput::ScrollRight:
			return false;
		}
		return false;
	}

	bool IKeyboardDevice::GetNavigationLastFrame(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return GetInputStateLastFrameDigital((DeviceInputID)KeyboardKey::Enter);
		case UINavigationInput::Cancel: return GetInputStateLastFrameDigital((DeviceInputID)KeyboardKey::Escape);
		case UINavigationInput::Left: return GetInputStateLastFrameDigital((DeviceInputID)KeyboardKey::Left);
		case UINavigationInput::Right: return GetInputStateLastFrameDigital((DeviceInputID)KeyboardKey::Right);
		case UINavigationInput::Up: return GetInputStateLastFrameDigital((DeviceInputID)KeyboardKey::Up);
		case UINavigationInput::Down: return GetInputStateLastFrameDigital((DeviceInputID)KeyboardKey::Down);
		case UINavigationInput::Home: return GetInputStateLastFrameDigital((DeviceInputID)KeyboardKey::Home);
		case UINavigationInput::End: return GetInputStateLastFrameDigital((DeviceInputID)KeyboardKey::End);
		case UINavigationInput::PageUp: return GetInputStateLastFrameDigital((DeviceInputID)KeyboardKey::PgUp);
		case UINavigationInput::PageDown: return GetInputStateLastFrameDigital((DeviceInputID)KeyboardKey::PgDn);
		case UINavigationInput::Back:
		case UINavigationInput::Forward:
		case UINavigationInput::Menu:
		case UINavigationInput::View:
		case UINavigationInput::PageLeft:
		case UINavigationInput::PageRight:
		case UINavigationInput::ScrollUp:
		case UINavigationInput::ScrollDown:
		case UINavigationInput::ScrollLeft:
		case UINavigationInput::ScrollRight:
			return false;
		}
		return false;
	}

	bool IMouseDevice::IsValidNavigation(UINavigationInput input) const
	{
		switch (input)
		{
			/// SM_MOUSEHORIZONTALWHEELPRESENT
			/// SM_MOUSEWHEELPRESENT
			/// SM_CMOUSEBUTTONS
		case UINavigationInput::Accept:
		case UINavigationInput::Cancel:
			return true;
		case UINavigationInput::ScrollUp:
		case UINavigationInput::ScrollDown:
			return GetVerticalWheelInput() != InvalidDeviceInputID;
		case UINavigationInput::ScrollLeft:
		case UINavigationInput::ScrollRight:
			return GetHorizontalWheelInput() != InvalidDeviceInputID;

		case UINavigationInput::Left:
		case UINavigationInput::Right:
		case UINavigationInput::Up:
		case UINavigationInput::Down:
		case UINavigationInput::Home:
		case UINavigationInput::End:
		case UINavigationInput::Back:
		case UINavigationInput::Forward:
		case UINavigationInput::Menu:
		case UINavigationInput::View:
		case UINavigationInput::PageLeft:
		case UINavigationInput::PageRight:
		case UINavigationInput::PageUp:
		case UINavigationInput::PageDown:
			return false;
		}
		return false;
	}

	bool IMouseDevice::GetNavigation(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return GetInputStateDigital((DeviceInputID)MouseButton::Left);
		case UINavigationInput::Cancel: return GetInputStateDigital((DeviceInputID)MouseButton::Right);
		case UINavigationInput::ScrollUp: return GetInputState(GetVerticalWheelInput()) < 0;
		case UINavigationInput::ScrollDown: return GetInputState(GetVerticalWheelInput()) > 0;
		case UINavigationInput::ScrollLeft: return GetInputState(GetHorizontalWheelInput()) < 0;
		case UINavigationInput::ScrollRight: return GetInputState(GetHorizontalWheelInput()) > 0;

		case UINavigationInput::Left:
		case UINavigationInput::Right:
		case UINavigationInput::Up:
		case UINavigationInput::Down:
		case UINavigationInput::Home:
		case UINavigationInput::End:
		case UINavigationInput::Back:
		case UINavigationInput::Forward:
		case UINavigationInput::Menu:
		case UINavigationInput::View:
		case UINavigationInput::PageLeft:
		case UINavigationInput::PageRight:
		case UINavigationInput::PageUp:
		case UINavigationInput::PageDown:
			return false;
		}
		return false;
	}

	bool IMouseDevice::GetNavigationLastFrame(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return GetInputStateLastFrameDigital((DeviceInputID)MouseButton::Left);
		case UINavigationInput::Cancel: return GetInputStateLastFrameDigital((DeviceInputID)MouseButton::Right);
		case UINavigationInput::ScrollUp: return GetInputStateLastFrame(GetVerticalWheelInput()) < 0;
		case UINavigationInput::ScrollDown: return GetInputStateLastFrame(GetVerticalWheelInput()) > 0;
		case UINavigationInput::ScrollLeft: return GetInputStateLastFrame(GetHorizontalWheelInput()) < 0;
		case UINavigationInput::ScrollRight: return GetInputStateLastFrame(GetHorizontalWheelInput()) > 0;

		case UINavigationInput::Left:
		case UINavigationInput::Right:
		case UINavigationInput::Up:
		case UINavigationInput::Down:
		case UINavigationInput::Home:
		case UINavigationInput::End:
		case UINavigationInput::Back:
		case UINavigationInput::Forward:
		case UINavigationInput::Menu:
		case UINavigationInput::View:
		case UINavigationInput::PageLeft:
		case UINavigationInput::PageRight:
		case UINavigationInput::PageUp:
		case UINavigationInput::PageDown:
			return false;
		}
		return false;
	}

	DeviceInputID IXboxGamepadDevice::GetMaxInput() const
	{
		return 20;
	}

	struct XboxButtonInputProperties : InputProperties
	{
		XboxButtonInputProperties(std::string_view name)
		{
			Name = name;
			Flags.set(InputFlags::Digital);
			DeadZoneMin = DeadZoneMax = MinValue = 0;
			StepSize = 1;
		}
	};

	struct XboxStickAxisInputProperties : InputProperties
	{
		XboxStickAxisInputProperties(std::string_view name, double min = -1.0, double max = 1.0)
		{
			Name = name;
			Flags.unset(InputFlags::Digital);
			Flags.set(InputFlags::ReturnsToNeutral, InputFlags::HasDeadzone);
			MinValue = min;
			MaxValue = max;
		}
	};

	InputProperties IXboxGamepadDevice::GetInputProperties(DeviceInputID input) const
	{
		if (!IsValidInput(input)) return {};
		if (input < ButtonCount)
		{
			switch ((XboxGamepadButton)input)
			{
			case XboxGamepadButton::A: return XboxButtonInputProperties("A");
			case XboxGamepadButton::B: return XboxButtonInputProperties("B");
			case XboxGamepadButton::X: return XboxButtonInputProperties("X");
			case XboxGamepadButton::Y: return XboxButtonInputProperties("Y");
			case XboxGamepadButton::RightBumper: return XboxButtonInputProperties("Right Bumper");
			case XboxGamepadButton::LeftBumper: return XboxButtonInputProperties("Left Bumper");
			case XboxGamepadButton::RightStickButton: return XboxButtonInputProperties("Right Stick");
			case XboxGamepadButton::LeftStickButton: return XboxButtonInputProperties("Left Stick");
			case XboxGamepadButton::Back: return XboxButtonInputProperties("Back");
			case XboxGamepadButton::Start: return XboxButtonInputProperties("Start");
			case XboxGamepadButton::Right: return XboxButtonInputProperties("Right");
			case XboxGamepadButton::Left: return XboxButtonInputProperties("Left");
			case XboxGamepadButton::Down: return XboxButtonInputProperties("Down");
			case XboxGamepadButton::Up: return XboxButtonInputProperties("Up");
			}
		}
		else
		{
			auto axis = input - ButtonCount;
			switch (axis)
			{
			case 0: return XboxStickAxisInputProperties("Left Stick X Axis");
			case 1: return XboxStickAxisInputProperties("Left Stick Y Axis");
			case 2: return XboxStickAxisInputProperties("Right Stick X Axis");
			case 3: return XboxStickAxisInputProperties("Right Stick Y Axis");
			case 4: return XboxStickAxisInputProperties("Left Trigger");
			case 5: return XboxStickAxisInputProperties("Right Trigger");
			}
		}
		return {};
	}

	uint8_t IXboxGamepadDevice::GetStickCount() const
	{
		return 2;
	}

	uint8_t IXboxGamepadDevice::GetStickAxisCount(uint8_t stick_num) const
	{
		return 2;
	}

	uint8_t IXboxGamepadDevice::GetButtonCount() const
	{
		return ButtonCount;
	}

	bool IXboxGamepadDevice::IsValidNavigation(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept:
		case UINavigationInput::Cancel:
		case UINavigationInput::Menu:
		case UINavigationInput::View:
		case UINavigationInput::Left:
		case UINavigationInput::Right:
		case UINavigationInput::Up:
		case UINavigationInput::Down:
		case UINavigationInput::Back:
		case UINavigationInput::Forward:
		case UINavigationInput::PageUp:
		case UINavigationInput::PageDown:
		case UINavigationInput::PageLeft:
		case UINavigationInput::PageRight:
		case UINavigationInput::ScrollUp:
		case UINavigationInput::ScrollDown:
		case UINavigationInput::ScrollLeft:
		case UINavigationInput::ScrollRight:
			return true;

		case UINavigationInput::Home:
		case UINavigationInput::End:
			return false;
		}
		return false;
	}

	bool IXboxGamepadDevice::GetNavigation(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return GetInputStateDigital((DeviceInputID)XboxGamepadButton::A);
		case UINavigationInput::Cancel: return GetInputStateDigital((DeviceInputID)XboxGamepadButton::B);
		case UINavigationInput::Menu: return GetInputStateDigital((DeviceInputID)XboxGamepadButton::Start);
		case UINavigationInput::View: return GetInputStateDigital((DeviceInputID)XboxGamepadButton::Back);
		case UINavigationInput::Left: return GetInputStateDigital((DeviceInputID)XboxGamepadButton::Left);
		case UINavigationInput::Right: return GetInputStateDigital((DeviceInputID)XboxGamepadButton::Right);
		case UINavigationInput::Up: return GetInputStateDigital((DeviceInputID)XboxGamepadButton::Up);
		case UINavigationInput::Down: return GetInputStateDigital((DeviceInputID)XboxGamepadButton::Down);
		case UINavigationInput::Back: return GetInputStateDigital((DeviceInputID)XboxGamepadButton::LeftBumper);
		case UINavigationInput::Forward: return GetInputStateDigital((DeviceInputID)XboxGamepadButton::RightBumper);
		case UINavigationInput::PageUp: return GetStickAxisState(0, 0) < -0.1;
		case UINavigationInput::PageDown: return GetStickAxisState(0, 0) > 0.1;
		case UINavigationInput::PageLeft: return GetStickAxisState(0, 1) < -0.1;
		case UINavigationInput::PageRight: return GetStickAxisState(0, 1) > 0.1;
		case UINavigationInput::ScrollUp: return GetStickAxisState(1, 0) < -0.1;
		case UINavigationInput::ScrollDown:	return GetStickAxisState(1, 0) > 0.1;
		case UINavigationInput::ScrollLeft:	return GetStickAxisState(1, 1) < -0.1;
		case UINavigationInput::ScrollRight: return GetStickAxisState(1, 1) > 0.1;

		case UINavigationInput::Home:
		case UINavigationInput::End:
			return false;
		}
		return false;
	}

	bool IXboxGamepadDevice::GetNavigationLastFrame(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return GetInputStateLastFrameDigital((DeviceInputID)XboxGamepadButton::A);
		case UINavigationInput::Cancel: return GetInputStateLastFrameDigital((DeviceInputID)XboxGamepadButton::B);
		case UINavigationInput::Menu: return GetInputStateLastFrameDigital((DeviceInputID)XboxGamepadButton::Start);
		case UINavigationInput::View: return GetInputStateLastFrameDigital((DeviceInputID)XboxGamepadButton::Back);
		case UINavigationInput::Left: return GetInputStateLastFrameDigital((DeviceInputID)XboxGamepadButton::Left);
		case UINavigationInput::Right: return GetInputStateLastFrameDigital((DeviceInputID)XboxGamepadButton::Right);
		case UINavigationInput::Up: return GetInputStateLastFrameDigital((DeviceInputID)XboxGamepadButton::Up);
		case UINavigationInput::Down: return GetInputStateLastFrameDigital((DeviceInputID)XboxGamepadButton::Down);
		case UINavigationInput::Back: return GetInputStateLastFrameDigital((DeviceInputID)XboxGamepadButton::LeftBumper);
		case UINavigationInput::Forward: return GetInputStateLastFrameDigital((DeviceInputID)XboxGamepadButton::RightBumper);
		case UINavigationInput::PageUp: return GetStickAxisStateLastFrame(0, 0) < -0.1;
		case UINavigationInput::PageDown: return GetStickAxisStateLastFrame(0, 0) > 0.1;
		case UINavigationInput::PageLeft: return GetStickAxisStateLastFrame(0, 1) < -0.1;
		case UINavigationInput::PageRight: return GetStickAxisStateLastFrame(0, 1) > 0.1;
		case UINavigationInput::ScrollUp: return GetStickAxisStateLastFrame(1, 0) < -0.1;
		case UINavigationInput::ScrollDown:	return GetStickAxisStateLastFrame(1, 0) > 0.1;
		case UINavigationInput::ScrollLeft:	return GetStickAxisStateLastFrame(1, 1) < -0.1;
		case UINavigationInput::ScrollRight: return GetStickAxisStateLastFrame(1, 1) > 0.1;

		case UINavigationInput::Home:
		case UINavigationInput::End:
			return false;
		}
		return false;
	}

	vec2 UINavigationInputToDirection(UINavigationInput input)
	{
		switch (input)
		{
		case UINavigationInput::Accept: return {};
		case UINavigationInput::Cancel: return {};
		case UINavigationInput::Left: return { -1.0f, 0.0f };
		case UINavigationInput::Right: return { 1.0f, 0.0f };
		case UINavigationInput::Up: return { 0.0f, -1.0f };
		case UINavigationInput::Down: return { 0.0f, 1.0f };
		case UINavigationInput::Home: return { 0.0f, -INFINITY };
		case UINavigationInput::End: return { 0.0f, INFINITY };
		case UINavigationInput::PageUp: return { 0.0f, -10.0f };
		case UINavigationInput::PageDown: return { 0.0f, 10.0f };
		case UINavigationInput::Back: return { -1.0f, 0.0f };
		case UINavigationInput::Forward: return { 1.0f, 0.0f };
		case UINavigationInput::Menu: return {};
		case UINavigationInput::View: return {};
		case UINavigationInput::PageLeft: return { -1.0f, 0.0f };
		case UINavigationInput::PageRight: return { 1.0f, 0.0f };
		case UINavigationInput::ScrollUp: return { 0.0f, -1.0f };
		case UINavigationInput::ScrollDown: return { 0.0f, 1.0f };
		case UINavigationInput::ScrollLeft: return { -1.0f, 0.0f };
		case UINavigationInput::ScrollRight: return { 1.0f, 0.0f };
		}
		return {};
	}

	void IInputDevice::ReportInvalidInput(DeviceInputID input) const
	{
		ParentSystem.ErrorReporter.NewError("Input is not valid")
			.Value("Input", input)
			.Value("Device", GetName())
			.Value("ValidRange", GetMaxInput() - 1)
			.Perform();
	}

}