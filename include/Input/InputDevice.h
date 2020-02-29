#pragma once

#include "../Common.h"
#include "../Includes/EnumFlags.h"
#include "../Includes/GLM.h"
#include <gsl/span>
#include <vector>
#include <string>

namespace gamelib
{
	using DeviceInputID = uint32_t;
	static constexpr inline DeviceInputID InvalidDeviceInputID = std::numeric_limits<DeviceInputID>::max();
	using DeviceOutputID = uint32_t;
	static constexpr inline DeviceOutputID InvalidDeviceOutputID = std::numeric_limits<DeviceOutputID>::max();

	using PlayerID = uintptr_t;

	struct IInputSystem;

	enum class KeyboardKey
	{
		None,

		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9,
		NumPad0, NumPad1, NumPad2, NumPad3, NumPad4, NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		Escape, Tilde, Minus, Equals, Backspace, Tab, OpenBrace, CloseBrace, Enter, Semicolon, Quote, Backslash, Backslash2, Comma,
		FullStop, Slash, Space, Insert, Delete, Home, End, PgUp, PgDn, Left, Right, Up, Down,

		NumPadSlash, NumPadAsterisk, NumPadMinus, NumPadPlus, NumPadDelete, NumPadEnter, Printscreen, Pause,
		Abnt_C1, Yen, Kana, Convert, Noconvert, At, Circumflex, Colon2, Kanji,
		NumPadEquals, Backquote, Semicolon2, Command, Back, VolumeUp, VolumeDown, Search,

		DPadCenter, ButtonX, ButtonY,
		DPadUp, DPadDown, DPadLeft, DPadRight, Select, Start,
		ButtonL1, ButtonR1, ButtonL2, ButtonR2, ButtonA, ButtonB, Thumbl, Thumbr, Unknown,

		Max = 227
	};

	enum class MouseButton
	{
		Left,
		Right,
		Middle,
		Button4,
		Button5
	};

	enum class InputFlags
	{
		Digital,
		ReturnsToNeutral,
		HasDeadzone,
		/// If axis, whether this axis influences or is influenced by another axis (e.g. between two axis on a pad stick, or between the absolute position of the mouse, and the delta)
		/// If button, whether this button cannot be pressed with/without another button
		Correlated,
		/// If axis, whether this axis is emulated from buttons
		/// If button, whether this button is emulated from an axis
		Emulated,
		/// Whether this button can repeat its 'pressed' events
		CanRepeat,
	};

	enum class InputDeviceFlags
	{
		Composite, /// e.g. Mouse+Keyboard combo, or two paired joy-cons
		Wireless,
		CanBeReset,
		CanBeDisabled,
	};

	enum class InputDeviceOutputType
	{
		Flag, /// E.g. for LEDs on keyboards
		Value, /// E.g. for progress bars, odometers, number displays, etc.
		Color,
		Vibration,
		Image,
		Video,
		Sound,
		Text,
		PersistentData, /// E.g. for saves
		Other
	};

	struct InputProperties
	{
		std::string Name;
		std::string ShortName;
		ghassanpl::enum_flags<InputFlags> Flags;

		double PressedThreshold = 0.5;

		double DeadZoneMin = -0.1;
		double DeadZoneMax = 0.1;

		double MinValue = -1;
		double NeutralValue = 0;
		double MaxValue = 1;
		double StepSize = 0;

		std::string GlyphURL;

		glm::vec3 PhysicalPositionOnDevice = {};
		glm::vec3 PositionOnDevicePreview = {}; /// On image or model
	};

	struct OutputProperties
	{
		std::string Name;
		std::string ShortName;

		InputDeviceOutputType Type = InputDeviceOutputType::Other;

		glm::vec3 Resolution = { 1,1,1 };

		glm::vec3 PhysicalPositionOnDevice = {};
		glm::vec3 PositionOnDevicePreview = {}; /// On image or model
	};

	/// TODO
	enum class InputDataType
	{
		Audio,
		Video,
		PersistentData,
		Other
	};

	/// TODO
	struct InputDataProperties
	{
		std::string Name;
		InputDataType Type = InputDataType::Other;
		glm::vec3 Resolution = { 1,1,1 };
	};

	enum class UINavigationInput
	{
		Accept, Cancel, Menu, View,
		Left, Right, Up, Down,
		Home, End, Back, Forward,
		PageUp, PageDown, PageLeft, PageRight,
		ScrollUp, ScrollDown, ScrollLeft, ScrollRight,
	};

	vec2 UINavigationInputToDirection(UINavigationInput input);

	struct IInputDevice
	{
		virtual ~IInputDevice() = default;

		IInputSystem& ParentSystem;
		IInputDevice(IInputSystem& sys) : ParentSystem(sys) {}

		seconds_t GetLastActiveTime() const { return mLastActiveTime; }
		void SetLastActiveTime(seconds_t time) { mLastActiveTime = time; }

		virtual std::string_view GetName() const;
		virtual ghassanpl::enum_flags<InputDeviceFlags> GetFlags() const = 0;

		virtual DeviceInputID GetMaxInput() const = 0;
		virtual bool IsValidInput(DeviceInputID input) const { return input < GetMaxInput(); }
		virtual double GetInputState(DeviceInputID input) const = 0;
		virtual bool GetInputStateDigital(DeviceInputID input) const = 0;
		virtual double GetInputStateLastFrame(DeviceInputID input) const = 0;
		virtual bool GetInputStateLastFrameDigital(DeviceInputID input) const = 0;

		virtual InputProperties GetInputProperties(DeviceInputID input) const = 0;

		virtual bool IsValidNavigation(UINavigationInput input) const = 0;
		virtual bool GetNavigation(UINavigationInput input) const = 0;
		virtual bool GetNavigationLastFrame(UINavigationInput input) const = 0;

