#include "../include/Debug/AllegroImGuiDebugger.h"

#include <imgui.h>

namespace gamelib
{
	void AllegroImGuiDebugger::Text(std::string&& str)
	{
		ImGui::Text("%s", str.c_str());
	}

	void AllegroImGuiDebugger::Value(std::string_view name, double val)
	{
		ImGui::Value(name.data(), (float)val, "%g");
	}

	void AllegroImGuiDebugger::Value(std::string_view name, vec2& val)
	{
		ImGui::InputFloat2(name.data(), &val.x);
	}
	
	void AllegroImGuiDebugger::Value(std::string_view name, ivec2& val)
	{
		ImGui::InputInt2(name.data(), &val.x);
	}
}