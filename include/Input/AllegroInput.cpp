#include "AllegroInput.h"
#include "InputSystem.h"
#include "../Includes/MagicEnum.h"
#include <allegro5/display.h>
#include <allegro5/keyboard.h>
#include <allegro5/joystick.h>
#include <allegro5/mouse.h>
#include <allegro5/mouse_cursor.h>
#include "../Includes/Assuming.h"

namespace gamelib
{
	struct ButtonInputProperties : InputProperties
	{
		ButtonInputProperties(std::string_view name, std::string glyph_url = {})
		{
			Name = name;
			Flags.set(InputFlags::Digital);
			DeadZoneMin = DeadZoneMax = MinValue = 0;
			StepSize = 1;
			GlyphURL = std::move(glyph_url);
		}
	};

	struct StickAxisInputProperties : InputProperties
	{
		StickAxisInputProperties(std::string_view name, double min = -1.0, double max = 1.0)
		{
			Name = name;
			Flags.unset(InputFlags::Digital);
			Flags.set(InputFlags::ReturnsToNeutral, InputFlags::HasDeadzone);
			MinValue = min;
			MaxValue = max;
		}
	};

	struct MouseAxisInputProperties : InputProperties
	{
		MouseAxisInputProperties(std::string_view name, double max)
		{
			Name = name;
			Flags.unset(InputFlags::Digital);
			MinValue = 0;
			MaxValue = max;
		}
	};

	std::string_view AllegroKeyboard::StringPropertyValue(IInputDevice::StringProperty property) const
	{
		if (property == StringProperty::Name)
			return "Main Keyboard";
		return "";
	}

	glm::vec3 AllegroKeyboard::NumberPropertyValue(NumberProperty property) const
	{
		return glm::vec3{};
	}

	DeviceInputID AllegroKeyboard::MaxInputID() const
	{
		return (int)KeyboardKey::Max;
	}

	double AllegroKeyboard::InputValue(DeviceInputID input) const
	{
		if (!IsInputValid(input))
			return 0;
		return CurrentState[input].Down ? 1.0 : 0.0;
	}

	bool AllegroKeyboard::IsInputPressed(DeviceInputID input) const
	{
		if (!IsInputValid(input))
			return false;
		return CurrentState[input].Down;
	}

	double AllegroKeyboard::InputValueLastFrame(DeviceInputID input) const
	{
		if (!IsInputValid(input))
			return 0;
		return LastFrameState[input].Down ? 1.0 : 0.0;
	}

	bool AllegroKeyboard::WasInputPressedLastFrame(DeviceInputID input) const
	{
		if (!IsInputValid(input))
			return false;
		return LastFrameState[input].Down;
	}

	InputProperties AllegroKeyboard::PropertiesOf(DeviceInputID input) const
	{
		if (!IsInputValid(input))
			ReportInvalidInput(input);
		return ButtonInputProperties{ magic_enum::enum_name((KeyboardKey)input) };
	}

	void AllegroKeyboard::ForceRefresh()
	{
		ALLEGRO_KEYBOARD_STATE state;
		al_get_keyboard_state(&state);
		for (int i = 0; i < (int)KeyboardKey::Max; i++)
		{
			CurrentState[i].Down = al_key_down(&state, i);
		}
	}

	void AllegroKeyboard::NewFrame()
	{
		LastFrameState = CurrentState;
		//ForceRefresh();
	}

	void AllegroKeyboard::KeyPressed(DeviceInputID key)
	{
		if (CurrentState[key].Down)
			CurrentState[key].RepeatCount++;
		CurrentState[key].Down = true;
	}

	void AllegroKeyboard::KeyReleased(DeviceInputID key)
	{
		CurrentState[key].RepeatCount = 0;
		CurrentState[key].Down = false;
	}

	enum_flags<InputDeviceFlags> AllegroKeyboard::Flags() const
	{
		return enum_flags<InputDeviceFlags>();
	}

	/// TODO: Leds

	bool AllegroKeyboard::IsStringPropertyValid(StringProperty property) const
	{
		return property == StringProperty::Name;
	}

