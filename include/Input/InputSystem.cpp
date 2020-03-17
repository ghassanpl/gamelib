#include "InputSystem.h"
#include "../Debugger.h"

namespace gamelib
{
	void IInputSystem::SetLastActiveDevice(IInputDevice* device, seconds_t current_time)
	{
		if (device)
			device->SetLastActiveTime(current_time);
		mLastActiveDevice = device;
	}

	void IInputSystem::Init()
	{
		SetLastActiveDevice(mKeyboard, 0);
	}

	void IInputSystem::Update()
	{
		for (auto& device : mInputDevices)
			device->NewFrame();
	}


	void IInputSystem::MapButton(DeviceInputID physical_button, InputDeviceIndex of_device, Input to_input)
	{
		//if (of_device >= mInputDevices.size())
			//Game->Warning("Input device index {} does not represent a connected device", of_device);
		mPlayers[to_input.Player].Mappings[to_input.ActionID].push_back(Mapping{ of_device, {physical_button, InvalidDeviceInputID} });
	}

	bool IInputSystem::IsButtonPressed(InputID input_id)
	{
		if (mPlayers.empty()) return false;

		auto& player = mPlayers.begin()->second;
		if (auto it = player.Mappings.find(input_id); it != player.Mappings.end())
		{
			for (auto& mapping : it->second)
			{
				if (auto device = GetInputDevice(mapping.DeviceID))
				{
					if (device->GetInputStateDigital(mapping.Inputs[0]))
						return true;
				}
			}
		}
		return false;
	}

	bool IInputSystem::IsButtonPressed(MouseButton but)
	{
		return GetMouse()->GetInputStateDigital((DeviceInputID)but);
	}

	bool IInputSystem::WasButtonPressed(InputID input_id)
	{
		if (mPlayers.empty()) return false;

		auto& player = mPlayers.begin()->second;
		if (auto it = player.Mappings.find(input_id); it != player.Mappings.end())
		{
			for (auto& mapping : it->second)
			{
				if (auto device = GetInputDevice(mapping.DeviceID))
				{
					if (device->GetInputStateDigital(mapping.Inputs[0]) && !device->GetInputStateLastFrameDigital(mapping.Inputs[0]))
						return true;
				}
			}
		}
		return false;
	}

	bool IInputSystem::WasButtonPressed(MouseButton but)
	{
		return GetMouse()->GetInputStateDigital((DeviceInputID)but) && !GetMouse()->GetInputStateLastFrameDigital((DeviceInputID)but);
	}
	
	float IInputSystem::GetAxis(Input of_input)
	{
		auto player = GetPlayer(of_input.Player);
		if (!player)
		{
			ErrorReporter.Warning("Player not found for input")
				.Value("PlayerID", of_input.Player)
				.Value("ActionID", of_input.ActionID);
			return {};
		}

		if (auto it = player->Mappings.find(of_input.ActionID); it != player->Mappings.end())
		{
			for (auto& mapping : it->second)
			{
				if (auto device = GetInputDevice(mapping.DeviceID))
				{
					return (float)device->GetInputState(mapping.Inputs[0]);
				}
			}
		}
		return {};
	}

	vec2 IInputSystem::GetAxis2D(Input of_input)
	{
		auto player = GetPlayer(of_input.Player);
		if (!player)
		{
			ErrorReporter.Warning("Player not found for input")
				.Value("PlayerID", of_input.Player)
				.Value("ActionID", of_input.ActionID);
			return {};
		}

		if (auto it = player->Mappings.find(of_input.ActionID); it != player->Mappings.end())
		{
			for (auto& mapping : it->second)
			{
				if (auto device = GetInputDevice(mapping.DeviceID))
				{
					return { (float)device->GetInputState(mapping.Inputs[0]), (float)device->GetInputState(mapping.Inputs[1]) };
				}
			}
		}
		return {};
	}

