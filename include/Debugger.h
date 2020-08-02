#pragma once

#include <string_view>
#include <cstdint>
#include "Includes/EnumFlags.h"
#include "Includes/Format.h"
#include "Includes/GLM.h"
#include "Includes/JSON.h"
#include "Includes/MagicEnum.h"

namespace gamelib
{

	enum class DebugAction
	{
		View,
		Reset,
		Pause,
		Unpause,
	};

	enum class DebugValueFlags
	{
		Writeable,
	};

	struct IDebugger
	{
		virtual ~IDebugger() = default;

		/// View stuff

		template <typename... ARGS>
		void Text(std::string_view format, ARGS&&... args)
		{
			this->Text(fmt::format(format, std::forward<ARGS>(args)...));
		}

		virtual void Text(std::string&& str) = 0;

		virtual void Value(std::string_view name, vec2 const& val, enum_flags<DebugValueFlags> flags) = 0;
		virtual void Value(std::string_view name, ivec2 const& val, enum_flags<DebugValueFlags> flags) = 0;
		virtual void Value(std::string_view name, double const& val, enum_flags<DebugValueFlags> flags) = 0;

		template <typename T>
		auto Value(std::string_view name, T& val, enum_flags<DebugValueFlags> flags = {})
		{
			this->Value(name, const_cast<const T&>(val), flags | (std::is_const_v<T> ? DebugValueFlags::Writeable : DebugValueFlags{}));
		}

		template <typename T>
		auto Value(std::string_view name, T&& val, enum_flags<DebugValueFlags> flags = {})
		{
			this->Value(name, const_cast<const T&>(val), flags);
		}

	protected:

		/*
		virtual void Value(std::string_view name, bool& value) = 0;
		virtual void Value(std::string_view name, char& value) = 0;
		virtual void Value(std::string_view name, int8_t& value) = 0;
		virtual void Value(std::string_view name, uint8_t& value) = 0;
		virtual void Value(std::string_view name, int16_t& value) = 0;
		virtual void Value(std::string_view name, uint16_t& value) = 0;
		virtual void Value(std::string_view name, int32_t& value) = 0;
		virtual void Value(std::string_view name, uint32_t& value) = 0;
		virtual void Value(std::string_view name, int64_t& value) = 0;
		virtual void Value(std::string_view name, uint64_t& value) = 0;
		virtual void Value(std::string_view name, double& value) = 0;
		virtual void Value(std::string_view name, float& value) = 0;
		
		virtual void PairValue(std::string_view name, float& value1, float& value2) = 0;
		
		virtual void Value(std::string_view name, bool const& value) = 0;
		virtual void Value(std::string_view name, uint64_t const& value) = 0;
		virtual void Value(std::string_view name, double const& value) = 0;

		template <typename T>
		void Value(std::string_view name, T& value)
		{
			DebugValue(*this, name, value);
		}

		template <typename T>
		void Value(std::string_view name, T const& value)
		{
			DebugValue(*this, name, value);
		}

		virtual bool StartGroup(std::string_view name) = 0;
		virtual void EndGroup() = 0;
		*/
	};

}