	bool AllegroKeyboard::IsNumberPropertyValid(NumberProperty property) const
	{
		return false;
	}

	/// Mouse

	std::string_view AllegroMouse::StringPropertyValue(IInputDevice::StringProperty property) const
	{
		if (property == StringProperty::Name)
			return "Main Mouse";
		return "";
	}

	DeviceInputID AllegroMouse::MaxInputID() const
	{
		return TotalInputs;
	}

	double AllegroMouse::InputValue(DeviceInputID input) const
	{
		if (!IsInputValid(input))
			return 0.0;
		return CurrentState[input];
	}

	bool AllegroMouse::IsInputPressed(DeviceInputID input) const
	{
		if (!IsInputValid(input) || input >= ButtonCount)
			return false;
		return CurrentState[input] != 0;
	}

	double AllegroMouse::InputValueLastFrame(DeviceInputID input) const
	{
		if (!IsInputValid(input))
			return 0.0;
		return LastFrameState[input];
	}

	bool AllegroMouse::WasInputPressedLastFrame(DeviceInputID input) const
	{
		if (!IsInputValid(input) || input >= ButtonCount)
			return false;
		return LastFrameState[input] != 0;
	}

	InputProperties AllegroMouse::PropertiesOf(DeviceInputID input) const
	{
		switch (input)
		{
		case (int)MouseButton::Left:
		case (int)MouseButton::Right:
		case (int)MouseButton::Middle:
			return InputProperties{ 
				.Name = (std::string)magic_enum::enum_name((MouseButton)input), 
				.GlyphURL = fmt::format("ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_Mouse_{}.png", magic_enum::enum_name((MouseButton)input))
			};
		case (int)MouseButton::Button4:
		case (int)MouseButton::Button5:
			return InputProperties{ (std::string)magic_enum::enum_name((MouseButton)input) };
		case Wheel0: /// wheel 0
			return InputProperties{ .Name = "Vertical Wheel", .MinValue = -INFINITY, .MaxValue = INFINITY };
		case Wheel1: /// wheel 1
			return InputProperties{ .Name = "Horizontal Wheel", .MinValue = -INFINITY, .MaxValue = INFINITY };
		case XAxis: /// x axis
			return MouseAxisInputProperties{ "X Axis", std::numeric_limits<double>::max() };
		case YAxis: /// y axis
			return MouseAxisInputProperties{ "Y Axis", std::numeric_limits<double>::max() };
		default:
			ReportInvalidInput(input);
			std::terminate();
		}
	}

	void AllegroMouse::ForceRefresh()
	{
	}

	void AllegroMouse::NewFrame()
	{
		LastFrameState = CurrentState;
		CurrentState[Wheel0] = 0;
		CurrentState[Wheel1] = 0;
	}

	void AllegroMouse::ShowCursor(bool show)
	{
		show ? al_show_mouse_cursor(al_get_current_display()) : al_hide_mouse_cursor(al_get_current_display());
		mCursorVisible = show;
	}

	bool AllegroMouse::IsCursorVisible() const
	{
		return mCursorVisible;
	}

	bool AllegroMouse::IsCursorShapeAvailable(MouseCursorShape shape) const
	{
		return int(shape) <= 19;
	}

	void AllegroMouse::SetCursorShape(MouseCursorShape shape)
	{
		if (IsCursorShapeAvailable(shape))
			al_set_system_mouse_cursor(al_get_current_display(), ALLEGRO_SYSTEM_MOUSE_CURSOR(int(shape)));
	}

	bool AllegroMouse::IsCustomCursorShapeAvailable(std::string_view shape) const
	{
		return false;
	}

	void AllegroMouse::SetCustomCursorShape(std::string_view name)
	{
	}

	void AllegroMouse::SetCustomCursorShape(void* resource, MouseCursorShape replace, vec2 hotspot)
	{
		/// al_destroy_mouse_cursor
		/// al_create_mouse_cursor
		/// al_set_mouse_cursor
	}

	bool AllegroMouse::CanWarp() const
	{
		return true;
	}

	void AllegroMouse::Warp(vec2 pos)
	{
		al_set_mouse_xy(al_get_current_display(), int(pos.x), int(pos.y));
	}

