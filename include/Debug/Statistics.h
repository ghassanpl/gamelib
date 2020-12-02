#pragma once

#include <utility>

namespace gamelib
{

	template <typename T>
	struct KPI
	{
		T CurrentValue{};
		T Sum{};
		T Max{};
		T Min{};
		size_t SampleCount = 0;

		KPI& operator=(T val) {
			CurrentValue = val;
			Sum += val;
			Max = std::max(Max, val);
			Min = std::min(Min, val);
			++SampleCount;
			return *this;
		};

		void reset()
		{
			CurrentValue = {};
			Sum = {};
			Max = {};
			Min = {};
			SampleCount = 0;
		}

		operator T() const { return CurrentValue; }
	};

}