		virtual bool IsConnected() const { return true; }

		virtual size_t GetSubDeviceCount() const { return 0; }
		virtual IInputDevice* GetSubDevice(size_t index) const { return nullptr; }

		virtual DeviceOutputID GetMaxOutput() const { return {}; }
		virtual bool IsValidOutput(DeviceOutputID input) const { return input < GetMaxOutput(); }
		virtual OutputProperties GetOutputProperties(DeviceOutputID input) const;
		virtual void SetOutput(DeviceOutputID index, glm::vec3 value) {}
		virtual void ResetOutput(DeviceOutputID index) {}
		virtual void SendOutputData(DeviceOutputID index, gsl::span<const uint8_t> data) {}
		virtual void EnableOutput(DeviceOutputID index, bool enable) {}

		/// TODO: Data
		// virtual size_t DataCount() const = 0;
		// virtual bool DataMeaningful(size_t index) const= 0;
		// virtual InputDataProperties const& DataProperties(size_t index) const = 0;
		// virtual std::vector<uint8_t> ReadData(size_t index, size_t max_data) = 0;

		enum class PowerSource
		{
			None,
			Wire,
			Batteries,
			InternalAccu,
			ExternalAccu,
			Unknown
		};

		virtual PowerSource GetPowerSource() const { return IInputDevice::PowerSource::Unknown; }
		virtual ghassanpl::enum_flags<PowerSource> AvailablePowerSources() const { return ghassanpl::enum_flags<PowerSource>{}; }

		enum class StringProperty
		{
			Name,
			Company,
			ImageURL,
			Website,
			SerialNumber,
		};

		virtual bool IsStringPropertyValid(StringProperty property) const = 0;
		virtual std::string_view GetStringProperty(StringProperty property) const = 0;

		enum class NumberProperty
		{
			PhysicalSize, /// vec3, in centimeters
			AutoOffTime, /// float, in seconds
			Range, /// float, in cm
			Charge, /// float, 0 - 1
		};

		virtual bool IsNumberPropertyValid(NumberProperty property) const = 0;
		virtual glm::vec3 GetNumberProperty(NumberProperty property) const = 0;

		virtual void ForceRefresh() = 0;
		virtual void NewFrame() = 0;

		virtual void ResetDevice() {}
		virtual void EnableDevice(bool enable) {}

		virtual PlayerID AssociatedPlayer() const { return mAssociatedPlayer; }
		virtual bool AssociatePlayer(PlayerID player) { mAssociatedPlayer = player; return true; }

	protected:

		void ReportInvalidInput(DeviceInputID input) const;

	private:

		seconds_t mLastActiveTime = {};
		PlayerID mAssociatedPlayer = {};

	};

	struct IKeyboardDevice : virtual IInputDevice
	{
		using IInputDevice::IInputDevice;

		virtual bool IsValidNavigation(UINavigationInput input) const override;
		virtual bool GetNavigation(UINavigationInput input) const override;
		virtual bool GetNavigationLastFrame(UINavigationInput input) const override;
	};

	struct IMouseDevice : virtual IInputDevice
	{
		using IInputDevice::IInputDevice;

		virtual DeviceInputID GetVerticalWheelInput() const = 0;
		virtual DeviceInputID GetHorizontalWheelInput() const = 0;

		virtual DeviceInputID GetXAxisInput() const = 0;
		virtual DeviceInputID GetYAxisInput() const = 0;

		virtual bool IsValidNavigation(UINavigationInput input) const override;
		virtual bool GetNavigation(UINavigationInput input) const override;
		virtual bool GetNavigationLastFrame(UINavigationInput input) const override;
	};

	struct IGamepadDevice : virtual IInputDevice
	{
		using IInputDevice::IInputDevice;

		virtual uint8_t GetStickCount() const = 0;
		virtual uint8_t GetStickAxisCount(uint8_t stick_num) const = 0;
		virtual uint8_t GetButtonCount() const = 0;

		virtual bool GetButtonState(uint8_t button_num) const = 0;
		virtual glm::vec3 GetStickState(uint8_t stick_num) const = 0;
		virtual float GetStickAxisState(uint8_t stick_num, uint8_t axis_num) const = 0;

		virtual bool GetButtonStateLastFrame(uint8_t button_num) const = 0;
		virtual glm::vec3 GetStickStateLastFrame(uint8_t stick_num) const = 0;
		virtual float GetStickAxisStateLastFrame(uint8_t stick_num, uint8_t axis_num) const = 0;
	};

	enum class XboxGamepadButton
	{
		A, B, X, Y,
		RightBumper,
		LeftBumper,
		RightStickButton,
		LeftStickButton,
		Back,
		Start,
		Right, Left, Down, Up,
	};

	struct IXboxGamepadDevice : virtual IGamepadDevice
	{
		using IGamepadDevice::IGamepadDevice;

		virtual DeviceInputID GetMaxInput() const override;

		virtual InputProperties GetInputProperties(DeviceInputID input) const override;

		virtual uint8_t GetStickCount() const override;
		virtual uint8_t GetStickAxisCount(uint8_t stick_num) const override;
		virtual uint8_t GetButtonCount() const override;

		virtual bool IsValidNavigation(UINavigationInput input) const override;
		virtual bool GetNavigation(UINavigationInput input) const override;
		virtual bool GetNavigationLastFrame(UINavigationInput input) const override;

		static constexpr uint64_t ButtonCount = 14;

		enum class Axes
		{
			LeftStick_XAxis = ButtonCount,
			LeftStick_YAxis,
			RightStick_XAxis,
			RightStick_YAxis,
			LeftTrigger,
			RightTrigger,
		};
	};

}