	void AllegroMouse::MouseWheelScrolled(float delta, unsigned wheel)
	{
		CurrentState[Wheel0 + wheel] += delta;
	}

	void AllegroMouse::MouseButtonPressed(MouseButton button)
	{
		CurrentState[(unsigned)button] = 1;
	}

	void AllegroMouse::MouseButtonReleased(MouseButton button)
	{
		CurrentState[(unsigned)button] = 0;
	}

	void AllegroMouse::MouseMoved(int x, int y)
	{
		CurrentState[XAxis] = x;
		CurrentState[YAxis] = y;
	}

	void AllegroMouse::MouseEntered()
	{
	}

	void AllegroMouse::MouseLeft()
	{
	}

	enum_flags<InputDeviceFlags> AllegroMouse::Flags() const
	{
		return enum_flags<InputDeviceFlags>();
	}

	bool AllegroMouse::IsStringPropertyValid(StringProperty property) const
	{
		return false;
	}

	bool AllegroMouse::IsNumberPropertyValid(NumberProperty property) const
	{
		return false;
	}

	glm::vec3 AllegroMouse::NumberPropertyValue(NumberProperty property) const
	{
		return glm::vec3();
	}

	bool AllegroMouse::IsConnected() const
	{
		return true;
	}

	/// Gamepad

	AllegroGamepad::AllegroGamepad(IInputSystem& sys, ALLEGRO_JOYSTICK* stick)
		: IInputDevice(sys), IXboxGamepadDevice(sys), mJoystick(stick)
	{
		mName = al_get_joystick_name(stick);
		mSticks.resize(al_get_joystick_num_sticks(stick));
		mButtons.resize(al_get_joystick_num_buttons(stick));
		mNumInputs += (uint8_t)mButtons.size();
		for (size_t i = 0; i < mSticks.size(); i++)
		{
			mSticks[i].Name = al_get_joystick_stick_name(stick, (int)i);
			mSticks[i].NumAxes = (uint8_t)al_get_joystick_num_axes(stick, (int)i);
			mNumInputs += mSticks[i].NumAxes;
			mNumAxes += mSticks[i].NumAxes;
			for (size_t a = 0; a < mSticks[i].NumAxes; a++)
			{
				mSticks[i].Axes[a] = StickAxisInputProperties(fmt::format("{} {}", mSticks[i].Name, al_get_joystick_axis_name(stick, (int)i, (int)a)));
			}
		}
		for (size_t i = 0; i < mButtons.size(); i++)
		{
			mButtons[i] = ButtonInputProperties(al_get_joystick_button_name(stick, (int)i));
		}
	}


	std::string_view AllegroGamepad::StringPropertyValue(IInputDevice::StringProperty property) const
	{
		if (property == StringProperty::Name)
			return mName;
		return "";
	}


	DeviceInputID AllegroGamepad::MaxInputID() const
	{
		return mNumInputs;
	}

	double AllegroGamepad::InputValue(DeviceInputID input) const
	{
		if (!IsInputValid(input)) return 0;
		if (input < mButtons.size())
		{
			return CurrentState.Button[input];
		}
		else
		{
			auto stick_and_axis = CalculateStickAndAxis(input - (DeviceInputID)mButtons.size());
			return CurrentState.Stick[stick_and_axis.first].Axis[stick_and_axis.second];
		}
	}

	bool AllegroGamepad::IsInputPressed(DeviceInputID input) const
	{
		return InputValue(input) > 0.5;
	}

	double AllegroGamepad::InputValueLastFrame(DeviceInputID input) const
	{
		if (!IsInputValid(input)) return 0;
		if (input < mButtons.size())
		{
			return LastFrameState.Button[input];
		}
		else
		{
			auto stick_and_axis = CalculateStickAndAxis(input - (DeviceInputID)mButtons.size());
			return LastFrameState.Stick[stick_and_axis.first].Axis[stick_and_axis.second];
		}
	}

	bool AllegroGamepad::WasInputPressedLastFrame(DeviceInputID input) const
	{
		return InputValueLastFrame(input) > 0.5;
	}

