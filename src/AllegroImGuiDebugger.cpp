#include "../include/Debug/AllegroImGuiDebugger.h"

#include <imgui.h>

namespace gamelib
{
	void AllegroImGuiDebugger::DoText(std::string&& str)
	{
		ImGui::Text("%s", str.c_str());
	}

	void AllegroImGuiDebugger::DoValue(std::string_view name, bool const& val, enum_flags<DebugValueFlags> flags)
	{
		if (flags.is_set(DebugValueFlags::Writeable))
			ImGui::Checkbox(name.data(), (bool*)&val);
		else
			ImGui::Value(name.data(), val);
	}

	void AllegroImGuiDebugger::DoValue(std::string_view name, int const& val, enum_flags<DebugValueFlags> flags)
	{
		if (flags.is_set(DebugValueFlags::Writeable))
			ImGui::DragInt(name.data(), (int*)&val);
		else
			ImGui::Value(name.data(), val);
	}

	void AllegroImGuiDebugger::DoValue(std::string_view name, size_t const& val, enum_flags<DebugValueFlags> flags)
	{
		ImGui::Text("%s: %zu", name.data(), val);
	}

	void AllegroImGuiDebugger::DoValue(std::string_view name, double const& val, enum_flags<DebugValueFlags> flags)
	{
		if (flags.is_set(DebugValueFlags::Writeable))
			ImGui::InputDouble(name.data(), (double*)&val);
		else
			ImGui::Value(name.data(), (float)val, "%g");
	}

	void AllegroImGuiDebugger::DoValue(std::string_view name, float const& val, enum_flags<DebugValueFlags> flags)
	{
		if (flags.is_set(DebugValueFlags::Writeable))
			ImGui::InputFloat(name.data(), (float*)&val);
		else
			ImGui::Value(name.data(), val, "%g");
	}

	void AllegroImGuiDebugger::DoValue(std::string_view name, vec2 const& val, enum_flags<DebugValueFlags> flags)
	{
		ImGui::InputFloat2(name.data(), (float*)&val.x, "%g", flags.is_set(DebugValueFlags::Writeable) ? 0 : ImGuiInputTextFlags_ReadOnly);
	}

	void AllegroImGuiDebugger::DoValue(std::string_view name, ivec2 const& val, enum_flags<DebugValueFlags> flags)
	{
		ImGui::InputInt2(name.data(), (int*)&val.x, flags.is_set(DebugValueFlags::Writeable) ? 0 : ImGuiInputTextFlags_ReadOnly);
	}

	bool AllegroImGuiDebugger::DoStartGroup(std::string_view name, std::string&& str)
	{
		if (ImGui::CollapsingHeader(name.data()))
		{
			ImGui::Indent();
			return true;
		}
		return false;
	}
	
	void AllegroImGuiDebugger::DoEndGroup()
	{
		ImGui::Unindent();
	}
}