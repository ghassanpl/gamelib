#pragma once
#include "../include/Debugger.h"

namespace gamelib
{
	struct AllegroImGuiDebugger : IDebugger
	{
		using IDebugger::Value;
		virtual void Text(std::string&& str) override;
		virtual void Value(std::string_view name, double const& val, bool writable) override;
		virtual void Value(std::string_view name, vec2 const& val, bool writable) override;
		virtual void Value(std::string_view name, ivec2 const& val, bool writable) override;
	};
}