	InputProperties AllegroGamepad::PropertiesOf(DeviceInputID input) const
	{
		if (!IsInputValid(input)) return {};
		if (input < mButtons.size())
		{
			return mButtons[input];
		}
		else
		{
			auto stick_and_axis = CalculateStickAndAxis(input - (DeviceInputID)mButtons.size());
			return mSticks[stick_and_axis.first].Axes[stick_and_axis.second];
		}
	}

	void AllegroGamepad::ForceRefresh()
	{
		static_assert(sizeof(ALLEGRO_JOYSTICK_STATE) == sizeof(CurrentState));
		al_get_joystick_state(mJoystick, (ALLEGRO_JOYSTICK_STATE*)&CurrentState);
	}

	void AllegroGamepad::NewFrame()
	{
		LastFrameState = CurrentState;
	}

	bool AllegroGamepad::IsConnected() const
	{
		return al_get_joystick_active(mJoystick);
	}

	uint8_t AllegroGamepad::StickCount() const
	{
		return (uint8_t)mSticks.size();
	}

	uint8_t AllegroGamepad::StickAxisCount(uint8_t stick_num) const
	{
		if (stick_num < mSticks.size())
			return mSticks[stick_num].NumAxes;
		return 0;
	}

	uint8_t AllegroGamepad::ButtonCount() const
	{
		return (uint8_t)mButtons.size();
	}

	bool AllegroGamepad::IsButtonPressed(uint8_t button_num) const
	{
		if (button_num < mButtons.size())
			return CurrentState.Button[button_num] != 0;
		return false;
	}

	glm::vec3 AllegroGamepad::StickValue(uint8_t stick_num) const
	{
		if (stick_num < mSticks.size())
			return { CurrentState.Stick[stick_num].Axis[0], CurrentState.Stick[stick_num].Axis[1], CurrentState.Stick[stick_num].Axis[2] };
		return {};
	}

	float AllegroGamepad::StickAxisValue(uint8_t stick_num, uint8_t axis_num) const
	{
		if (stick_num < mSticks.size() && mSticks[stick_num].NumAxes < axis_num)
			return CurrentState.Stick[stick_num].Axis[axis_num];
		return {};
	}

	std::pair<uint8_t, uint8_t> AllegroGamepad::CalculateStickAndAxis(DeviceInputID input) const
	{
		AssumingLess(input, mNumAxes);
		uint8_t stick = 0;
		uint8_t axis = 0;
		while (input > 0)
		{
			AssumingLess(stick, mSticks.size());
			AssumingLess(axis, mSticks[stick].Axes.size());

			axis++;
			if (axis >= mSticks[stick].NumAxes)
			{
				stick++;
				axis = 0;
			}
			input--;
		}
		return { stick, axis };
	}

	bool AllegroGamepad::WasButtonPressedLastFrame(uint8_t button_num) const
	{
		if (button_num < mButtons.size())
			return LastFrameState.Button[button_num] != 0;
		return false;
	}

	glm::vec3 AllegroGamepad::StickValueLastFrame(uint8_t stick_num) const
	{
		if (stick_num < mSticks.size())
			return { LastFrameState.Stick[stick_num].Axis[0], LastFrameState.Stick[stick_num].Axis[1], LastFrameState.Stick[stick_num].Axis[2] };
		return {};
	}

	float AllegroGamepad::StickAxisValueLastFrame(uint8_t stick_num, uint8_t axis_num) const
	{
		if (stick_num < mSticks.size() && mSticks[stick_num].NumAxes < axis_num)
			return LastFrameState.Stick[stick_num].Axis[axis_num];
		return {};
	}

	enum_flags<InputDeviceFlags> AllegroGamepad::Flags() const
	{
		return enum_flags<InputDeviceFlags>();
	}

	bool AllegroGamepad::IsStringPropertyValid(StringProperty property) const
	{
		return false;
	}

	bool AllegroGamepad::IsNumberPropertyValid(NumberProperty property) const
	{
		return false;
	}

	glm::vec3 AllegroGamepad::NumberPropertyValue(NumberProperty property) const
	{
		return glm::vec3();
	}

