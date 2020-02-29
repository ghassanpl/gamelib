#include "AllegroInput.h"
#include "InputSystem.h"
#include "../Includes/MagicEnum.h"
#include <allegro5/keyboard.h>
#include <allegro5/joystick.h>
#include "../../../assuming/include/assuming.h"

namespace gamelib
{
	struct ButtonInputProperties : InputProperties
	{
		ButtonInputProperties(std::string_view name)
		{
			Name = name;
			Flags.set(InputFlags::Digital);
			DeadZoneMin = DeadZoneMax = MinValue = 0;
			StepSize = 1;
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

	std::string_view AllegroKeyboard::GetStringProperty(IInputDevice::StringProperty property) const
	{
		if (property == StringProperty::Name)
			return "Main Keyboard";
		return "";
	}

	glm::vec3 AllegroKeyboard::GetNumberProperty(NumberProperty property) const
	{
		return glm::vec3{};
	}

	DeviceInputID AllegroKeyboard::GetMaxInput() const
	{
		return (int)KeyboardKey::Max;
	}

	double AllegroKeyboard::GetInputState(DeviceInputID input) const
	{
		if (!IsValidInput(input))
			return 0;
		return CurrentState[input].Down ? 1.0 : 0.0;
	}

	bool AllegroKeyboard::GetInputStateDigital(DeviceInputID input) const
	{
		if (!IsValidInput(input))
			return false;
		return CurrentState[input].Down;
	}

	double AllegroKeyboard::GetInputStateLastFrame(DeviceInputID input) const
	{
		if (!IsValidInput(input))
			return 0;
		return LastFrameState[input].Down ? 1.0 : 0.0;
	}

	bool AllegroKeyboard::GetInputStateLastFrameDigital(DeviceInputID input) const
	{
		if (!IsValidInput(input))
			return false;
		return LastFrameState[input].Down;
	}

	InputProperties AllegroKeyboard::GetInputProperties(DeviceInputID input) const
	{
		if (!IsValidInput(input))
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

	ghassanpl::enum_flags<InputDeviceFlags> AllegroKeyboard::GetFlags() const
	{
		return ghassanpl::enum_flags<InputDeviceFlags>();
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

	std::string_view AllegroMouse::GetStringProperty(IInputDevice::StringProperty property) const
	{
		if (property == StringProperty::Name)
			return "Main Mouse";
		return "";
	}

	DeviceInputID AllegroMouse::GetMaxInput() const
	{
		return TotalInputs;
	}

	double AllegroMouse::GetInputState(DeviceInputID input) const
	{
		if (!IsValidInput(input))
			return 0.0;
		return CurrentState[input];
	}

	bool AllegroMouse::GetInputStateDigital(DeviceInputID input) const
	{
		if (!IsValidInput(input) || input >= ButtonCount)
			return false;
		return CurrentState[input] != 0;
	}

	double AllegroMouse::GetInputStateLastFrame(DeviceInputID input) const
	{
		if (!IsValidInput(input))
			return 0.0;
		return LastFrameState[input];
	}

	bool AllegroMouse::GetInputStateLastFrameDigital(DeviceInputID input) const
	{
		if (!IsValidInput(input) || input >= ButtonCount)
			return false;
		return LastFrameState[input] != 0;
	}

	InputProperties AllegroMouse::GetInputProperties(DeviceInputID input) const
	{
		switch (input)
		{
		case (int)MouseButton::Left:
		case (int)MouseButton::Right:
		case (int)MouseButton::Middle:
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

	ghassanpl::enum_flags<InputDeviceFlags> AllegroMouse::GetFlags() const
	{
		return ghassanpl::enum_flags<InputDeviceFlags>();
	}

	bool AllegroMouse::IsStringPropertyValid(StringProperty property) const
	{
		return false;
	}

	bool AllegroMouse::IsNumberPropertyValid(NumberProperty property) const
	{
		return false;
	}

	glm::vec3 AllegroMouse::GetNumberProperty(NumberProperty property) const
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


	std::string_view AllegroGamepad::GetStringProperty(IInputDevice::StringProperty property) const
	{
		if (property == StringProperty::Name)
			return mName;
		return "";
	}


	DeviceInputID AllegroGamepad::GetMaxInput() const
	{
		return mNumInputs;
	}

	double AllegroGamepad::GetInputState(DeviceInputID input) const
	{
		if (!IsValidInput(input)) return 0;
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

	bool AllegroGamepad::GetInputStateDigital(DeviceInputID input) const
	{
		return GetInputState(input) > 0.5;
	}

	double AllegroGamepad::GetInputStateLastFrame(DeviceInputID input) const
	{
		if (!IsValidInput(input)) return 0;
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

	bool AllegroGamepad::GetInputStateLastFrameDigital(DeviceInputID input) const
	{
		return GetInputStateLastFrame(input) > 0.5;
	}

	InputProperties AllegroGamepad::GetInputProperties(DeviceInputID input) const
	{
		if (!IsValidInput(input)) return {};
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

	uint8_t AllegroGamepad::GetStickCount() const
	{
		return (uint8_t)mSticks.size();
	}

	uint8_t AllegroGamepad::GetStickAxisCount(uint8_t stick_num) const
	{
		if (stick_num < mSticks.size())
			return mSticks[stick_num].NumAxes;
		return 0;
	}

	uint8_t AllegroGamepad::GetButtonCount() const
	{
		return (uint8_t)mButtons.size();
	}

	bool AllegroGamepad::GetButtonState(uint8_t button_num) const
	{
		if (button_num < mButtons.size())
			return CurrentState.Button[button_num] != 0;
		return false;
	}

	glm::vec3 AllegroGamepad::GetStickState(uint8_t stick_num) const
	{
		if (stick_num < mSticks.size())
			return { CurrentState.Stick[stick_num].Axis[0], CurrentState.Stick[stick_num].Axis[1], CurrentState.Stick[stick_num].Axis[2] };
		return {};
	}

	float AllegroGamepad::GetStickAxisState(uint8_t stick_num, uint8_t axis_num) const
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

	bool AllegroGamepad::GetButtonStateLastFrame(uint8_t button_num) const
	{
		if (button_num < mButtons.size())
			return LastFrameState.Button[button_num] != 0;
		return false;
	}

	glm::vec3 AllegroGamepad::GetStickStateLastFrame(uint8_t stick_num) const
	{
		if (stick_num < mSticks.size())
			return { LastFrameState.Stick[stick_num].Axis[0], LastFrameState.Stick[stick_num].Axis[1], LastFrameState.Stick[stick_num].Axis[2] };
		return {};
	}

	float AllegroGamepad::GetStickAxisStateLastFrame(uint8_t stick_num, uint8_t axis_num) const
	{
		if (stick_num < mSticks.size() && mSticks[stick_num].NumAxes < axis_num)
			return LastFrameState.Stick[stick_num].Axis[axis_num];
		return {};
	}

	ghassanpl::enum_flags<InputDeviceFlags> AllegroGamepad::GetFlags() const
	{
		return ghassanpl::enum_flags<InputDeviceFlags>();
	}

	bool AllegroGamepad::IsStringPropertyValid(StringProperty property) const
	{
		return false;
	}

	bool AllegroGamepad::IsNumberPropertyValid(NumberProperty property) const
	{
		return false;
	}

	glm::vec3 AllegroGamepad::GetNumberProperty(NumberProperty property) const
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
			static_cast<AllegroKeyboard*>(GetKeyboard())->KeyPressed(event.keyboard.keycode);
			SetLastActiveDevice(mKeyboard, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_KEY_CHAR:
			SetLastActiveDevice(mKeyboard, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_KEY_UP:
			static_cast<AllegroKeyboard*>(GetKeyboard())->KeyReleased(event.keyboard.keycode);
			SetLastActiveDevice(mKeyboard, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_MOUSE_AXES:
			static_cast<AllegroMouse*>(GetMouse())->MouseWheelScrolled(event.mouse.dz, event.mouse.dw);
			static_cast<AllegroMouse*>(GetMouse())->MouseMoved(event.mouse.x, event.mouse.y);
			SetLastActiveDevice(mMouse, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			static_cast<AllegroMouse*>(GetMouse())->MouseButtonPressed(MouseButton(event.mouse.button - 1));
			SetLastActiveDevice(mMouse, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			static_cast<AllegroMouse*>(GetMouse())->MouseButtonReleased(MouseButton(event.mouse.button - 1));
			SetLastActiveDevice(mMouse, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
			static_cast<AllegroMouse*>(GetMouse())->MouseEntered();
			SetLastActiveDevice(mMouse, event.any.timestamp);
			break;
		case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
			static_cast<AllegroMouse*>(GetMouse())->MouseLeft();
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
			if (GetLastDeviceActive() == mFirstGamepad)
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