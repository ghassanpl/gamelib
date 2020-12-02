#pragma once
#include "../include/Debugger.h"

namespace gamelib
{
	struct AllegroImGuiDebugger : IDebugger
	{
		using IDebugger::Value;
		virtual void DoText(std::string&& str) override;
		virtual void DoValue(std::string_view name, bool const& val, enum_flags<DebugValueFlags> flags) override;
		virtual void DoValue(std::string_view name, int const& val, enum_flags<DebugValueFlags> flags) override;
		virtual void DoValue(std::string_view name, size_t const& val, enum_flags<DebugValueFlags> flags) override;
		virtual void DoValue(std::string_view name, float const& val, enum_flags<DebugValueFlags> flags) override;
		virtual void DoValue(std::string_view name, double const& val, enum_flags<DebugValueFlags> flags) override;
		virtual void DoValue(std::string_view name, vec2 const& val, enum_flags<DebugValueFlags> flags) override;
		virtual void DoValue(std::string_view name, ivec2 const& val, enum_flags<DebugValueFlags> flags) override;

		virtual bool DoStartGroup(std::string_view name, std::string&& str) override;
		virtual void DoEndGroup() override;

	};
}