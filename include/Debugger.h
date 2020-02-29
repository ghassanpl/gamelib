#pragma once

#include <string_view>
#include <cstdint>

namespace gamelib
{

	enum class DebugAction
	{
		View,
		Reset,
		Pause,
		Unpause,
	};

	struct IDebugger
	{
		virtual ~IDebugger() = default;

		/// View stuff

		void DebugValue(std::string_view name, bool value);
		void DebugValue(std::string_view name, char value);
		void DebugValue(std::string_view name, int8_t value);
		void DebugValue(std::string_view name, uint8_t value);
		void DebugValue(std::string_view name, int16_t value);
		void DebugValue(std::string_view name, uint16_t value);
		void DebugValue(std::string_view name, int32_t value);
		void DebugValue(std::string_view name, uint32_t value);
		void DebugValue(std::string_view name, int64_t value);
		void DebugValue(std::string_view name, uint64_t value);
		void DebugValue(std::string_view name, double value);
		void DebugValue(std::string_view name, float value);

		bool StartGroup(std::string_view name);
		void EndGroup();
	};

}