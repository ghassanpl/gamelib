#include "InputDevice.h"
#include "InputSystem.h"

#pragma warning(error: 4061)
#pragma warning(error: 4062)

namespace gamelib
{

	std::string_view IInputDevice::Name() const { return StringPropertyValue(StringProperty::Name); }

	OutputProperties IInputDevice::OutputProperties(DeviceOutputID input) const
	{
		ParentSystem.ErrorReporter->NewError("Input device has no output properties").Value("Device", Name()).Perform();
		std::terminate();
	}

	bool IKeyboardDevice::IsNavigationValid(UINavigationInput input) const
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

	bool IKeyboardDevice::IsNavigationPressed(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return IsInputPressed((DeviceInputID)KeyboardKey::Enter);
		case UINavigationInput::Cancel: return IsInputPressed((DeviceInputID)KeyboardKey::Escape);
		case UINavigationInput::Left: return IsInputPressed((DeviceInputID)KeyboardKey::Left);
		case UINavigationInput::Right: return IsInputPressed((DeviceInputID)KeyboardKey::Right);
		case UINavigationInput::Up: return IsInputPressed((DeviceInputID)KeyboardKey::Up);
		case UINavigationInput::Down: return IsInputPressed((DeviceInputID)KeyboardKey::Down);
		case UINavigationInput::Home: return IsInputPressed((DeviceInputID)KeyboardKey::Home);
		case UINavigationInput::End: return IsInputPressed((DeviceInputID)KeyboardKey::End);
		case UINavigationInput::PageUp: return IsInputPressed((DeviceInputID)KeyboardKey::PgUp);
		case UINavigationInput::PageDown: return IsInputPressed((DeviceInputID)KeyboardKey::PgDn);
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

	bool IKeyboardDevice::WasNavigationPressedLastFrame(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return WasInputPressedLastFrame((DeviceInputID)KeyboardKey::Enter);
		case UINavigationInput::Cancel: return WasInputPressedLastFrame((DeviceInputID)KeyboardKey::Escape);
		case UINavigationInput::Left: return WasInputPressedLastFrame((DeviceInputID)KeyboardKey::Left);
		case UINavigationInput::Right: return WasInputPressedLastFrame((DeviceInputID)KeyboardKey::Right);
		case UINavigationInput::Up: return WasInputPressedLastFrame((DeviceInputID)KeyboardKey::Up);
		case UINavigationInput::Down: return WasInputPressedLastFrame((DeviceInputID)KeyboardKey::Down);
		case UINavigationInput::Home: return WasInputPressedLastFrame((DeviceInputID)KeyboardKey::Home);
		case UINavigationInput::End: return WasInputPressedLastFrame((DeviceInputID)KeyboardKey::End);
		case UINavigationInput::PageUp: return WasInputPressedLastFrame((DeviceInputID)KeyboardKey::PgUp);
		case UINavigationInput::PageDown: return WasInputPressedLastFrame((DeviceInputID)KeyboardKey::PgDn);
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

	bool IMouseDevice::IsNavigationValid(UINavigationInput input) const
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
			return VerticalWheelInputID() != InvalidDeviceInputID;
		case UINavigationInput::ScrollLeft:
		case UINavigationInput::ScrollRight:
			return HorizontalWheelInputID() != InvalidDeviceInputID;

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

	bool IMouseDevice::IsNavigationPressed(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return IsInputPressed((DeviceInputID)MouseButton::Left);
		case UINavigationInput::Cancel: return IsInputPressed((DeviceInputID)MouseButton::Right);
		case UINavigationInput::ScrollUp: return InputValue(VerticalWheelInputID()) < 0;
		case UINavigationInput::ScrollDown: return InputValue(VerticalWheelInputID()) > 0;
		case UINavigationInput::ScrollLeft: return InputValue(HorizontalWheelInputID()) < 0;
		case UINavigationInput::ScrollRight: return InputValue(HorizontalWheelInputID()) > 0;

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

	bool IMouseDevice::WasNavigationPressedLastFrame(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return WasInputPressedLastFrame((DeviceInputID)MouseButton::Left);
		case UINavigationInput::Cancel: return WasInputPressedLastFrame((DeviceInputID)MouseButton::Right);
		case UINavigationInput::ScrollUp: return InputValueLastFrame(VerticalWheelInputID()) < 0;
		case UINavigationInput::ScrollDown: return InputValueLastFrame(VerticalWheelInputID()) > 0;
		case UINavigationInput::ScrollLeft: return InputValueLastFrame(HorizontalWheelInputID()) < 0;
		case UINavigationInput::ScrollRight: return InputValueLastFrame(HorizontalWheelInputID()) > 0;

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

	DeviceInputID IXboxGamepadDevice::MaxInputID() const
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

	InputProperties IXboxGamepadDevice::PropertiesOf(DeviceInputID input) const
	{
		if (!IsInputValid(input)) return {};
		if (input < DefaultButtonCount)
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
			auto axis = input - DefaultButtonCount;
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

	uint8_t IXboxGamepadDevice::StickCount() const
	{
		return 2;
	}

	uint8_t IXboxGamepadDevice::StickAxisCount(uint8_t stick_num) const
	{
		return 2;
	}

	uint8_t IXboxGamepadDevice::ButtonCount() const
	{
		return DefaultButtonCount;
	}

	bool IXboxGamepadDevice::IsNavigationValid(UINavigationInput input) const
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

	bool IXboxGamepadDevice::IsNavigationPressed(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return IsInputPressed((DeviceInputID)XboxGamepadButton::A);
		case UINavigationInput::Cancel: return IsInputPressed((DeviceInputID)XboxGamepadButton::B);
		case UINavigationInput::Menu: return IsInputPressed((DeviceInputID)XboxGamepadButton::Start);
		case UINavigationInput::View: return IsInputPressed((DeviceInputID)XboxGamepadButton::Back);
		case UINavigationInput::Left: return IsInputPressed((DeviceInputID)XboxGamepadButton::Left);
		case UINavigationInput::Right: return IsInputPressed((DeviceInputID)XboxGamepadButton::Right);
		case UINavigationInput::Up: return IsInputPressed((DeviceInputID)XboxGamepadButton::Up);
		case UINavigationInput::Down: return IsInputPressed((DeviceInputID)XboxGamepadButton::Down);
		case UINavigationInput::Back: return IsInputPressed((DeviceInputID)XboxGamepadButton::LeftBumper);
		case UINavigationInput::Forward: return IsInputPressed((DeviceInputID)XboxGamepadButton::RightBumper);
		case UINavigationInput::PageUp: return StickAxisValue(0, 0) < -0.1;
		case UINavigationInput::PageDown: return StickAxisValue(0, 0) > 0.1;
		case UINavigationInput::PageLeft: return StickAxisValue(0, 1) < -0.1;
		case UINavigationInput::PageRight: return StickAxisValue(0, 1) > 0.1;
		case UINavigationInput::ScrollUp: return StickAxisValue(1, 0) < -0.1;
		case UINavigationInput::ScrollDown:	return StickAxisValue(1, 0) > 0.1;
		case UINavigationInput::ScrollLeft:	return StickAxisValue(1, 1) < -0.1;
		case UINavigationInput::ScrollRight: return StickAxisValue(1, 1) > 0.1;

		case UINavigationInput::Home:
		case UINavigationInput::End:
			return false;
		}
		return false;
	}

	bool IXboxGamepadDevice::WasNavigationPressedLastFrame(UINavigationInput input) const
	{
		switch (input)
		{
		case UINavigationInput::Accept: return WasInputPressedLastFrame((DeviceInputID)XboxGamepadButton::A);
		case UINavigationInput::Cancel: return WasInputPressedLastFrame((DeviceInputID)XboxGamepadButton::B);
		case UINavigationInput::Menu: return WasInputPressedLastFrame((DeviceInputID)XboxGamepadButton::Start);
		case UINavigationInput::View: return WasInputPressedLastFrame((DeviceInputID)XboxGamepadButton::Back);
		case UINavigationInput::Left: return WasInputPressedLastFrame((DeviceInputID)XboxGamepadButton::Left);
		case UINavigationInput::Right: return WasInputPressedLastFrame((DeviceInputID)XboxGamepadButton::Right);
		case UINavigationInput::Up: return WasInputPressedLastFrame((DeviceInputID)XboxGamepadButton::Up);
		case UINavigationInput::Down: return WasInputPressedLastFrame((DeviceInputID)XboxGamepadButton::Down);
		case UINavigationInput::Back: return WasInputPressedLastFrame((DeviceInputID)XboxGamepadButton::LeftBumper);
		case UINavigationInput::Forward: return WasInputPressedLastFrame((DeviceInputID)XboxGamepadButton::RightBumper);
		case UINavigationInput::PageUp: return StickAxisValueLastFrame(0, 0) < -0.1;
		case UINavigationInput::PageDown: return StickAxisValueLastFrame(0, 0) > 0.1;
		case UINavigationInput::PageLeft: return StickAxisValueLastFrame(0, 1) < -0.1;
		case UINavigationInput::PageRight: return StickAxisValueLastFrame(0, 1) > 0.1;
		case UINavigationInput::ScrollUp: return StickAxisValueLastFrame(1, 0) < -0.1;
		case UINavigationInput::ScrollDown:	return StickAxisValueLastFrame(1, 0) > 0.1;
		case UINavigationInput::ScrollLeft:	return StickAxisValueLastFrame(1, 1) < -0.1;
		case UINavigationInput::ScrollRight: return StickAxisValueLastFrame(1, 1) > 0.1;

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
		ParentSystem.ErrorReporter->NewError("Input is not valid")
			.Value("Input", input)
			.Value("Device", Name())
			.Value("ValidRange", MaxInputID() - 1)
			.Perform();
	}

}