#include "../include/Debug/AllegroImGuiDebugger.h"

#include <imgui.h>

namespace gamelib
{
	void AllegroImGuiDebugger::Text(std::string&& str)
	{
		ImGui::Text("%s", str.c_str());
	}

	void AllegroImGuiDebugger::Value(std::string_view name, double const& val, enum_flags<DebugValueFlags> flags)
	{
		if (flags.is_set(DebugValueFlags::Writeable))
			ImGui::InputDouble(name.data(), (double*)&val);
		else
			ImGui::Value(name.data(), (float)val, "%g");
	}

	void AllegroImGuiDebugger::Value(std::string_view name, vec2 const& val, enum_flags<DebugValueFlags> flags)
	{
		ImGui::InputFloat2(name.data(), (float*)&val.x, "%g", flags.is_set(DebugValueFlags::Writeable) ? 0 : ImGuiInputTextFlags_ReadOnly);
	}

	void AllegroImGuiDebugger::Value(std::string_view name, ivec2 const& val, enum_flags<DebugValueFlags> flags)
	{
		ImGui::InputInt2(name.data(), (int*)&val.x, flags.is_set(DebugValueFlags::Writeable) ? 0 : ImGuiInputTextFlags_ReadOnly);
	}
}