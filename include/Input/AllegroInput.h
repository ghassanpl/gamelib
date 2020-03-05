#pragma once

#include "../Common.h"
#include "InputDevice.h"
#include "InputSystem.h"

struct ALLEGRO_JOYSTICK;

namespace gamelib
{
	struct AllegroInput : IInputSystem
	{
		using IInputSystem::IInputSystem;

		virtual void Init() override;

		void ProcessEvent(ALLEGRO_EVENT const& event);

		void RefreshJoysticks();
	};

	struct AllegroKeyboard : IKeyboardDevice
	{
		using IKeyboardDevice::IKeyboardDevice;

		virtual DeviceInputID GetMaxInput() const override;
		virtual double GetInputState(DeviceInputID input) const override;
		virtual bool GetInputStateDigital(DeviceInputID input) const override;
		virtual double GetInputStateLastFrame(DeviceInputID input) const override;
		virtual bool GetInputStateLastFrameDigital(DeviceInputID input) const override;
		virtual InputProperties GetInputProperties(DeviceInputID input) const override;
		virtual void ForceRefresh() override;
		virtual void NewFrame() override;
		virtual std::string_view GetStringProperty(StringProperty property) const override;
		virtual glm::vec3 GetNumberProperty(NumberProperty property) const override;

		// Inherited via IKeyboardDevice
		virtual void KeyPressed(DeviceInputID key);
		virtual void KeyReleased(DeviceInputID key);

		struct KeyState
		{
			bool Down = false;
			int RepeatCount = 0;
			seconds_t LastChangeTime = 0;
		};

		std::array<KeyState, (size_t)KeyboardKey::Max> CurrentState;
		std::array<KeyState, (size_t)KeyboardKey::Max> LastFrameState;

		// Inherited via IKeyboardDevice
		virtual enum_flags<InputDeviceFlags> GetFlags() const override;
		virtual bool IsStringPropertyValid(StringProperty property) const override;
		virtual bool IsNumberPropertyValid(NumberProperty property) const override;
	};

	struct AllegroMouse : IMouseDevice
	{
		using IMouseDevice::IMouseDevice;

		virtual DeviceInputID GetMaxInput() const override;
		virtual double GetInputState(DeviceInputID input) const override;
		virtual bool GetInputStateDigital(DeviceInputID input) const override;
		virtual double GetInputStateLastFrame(DeviceInputID input) const override;
		virtual bool GetInputStateLastFrameDigital(DeviceInputID input) const override;
		virtual InputProperties GetInputProperties(DeviceInputID input) const override;
		virtual void ForceRefresh() override;
		virtual void NewFrame() override;
		virtual bool IsConnected() const override;
		virtual enum_flags<InputDeviceFlags> GetFlags() const override;
		virtual bool IsStringPropertyValid(StringProperty property) const override;
		virtual std::string_view GetStringProperty(StringProperty property) const override;
		virtual bool IsNumberPropertyValid(NumberProperty property) const override;
		virtual glm::vec3 GetNumberProperty(NumberProperty property) const override;

		// Inherited via IMouseDevice

		virtual DeviceInputID GetVerticalWheelInput() const override { return Wheel0; }
		virtual DeviceInputID GetHorizontalWheelInput() const override { return Wheel1; }

		virtual DeviceInputID GetXAxisInput() const override { return XAxis; }
		virtual DeviceInputID GetYAxisInput() const override { return YAxis; }

		virtual void MouseWheelScrolled(float delta, unsigned wheel);
		virtual void MouseButtonPressed(MouseButton button);
		virtual void MouseButtonReleased(MouseButton button);
		virtual void MouseMoved(int x, int y);
		virtual void MouseEntered();
		virtual void MouseLeft();

		static constexpr uint64_t ButtonCount = 5;

		static constexpr uint64_t Wheel0 = ButtonCount + 0;
		static constexpr uint64_t Wheel1 = ButtonCount + 1;
		static constexpr uint64_t XAxis = ButtonCount + 2;
		static constexpr uint64_t YAxis = ButtonCount + 3;

	private:

		static constexpr uint64_t TotalInputs = ButtonCount + 2 /* wheels */ + 2 /* axes */;

		std::array<double, TotalInputs> CurrentState{};
		std::array<double, TotalInputs> LastFrameState{};
	};

	struct AllegroGamepad : IXboxGamepadDevice
	{
		AllegroGamepad(IInputSystem& sys, ALLEGRO_JOYSTICK* stick);

		virtual DeviceInputID GetMaxInput() const override;
		virtual double GetInputState(DeviceInputID input) const override;
		virtual bool GetInputStateDigital(DeviceInputID input) const override;
		virtual double GetInputStateLastFrame(DeviceInputID input) const override;
		virtual bool GetInputStateLastFrameDigital(DeviceInputID input) const override;
		virtual InputProperties GetInputProperties(DeviceInputID input) const override;
		virtual void ForceRefresh() override;
		virtual void NewFrame() override;
		virtual bool IsConnected() const override;
		virtual enum_flags<InputDeviceFlags> GetFlags() const override;
		virtual bool IsStringPropertyValid(StringProperty property) const override;
		virtual std::string_view GetStringProperty(StringProperty property) const override;
		virtual bool IsNumberPropertyValid(NumberProperty property) const override;
		virtual glm::vec3 GetNumberProperty(NumberProperty property) const override;

		// Inherited via IGamepadDevice
		virtual uint8_t GetStickCount() const override;
		virtual uint8_t GetStickAxisCount(uint8_t stick_num) const override;
		virtual uint8_t GetButtonCount() const override;

		virtual bool GetButtonState(uint8_t button_num) const override;
		virtual glm::vec3 GetStickState(uint8_t stick_num) const override;
		virtual float GetStickAxisState(uint8_t stick_num, uint8_t axis_num) const override;

		virtual bool GetButtonStateLastFrame(uint8_t button_num) const override;
		virtual glm::vec3 GetStickStateLastFrame(uint8_t stick_num) const override;
		virtual float GetStickAxisStateLastFrame(uint8_t stick_num, uint8_t axis_num) const override;

		struct JoystickState
		{
			struct {
				float Axis[3] = { 0,0,0 };
			} Stick[16] = {};
			int Button[32] = {};
		};

		JoystickState CurrentState{};
		JoystickState LastFrameState{};

	private:

		struct JoystickStick
		{
			const char* Name = "";
			uint8_t NumAxes = 0;
			std::array<InputProperties, 3> Axes;
		};

		const char* mName = "Generic Gamepad";
		std::vector<JoystickStick> mSticks;
		std::vector<InputProperties> mButtons;
		uint8_t mNumInputs = 0;
		uint8_t mNumAxes = 0;

		std::pair<uint8_t, uint8_t> CalculateStickAndAxis(DeviceInputID input) const;

		ALLEGRO_JOYSTICK* mJoystick = nullptr;



		// Inherited via IGamepadDevice
	};
}