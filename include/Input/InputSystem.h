#pragma once

#include <compare>
#include <map>

#include "InputDevice.h"
#include "../ErrorReporter.h"

union ALLEGRO_EVENT;

namespace gamelib
{
	using InputID = int;
	inline constexpr InputID InvalidInput = 0x5F5F5F5F;

	struct IInputSystem
	{
		IErrorReporter& ErrorReporter;

		IInputSystem(IErrorReporter& errep) noexcept : ErrorReporter(errep) {}
		virtual ~IInputSystem() = default;

		virtual void Init();

		virtual void Update();

		using InputDeviceIndex = size_t;

		static constexpr InputDeviceIndex KeyboardDeviceID = 0;
		static constexpr InputDeviceIndex MouseDeviceID = 1;
		static constexpr InputDeviceIndex FirstGamepadDeviceID = 2;

		IKeyboardDevice* GetKeyboard() const { return mKeyboard; }
		IMouseDevice* GetMouse() const { return mMouse; }
		IGamepadDevice* GetFirstGamepad() const { return mFirstGamepad; }

		struct Input
		{
			InputID ActionID = InvalidInput;
			PlayerID Player = {};

			Input() = default;
			Input(InputID id, PlayerID player = {}) : ActionID(id), Player(player) {}
			template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
			Input(T id, PlayerID player = {}) : ActionID((InputID)id), Player(player) {}

#ifndef __clang__
			auto operator<=>(Input const& other) const noexcept = default;
#endif
		};

		void MapKey(KeyboardKey key, InputID to_input) { MapButton((DeviceInputID)key, KeyboardDeviceID, to_input); }
		void MapMouse(MouseButton mouse_button, InputID to_input) { MapButton((DeviceInputID)mouse_button, MouseDeviceID, to_input); }
		void MapGamepad(XboxGamepadButton pad_button, InputID to_input) { MapButton((DeviceInputID)pad_button, FirstGamepadDeviceID, to_input); }
		void MapNavigation(UINavigationInput ui_input, InputID to_input);

		void MapKeyAndButton(InputID to_input, KeyboardKey key, XboxGamepadButton pad_button)
		{
			MapKey(key, to_input);
			MapGamepad(pad_button, to_input);
		}

		void MapButton(DeviceInputID physical_button, InputDeviceIndex of_device, Input to_input);
		void MapAxis1D(DeviceInputID physical_axis, InputDeviceIndex of_device, Input to_input);
		void MapAxis2D(DeviceInputID physical_axis1, DeviceInputID physical_axis2, InputDeviceIndex of_device, Input to_input);

		void MapButtonToAxis(DeviceInputID physical_button, InputDeviceIndex of_device, double to_axis_value, Input of_input);

		bool IsButtonPressed(InputID input_id);
		bool IsButtonPressed(MouseButton but);
		bool IsKeyPressed(KeyboardKey key);

		bool WasButtonPressed(InputID input_id);
		bool WasButtonPressed(MouseButton but);
		bool WasKeyPressed(KeyboardKey key);

		bool WasButtonReleased(Input input_id);
		int  ButtonRepeatCount(Input input_id);

		bool IsNavigationPressed(UINavigationInput input_id);
		bool WasNavigationPressed(UINavigationInput input_id);
		bool WasNavigationReleased(UINavigationInput input_id);
		int  NavigationRepeatCount(UINavigationInput input_id);

		float GetAxis(Input of_input);
		vec2 GetAxis2D(Input of_input);

		/// TODO: Input Command callbacks (Down, Up, Press, Hold, etc)

		IInputDevice* GetLastDeviceActive() const { return mLastActiveDevice; }

		std::string GetButtonNamesForInput(Input input, std::string_view button_format);
		std::string GetButtonNameForInput(Input input, std::string_view button_format);

		std::string GetButtonNamesForInput(Input input)
		{
			return GetButtonNamesForInput(input, "{}");
		}

		std::string GetButtonNameForInput(Input input)
		{
			return GetButtonNameForInput(input, "{}");
		}

	protected:

		struct Mapping
		{
			InputDeviceIndex DeviceID = 0;
			DeviceInputID Inputs[2] = { 0, InvalidDeviceInputID };
		};

		IKeyboardDevice* mKeyboard = nullptr;
		IMouseDevice* mMouse = nullptr;
		IGamepadDevice* mFirstGamepad = nullptr;

		IInputDevice* mLastActiveDevice = nullptr;
		void SetLastActiveDevice(IInputDevice* device, seconds_t current_time);

		std::vector<std::unique_ptr<IInputDevice>> mInputDevices;
		std::map<void*, IGamepadDevice*> mJoystickMap;

		IInputDevice* GetInputDevice(InputDeviceIndex id);
		std::string GetInputDeviceName(InputDeviceIndex id);

		struct PlayerInformation
		{
			PlayerID ID = {};
			std::map<int, std::vector<Mapping>> Mappings;
			std::vector<InputDeviceIndex> BoundDeviceIDs;
		};

		PlayerInformation* GetPlayer(PlayerID id);

		std::map<PlayerID, PlayerInformation> mPlayers;

		void DebugInput();
	};
}