	vec2 IInputSystem::GetMousePosition() const
	{
		return { (float)GetMouse()->GetInputState(GetMouse()->GetXAxisInput()), (float)GetMouse()->GetInputState(GetMouse()->GetYAxisInput()) };
	}

	IInputDevice* IInputSystem::GetInputDevice(InputDeviceIndex id)
	{
		if (id < mInputDevices.size())
			return mInputDevices[id].get();
		return nullptr;
	}

	std::string IInputSystem::GetInputDeviceName(InputDeviceIndex id)
	{
		if (auto dev = GetInputDevice(id))
			return (std::string)dev->GetName();
		else
			return fmt::format("Disconnected Device {}", id); /// TODO: Cache device names so we can add ("(Previously {})", OldDeviceName)
	}

	IInputSystem::PlayerInformation* IInputSystem::GetPlayer(PlayerID id)
	{
		auto player_it = mPlayers.find(id);
		if (player_it == mPlayers.end())
			return {};
		return &player_it->second;
	}

	std::string IInputSystem::GetButtonNamesForInput(Input button, std::string_view button_format)
	{
		std::string buttons;
		for (auto& mapping : GetPlayer(button.Player)->Mappings[button.ActionID])
		{
			if (auto device = GetInputDevice(mapping.DeviceID))
			{
				auto props = device->GetInputProperties(mapping.Inputs[0]);
				if (!buttons.empty()) buttons += ", ";
				buttons += fmt::format(button_format, props.Name);
			}
			else
			{
				if (!buttons.empty()) buttons += ", ";
				buttons += fmt::format(button_format, fmt::format("#{}@{}#", mapping.Inputs[0], mapping.DeviceID));
			}
		}
		return buttons;
	}

	std::string IInputSystem::GetButtonNameForInput(Input input, std::string_view button_format)
	{
		/// Find the correct mapping from the device that was updated the latest

		IInputDevice* last_device = nullptr;
		Mapping* last_mapping = nullptr;
		seconds_t last_active = {};
		for (auto& mapping : GetPlayer(input.Player)->Mappings[input.ActionID])
		{
			if (auto device = GetInputDevice(mapping.DeviceID); device && device->GetLastActiveTime() >= last_active)
			{
				last_mapping = &mapping;
				last_device = device;
				last_active = device->GetLastActiveTime();
			}
		}

		if (last_mapping)
		{
			auto props = last_device->GetInputProperties(last_mapping->Inputs[0]);
			return fmt::format(button_format, props.Name);
		}

		return "";
	}

	void IInputSystem::Debug(IDebugger& debugger)
	{
		debugger.Text("Last Active Device: {}", mLastActiveDevice ? mLastActiveDevice->GetName() : "none");
		/*
		IKeyboardDevice* mKeyboard = nullptr;
		IMouseDevice* mMouse = nullptr;
		IGamepadDevice* mFirstGamepad = nullptr;

		IInputDevice* mLastActiveDevice = nullptr;

		std::vector<std::unique_ptr<IInputDevice>> mInputDevices;
		std::map<void*, IGamepadDevice*> mJoystickMap;

		std::map<PlayerID, PlayerInformation> mPlayers;
		*/
	}

#if 0
	std::string IntIDToString(int id)
	{
		char chars[5] = { 0 };
		int i = 0;
		for (; i < 4; i++)
		{
			if (!::isprint(chars[3 - i] = (id & 0xFF)))
				return "-";
			if ((id >>= 8) == 0)
				break;
		}
		return baselib::Stringify('\'', chars + (3 - i), '\'');
	}

