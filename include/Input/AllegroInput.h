#pragma once

#include "../Common.h"
#include "InputDevice.h"
#include "InputSystem.h"
#include <array>

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

		virtual DeviceInputID MaxInputID() const override;
		virtual double InputValue(DeviceInputID input) const override;
		virtual bool IsInputPressed(DeviceInputID input) const override;
		virtual double InputValueLastFrame(DeviceInputID input) const override;
		virtual bool WasInputPressedLastFrame(DeviceInputID input) const override;
		virtual InputProperties PropertiesOf(DeviceInputID input) const override;
		virtual void ForceRefresh() override;
		virtual void NewFrame() override;
		virtual std::string_view StringPropertyValue(StringProperty property) const override;
		virtual glm::vec3 NumberPropertyValue(NumberProperty property) const override;

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
		virtual enum_flags<InputDeviceFlags> Flags() const override;
		virtual bool IsStringPropertyValid(StringProperty property) const override;
		virtual bool IsNumberPropertyValid(NumberProperty property) const override;
	};

	struct AllegroMouse : IMouseDevice
	{
		using IMouseDevice::IMouseDevice;

		virtual DeviceInputID MaxInputID() const override;
		virtual double InputValue(DeviceInputID input) const override;
		virtual bool IsInputPressed(DeviceInputID input) const override;
		virtual double InputValueLastFrame(DeviceInputID input) const override;
		virtual bool WasInputPressedLastFrame(DeviceInputID input) const override;
		virtual InputProperties PropertiesOf(DeviceInputID input) const override;
		virtual void ForceRefresh() override;
		virtual void NewFrame() override;
		virtual bool IsConnected() const override;
		virtual enum_flags<InputDeviceFlags> Flags() const override;
		virtual bool IsStringPropertyValid(StringProperty property) const override;
		virtual std::string_view StringPropertyValue(StringProperty property) const override;
		virtual bool IsNumberPropertyValid(NumberProperty property) const override;
		virtual glm::vec3 NumberPropertyValue(NumberProperty property) const override;

		// Inherited via IMouseDevice

		virtual DeviceInputID VerticalWheelInputID() const override { return Wheel0; }
		virtual DeviceInputID HorizontalWheelInputID() const override { return Wheel1; }

		virtual DeviceInputID XAxisInputID() const override { return XAxis; }
		virtual DeviceInputID YAxisInputID() const override { return YAxis; }

		virtual void ShowCursor(bool show) override;
		virtual bool IsCursorVisible() const override;
		virtual bool IsCursorShapeAvailable(MouseCursorShape shape) const override;
		virtual void SetCursorShape(MouseCursorShape shape) override;
		virtual bool IsCustomCursorShapeAvailable(std::string_view shape) const override;
		virtual void SetCustomCursorShape(std::string_view name) override;
		virtual void SetCustomCursorShape(void* resource, MouseCursorShape replace = {}, vec2 hotspot = {}) override;
		virtual bool CanWarp() const override;
		virtual void Warp(vec2 pos) override;


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
		bool mCursorVisible = true;
	};

	struct AllegroGamepad : IXboxGamepadDevice
	{
		AllegroGamepad(IInputSystem& sys, ALLEGRO_JOYSTICK* stick);

		virtual DeviceInputID MaxInputID() const override;
		virtual double InputValue(DeviceInputID input) const override;
		virtual bool IsInputPressed(DeviceInputID input) const override;
		virtual double InputValueLastFrame(DeviceInputID input) const override;
		virtual bool WasInputPressedLastFrame(DeviceInputID input) const override;
		virtual InputProperties PropertiesOf(DeviceInputID input) const override;
		virtual void ForceRefresh() override;
		virtual void NewFrame() override;
		virtual bool IsConnected() const override;
		virtual enum_flags<InputDeviceFlags> Flags() const override;
		virtual bool IsStringPropertyValid(StringProperty property) const override;
		virtual std::string_view StringPropertyValue(StringProperty property) const override;
		virtual bool IsNumberPropertyValid(NumberProperty property) const override;
		virtual glm::vec3 NumberPropertyValue(NumberProperty property) const override;

		// Inherited via IGamepadDevice
		virtual uint8_t StickCount() const override;
		virtual uint8_t StickAxisCount(uint8_t stick_num) const override;
		virtual uint8_t ButtonCount() const override;

		virtual bool IsButtonPressed(uint8_t button_num) const override;
		virtual glm::vec3 StickValue(uint8_t stick_num) const override;
		virtual float StickAxisValue(uint8_t stick_num, uint8_t axis_num) const override;

		virtual bool WasButtonPressedLastFrame(uint8_t button_num) const override;
		virtual glm::vec3 StickValueLastFrame(uint8_t stick_num) const override;
		virtual float StickAxisValueLastFrame(uint8_t stick_num, uint8_t axis_num) const override;

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