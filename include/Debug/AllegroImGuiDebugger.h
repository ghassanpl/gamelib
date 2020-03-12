#pragma once
#include "../include/Debugger.h"

namespace gamelib
{
	struct AllegroImGuiDebugger : IDebugger
	{
		virtual void Text(std::string&& str) override;
		virtual void Value(std::string_view name, double val) override;
		virtual void Value(std::string_view name, vec2& val) override;
		virtual void Value(std::string_view name, ivec2& val) override;
	};
}