	void BadGame::DebugPlayer(PlayerInformation const& player)
	{
		ImGui::Text("Player ID: %lli", player.ID);

		if (player.BoundDeviceIDs.empty())
			ImGui::Text("Player Bound Devices: none");
		else
		{
			if (ImGui::CollapsingHeader("Player Bound Devices"))
			{
				for (auto& devid : player.BoundDeviceIDs)
				{
					ImGui::BulletText("%s", GetInputDeviceName(devid).c_str());
				}
			}
		}

		if (ImGui::CollapsingHeader("Player Mappings"))
		{
			for (auto& mapping : player.Mappings)
			{
				Controls::Text(mapping.first, " (", IntIDToString(mapping.first), "):");
				ImGui::Indent();
				for (auto& button : mapping.second)
				{
					if (auto device = GetInputDevice(button.DeviceID))
					{
						Controls::Text(device->GetInputProperties(button.Inputs[0]).Name, " @ ", device->GetName());
						if (device->IsValidInput(button.Inputs[1]))
							Controls::Text(device->GetInputProperties(button.Inputs[1]).Name, " @ ", device->GetName());
					}
					else
					{
						Controls::Text("Input #", button.Inputs[0], " @ ", GetInputDeviceName(button.DeviceID));
					}
				}
				ImGui::Unindent();
			}
		}
	}


	void DebugDevice(IInputDevice& device)
	{
		ImGui::PushID(&device);
		std::string name = (std::string)device.GetName();
		if (ImGui::CollapsingHeader(name.c_str()))
		{
			if (ImGui::Button("Force Refresh"))
				device.ForceRefresh();
			Controls::StatusLight(device.IsConnected(), "Connected", "Disconnected");
			//Controls::StatusLight(device.IsFocused(), "Focused", "Not Focused");
			if (ImGui::CollapsingHeader("Inputs"))
			{
				for (DeviceInputID i = 0; i < device.GetMaxInput(); i++)
				{
					if (device.IsValidInput(i))
					{
						auto properties = device.GetInputProperties(i);
						Controls::StatusLightV(device.GetInputStateDigital(i), "Input #", i, " - ", properties.Name, ": ", device.GetInputState(i), " / ", device.GetInputStateLastFrame(i));
						/*
						ImGui::Indent();
						auto properties = device.GetInputProperties(i);
						Controls::Text("Name: \"", properties.Name, "\", ", properties.Flags, ", Value Range: [", properties.MinValue, "; ", properties.MaxValue,
							"], Step Size: ", properties.StepSize, ", Pressed Threshold: ", properties.PressedThreshold);
						ImGui::Unindent();
						*/

						ImGui::PushID((int)i);
						if (ImGui::TreeNode("Properties"))
						{
							Controls::Text("Name: \"", properties.Name, "\"");
							Controls::Text("Flags: ", properties.Flags, "");
							Controls::Text("MinValue: ", properties.MinValue, "");
							Controls::Text("MaxValue: ", properties.MaxValue, "");
							Controls::Text("NeutralValue: ", properties.NeutralValue, "");
							if (properties.Flags.is_set(InputFlags::HasDeadzone))
							{
								Controls::Text("DeadZoneMin: ", properties.DeadZoneMin, "");
								Controls::Text("DeadZoneMax: ", properties.DeadZoneMax, "");
							}
							Controls::Text("PressedThreshold: ", properties.PressedThreshold, "");
							Controls::Text("StepSize: ", properties.StepSize, "");
							ImGui::TreePop();
						}
						ImGui::PopID();
					}
				}
			}
		}
		ImGui::PopID();
	}

	void BadGame::DebugInput()
	{
		Controls::Text("Last Active Device: {}", mLastActiveDevice ? mLastActiveDevice->GetName() : "none");
		if (mPlayers.size() > 1)
		{
			if (ImGui::CollapsingHeader("Players"))
			{
				for (auto& player : mPlayers)
				{
					DebugPlayer(player.second);
				}
			}
		}
		else if (mPlayers.size() == 1)
		{
			DebugPlayer(mPlayers.begin()->second);
		}
		else
			ImGui::Text("No players set up");

		ImGui::Separator();

		if (ImGui::CollapsingHeader("Input Devices"))
		{
			ImGui::Indent();
			for (auto& dev : mInputDevices)
			{
				DebugDevice(*dev);
			}
			ImGui::Unindent();
		}
	}

#endif
}