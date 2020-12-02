#pragma once

#include <compare>
#include <map>
#include <any>
#include <concepts>

#include "InputDevice.h"
#include "../ErrorReporter.h"

union ALLEGRO_EVENT;

namespace gamelib
{
	using InputID = std::string;
	inline static const InputID InvalidInput = {};

	struct IDebugger;

	struct IInputSystem
	{
		std::shared_ptr<IErrorReporter> ErrorReporter;
		std::shared_ptr<IDebugger> Debugger;

		IInputSystem(std::shared_ptr<IErrorReporter> error_reporter, std::shared_ptr<IDebugger> debugger) noexcept;
		virtual ~IInputSystem() = default;

		virtual void Init();

		virtual void Update();

		virtual void Debug();

		using InputDeviceIndex = size_t;

		static constexpr InputDeviceIndex KeyboardDeviceID = 0;
		static constexpr InputDeviceIndex MouseDeviceID = 1;
		static constexpr InputDeviceIndex FirstGamepadDeviceID = 2;

		/// TODO: Helpers/devices for:
		/// - Accelerometer (with subdevices)
		///		- Acceleration
		///		- Gravity
		///		- Gyro/Orientation
		/// - Compass 
		/// - Environment
		///		- Temperature, Light, Magnetic field, Pressure, Humidity
		/// - Proximity
		/// - Touchpad
		/// - GPS
		/// - Steps
		/// - Altitude
		/// 
		/// TODO: Maybe add a GenericDeviceType enum? { Keyboard, Mouse, Gamepad, Accelerometer, Compass, EnvironmentSensor, GPS, ... }

		IKeyboardDevice* Keyboard() const { return dynamic_cast<IKeyboardDevice*>(mInputDevices[KeyboardDeviceID].get()); }
		IMouseDevice* Mouse() const { return dynamic_cast<IMouseDevice*>(mInputDevices[MouseDeviceID].get());; }
		IGamepadDevice* FirstGamepad() const { return dynamic_cast<IGamepadDevice*>(mInputDevices[FirstGamepadDeviceID].get());; }

		std::vector<IInputDevice*> AllInputDevices() const;

		struct Input
		{
			InputID ActionID = InvalidInput;
			PlayerID Player = InvalidPlayerID;

			Input() = default;
			
			template <typename... ARGS>
			Input(ARGS&&... args) 
			requires std::is_constructible_v<InputID, ARGS...> 
				: ActionID(std::forward<ARGS>(args)...)
			{
			}

			template <typename... ARGS>
			Input(PlayerID player, ARGS&&... args) 
			requires std::is_constructible_v<InputID, ARGS...> 
				: Input(std::forward<ARGS>(args)...), Player(player)
			{
			}
			
			template <typename... ARGS>
			Input(void const* player, ARGS&&... args) 
			requires std::is_constructible_v<InputID, ARGS...> 
				: Input(std::forward<ARGS>(args)..., reinterpret_cast<PlayerID>(player))
			{
			}

#ifndef __clang__
			auto operator<=>(Input const& other) const noexcept = default;
#endif
		};

		void MapKey(KeyboardKey key, Input to_input) { MapButton((DeviceInputID)key, KeyboardDeviceID, to_input); }
		void MapMouse(MouseButton mouse_button, Input to_input) { MapButton((DeviceInputID)mouse_button, MouseDeviceID, to_input); }
		void MapGamepad(XboxGamepadButton pad_button, Input to_input) { MapButton((DeviceInputID)pad_button, FirstGamepadDeviceID, to_input); }
		void MapNavigation(UINavigationInput ui_input, Input to_input);
		
		/// TODO: void BindButtonPressed(Input button, func callback); /// maybe Bind* functions should return RegisteredCallbackID ?
		/// TODO: void BindButtonReleased(Input button, func callback);
		/// TODO: void BindNavigationPressed(UINavigationInput input, func callback);
		/// TODO: void BindNavigationReleased(UINavigationInput input, func callback);
		/// TODO: void BindGamepadConnectionEvent(func callback);
		/// TODO: void BindDeviceStatusEvent(func callback);
		/// TODO: void BindInputChanged(Input input, func callback);
		/// TODO: void BindInputPropertiesChanged(Input input, func callback);/// 
		/// TODO: void UnbindCallback(RegisteredCallbackID id);
		/// 
		/// struct SequenceElement { InputID input; seconds_t time; };
		/// TODO: void MapSequence(std::span<SequenceElement> elements, InputID to_input);
		
