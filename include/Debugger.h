#pragma once

#include <string_view>
#include <cstdint>
#include "Debug/Statistics.h"
#include "Includes/EnumFlags.h"
#include "Includes/GLM.h"
#include "Includes/Format.h"

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
			this->DoText(fmt::format(format, std::forward<ARGS>(args)...));
		}

		void Value(std::string_view name, bool const& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags); }
		void Value(std::string_view name, int const& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags); }
		void Value(std::string_view name, size_t const& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags); }
		void Value(std::string_view name, vec2 const& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags); }
		void Value(std::string_view name, ivec2 const& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags); }
		void Value(std::string_view name, float const& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags); }
		void Value(std::string_view name, double const& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags); }

		void Value(std::string_view name, bool& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags + DebugValueFlags::Writeable); }
		void Value(std::string_view name, int& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags + DebugValueFlags::Writeable); }
		void Value(std::string_view name, size_t& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags + DebugValueFlags::Writeable); }
		void Value(std::string_view name, vec2& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags + DebugValueFlags::Writeable); }
		void Value(std::string_view name, ivec2& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags + DebugValueFlags::Writeable); }
		void Value(std::string_view name, float& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags + DebugValueFlags::Writeable); }
		void Value(std::string_view name, double& val, enum_flags<DebugValueFlags> flags = {}) { DoValue(name, val, flags + DebugValueFlags::Writeable); }

		template <typename... ARGS>
		bool StartGroup(std::string_view name, std::string_view format, ARGS&&... args)
		{
			return this->DoStartGroup(name, fmt::format(format, std::forward<ARGS>(args)...));
		}

		void EndGroup() { DoEndGroup(); }

		template <typename T>
		void Value(std::string_view name, KPI<T> const& kpi)
		{
			if (this->StartGroup(name, "{}", kpi.CurrentValue))
			{
				this->Text("Current: {}", kpi.CurrentValue);
				this->Text("Samples: {}", kpi.SampleCount);
				this->Text("Sum: {}", kpi.Sum);
				this->Text("Max: {}", kpi.Max);
				this->Text("Min: {}", kpi.Min);
				if (kpi.SampleCount)
				{
					this->Text("Avg: {}", kpi.Sum / kpi.SampleCount);
				}
				this->EndGroup();
			}
		}

	protected:

		virtual void DoText(std::string&& str) = 0;

		virtual bool DoStartGroup(std::string_view name, std::string&& str) = 0;
		virtual void DoEndGroup() = 0;
								 
		virtual void DoValue(std::string_view name, bool const& val, enum_flags<DebugValueFlags> flags) = 0;
		virtual void DoValue(std::string_view name, int const& val, enum_flags<DebugValueFlags> flags) = 0;
		virtual void DoValue(std::string_view name, size_t const& val, enum_flags<DebugValueFlags> flags) = 0;
		virtual void DoValue(std::string_view name, vec2 const& val, enum_flags<DebugValueFlags> flags) = 0;
		virtual void DoValue(std::string_view name, ivec2 const& val, enum_flags<DebugValueFlags> flags) = 0;
		virtual void DoValue(std::string_view name, float const& val, enum_flags<DebugValueFlags> flags) = 0;
		virtual void DoValue(std::string_view name, double const& val, enum_flags<DebugValueFlags> flags) = 0;

	};

}