	void AllegroInput::Init()
	{
		mInputDevices.push_back(std::make_unique<AllegroKeyboard>(*this)); /// static constexpr InputDeviceIndex KeyboardDeviceID = 0;
		mKeyboard = dynamic_cast<AllegroKeyboard*>(mInputDevices.back().get());

		mInputDevices.push_back(std::make_unique<AllegroMouse>(*this)); /// static constexpr InputDeviceIndex MouseDeviceID = 1;
		mMouse = dynamic_cast<AllegroMouse*>(mInputDevices.back().get());

		RefreshJoysticks();

		IInputSystem::Init();
	}

	void AllegroInput::ProcessEvent(ALLEGRO_EVENT const& event)
	{
		switch (event.type)
		{
		case ALLEGRO_EVENT_KEY_DOWN:
			static_cast<AllegroKeyboard*>(Keyboard())->KeyPressed(event.keyboard.keycode);
			SetLastActiveDevice(mKeyboard, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_KEY_CHAR:
			SetLastActiveDevice(mKeyboard, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_KEY_UP:
			static_cast<AllegroKeyboard*>(Keyboard())->KeyReleased(event.keyboard.keycode);
			SetLastActiveDevice(mKeyboard, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_MOUSE_AXES:
			static_cast<AllegroMouse*>(Mouse())->MouseWheelScrolled(event.mouse.dz, event.mouse.dw);
			static_cast<AllegroMouse*>(Mouse())->MouseMoved(event.mouse.x, event.mouse.y);
			SetLastActiveDevice(mMouse, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			static_cast<AllegroMouse*>(Mouse())->MouseButtonPressed(MouseButton(event.mouse.button - 1));
			SetLastActiveDevice(mMouse, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			static_cast<AllegroMouse*>(Mouse())->MouseButtonReleased(MouseButton(event.mouse.button - 1));
			SetLastActiveDevice(mMouse, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
			static_cast<AllegroMouse*>(Mouse())->MouseEntered();
			SetLastActiveDevice(mMouse, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
			static_cast<AllegroMouse*>(Mouse())->MouseLeft();
			SetLastActiveDevice(mMouse, event.any.timestamp);
			break;

		case ALLEGRO_EVENT_JOYSTICK_AXIS:
			Assuming(mJoystickMap.contains(event.joystick.id));
			SetLastActiveDevice(mJoystickMap[event.joystick.id], event.any.timestamp);
			dynamic_cast<AllegroGamepad*>(mLastActiveDevice)->CurrentState.Stick[event.joystick.stick].Axis[event.joystick.axis] = event.joystick.pos;
			break;
		case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
			Assuming(mJoystickMap.contains(event.joystick.id));
			SetLastActiveDevice(mJoystickMap[event.joystick.id], event.any.timestamp);
			dynamic_cast<AllegroGamepad*>(mLastActiveDevice)->CurrentState.Button[event.joystick.button] = 1;
			break;
		case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
			Assuming(mJoystickMap.contains(event.joystick.id));
			SetLastActiveDevice(mJoystickMap[event.joystick.id], event.any.timestamp);
			dynamic_cast<AllegroGamepad*>(mLastActiveDevice)->CurrentState.Button[event.joystick.button] = 0;
			break;
		case ALLEGRO_EVENT_JOYSTICK_CONFIGURATION:
			RefreshJoysticks();
			break;
		}
	}

	void AllegroInput::RefreshJoysticks()
	{
		if (al_reconfigure_joysticks())
		{
			if (LastDeviceActive() == mFirstGamepad)
				SetLastActiveDevice(nullptr, 0);
			mFirstGamepad = nullptr;
			mJoystickMap.clear();

			mInputDevices.resize(2);
			for (int i = 0; i < al_get_num_joysticks(); i++)
			{
				auto stick = al_get_joystick(i);
				auto gamepad = std::make_unique<AllegroGamepad>(*this, stick);
				mJoystickMap[stick] = gamepad.get();
				if (i == 0) mFirstGamepad = gamepad.get();
				mInputDevices.push_back(std::move(gamepad));
			}
		}
	}

}