		/// TODO: Unmap(???) /// Maybe Map* functions should return a MappingID ?
		
		/// TODO: Screen buttons/joysticks
		
		void MapKeyAndButton(Input to_input, KeyboardKey key, XboxGamepadButton pad_button)
		{
			MapKey(key, to_input);
			MapGamepad(pad_button, to_input);
		}

		void MapButton(DeviceInputID physical_button, InputDeviceIndex of_device, Input to_input);
		void MapAxis1D(DeviceInputID physical_axis, InputDeviceIndex of_device, Input to_input);
		void MapAxis2D(DeviceInputID physical_axis1, DeviceInputID physical_axis2, InputDeviceIndex of_device, Input to_input);
		
		/// Data events: voice command, hand/body shape/gesture; maybe should be called "Match" or "Pattern" events?
		//void MapDataEvent(...);

		void MapButtonToAxis(DeviceInputID physical_button, InputDeviceIndex of_device, double to_pressed_value, double and_released_value, Input of_input);
		/// mapPhysicalButton:ofDevice:toPressedValue:andReleasedValue:ofAxisInput:

		bool IsButtonPressed(Input input_id);
		bool IsButtonPressed(MouseButton but);
		bool IsKeyPressed(KeyboardKey key);

		bool WasButtonPressed(Input input_id);
		bool WasButtonPressed(MouseButton but);
		bool WasKeyPressed(KeyboardKey key);

		bool WasButtonReleased(Input input_id);
		bool WasButtonReleased(MouseButton but);
		bool WasKeyReleased(KeyboardKey key);

		int  ButtonRepeatCount(Input input_id);

		bool IsNavigationPressed(UINavigationInput input_id);
		bool WasNavigationPressed(UINavigationInput input_id);
		bool WasNavigationReleased(UINavigationInput input_id);
		int  NavigationRepeatCount(UINavigationInput input_id);

		float AxisValue(Input of_input);
		vec2 Axis2DValue(Input of_input);

		void ResetInput(Input input);
		seconds_t InputPressedTime(Input input);

		virtual vec2 MousePosition() const;

		/// TODO: Input Command callbacks (Down, Up, Press, Hold, etc)

		IInputDevice* LastDeviceActive() const { return mLastActiveDevice; }

		std::string ButtonNamesForInput(Input input, std::string_view button_format);
		std::string ButtonNameForInput(Input input, std::string_view button_format);

		std::string ButtonNamesForInput(Input input)
		{
			return ButtonNamesForInput(input, "{}");
		}

		std::string ButtonNameForInput(Input input)
		{
			return ButtonNameForInput(input, "{}");
		}

	protected:

		struct Mapping
		{
			InputDeviceIndex DeviceID = 0;
			DeviceInputID Inputs[2] = { 0, InvalidDeviceInputID };
		};

		IInputDevice* mLastActiveDevice = nullptr;
		void SetLastActiveDevice(IInputDevice* device, seconds_t current_time);

		std::vector<std::unique_ptr<IInputDevice>> mInputDevices;
		std::map<void*, IGamepadDevice*> mJoystickMap;

		IInputDevice* InputDevice(InputDeviceIndex id);
		std::string InputDeviceName(InputDeviceIndex id);

		struct PlayerInformation
		{
			std::map<InputID, std::vector<Mapping>> Mappings;
			std::vector<InputDeviceIndex> BoundDeviceIDs;
		};

		PlayerInformation* GetPlayer(PlayerID id);

		std::map<PlayerID, PlayerInformation> mPlayers;

		void DebugInput();
	};

	template <typename USER_DATA>
	struct InputInformation : USER_DATA
	{
		InputID ID{};
	};

	/// A utility map that maps input ids to any additional information you may wish to store
	template <typename USER_DATA>
	using InputInfos = std::map<InputID, InputInformation<USER_DATA>, std::less<>>;
}