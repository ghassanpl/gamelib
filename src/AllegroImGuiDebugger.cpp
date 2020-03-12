#include "../include/Debug/AllegroImGuiDebugger.h"

#include <imgui.h>

namespace gamelib
{
	void AllegroImGuiDebugger::Text(std::string&& str)
	{
		ImGui::Text("%s", str.c_str());
	}

	void AllegroImGuiDebugger::Value(std::string_view name, double const& val, bool writable)
	{
		if (writable)
			ImGui::InputDouble(name.data(), (double*)&val);
		else
			ImGui::Value(name.data(), (float)val, "%g");
	}

	void AllegroImGuiDebugger::Value(std::string_view name, vec2 const& val, bool writable)
	{
		ImGui::InputFloat2(name.data(), (float*)&val.x, "%g", writable ? 0 : ImGuiInputTextFlags_ReadOnly);
	}

	void AllegroImGuiDebugger::Value(std::string_view name, ivec2 const& val, bool writable)
	{
		ImGui::InputInt2(name.data(), (int*)&val.x, writable ? 0 : ImGuiInputTextFlags_ReadOnly);
	}
}