#pragma once

#include "../Common.h"
#include "../Errors.h"

/// Shamelessly stolen from https://github.com/epyon/nova-ecs/blob/master/nova-ecs/handle_manager.hh

namespace gamelib
{
	class Handle
	{
	public:
		static constexpr int INDEX_BITS = 16;
		static constexpr int COUNTER_BITS = 16;

		constexpr Handle() : Index(0), Counter(0) {}
		constexpr Handle(unsigned a_index, unsigned a_counter)
			: Index(a_index), Counter(a_counter) {}

		constexpr inline bool operator==(const Handle& rhs) const {
			return Index == rhs.Index && Counter == rhs.Counter;
		}
		constexpr inline bool operator!=(const Handle& rhs) const { return !(*this == rhs); }

		constexpr bool IsValid()    const { return !(Index == 0 && Counter == 0); }
		constexpr operator bool()    const { return IsValid(); }
		constexpr unsigned Hash()    const { return Counter << INDEX_BITS | Index; }
		unsigned Index : INDEX_BITS;
		unsigned Counter : COUNTER_BITS;
	};
}

namespace std
{
	template<> struct hash<gamelib::Handle>
	{
		size_t operator()(const gamelib::Handle& s) const noexcept
		{
			return s.Hash();
		}
	};
}

namespace gamelib
{
	class HandleManager : public IErrorReportingInterface
	{
		typedef int      index_type;
		typedef unsigned value_type;
		static const index_type NONE = index_type(-1);
		static const index_type USED = index_type(-2);
	
	public:

		HandleManager(IErrorReporter& error_reporter) : IErrorReportingInterface(error_reporter), mFirstFree(NONE), mLastFree(NONE) {}

		Handle CreateHandle()
		{
			value_type i = get_free_entry();
			mEntries[i].Counter++;
			AssumingNotEqual(mEntries[i].Counter, 0, "Out of handles");
			mEntries[i].NextFree = USED;
			return Handle(i, mEntries[i].Counter);
		}

		void FreeHandle(Handle h)
		{
			value_type index = h.Index;
			mEntries[index].NextFree = NONE;
			if (mLastFree == NONE)
			{
				mFirstFree = mLastFree = index;
				return;
			}
			mEntries[mLastFree].NextFree = index;
			mLastFree = index;
		}

		bool IsValid(Handle h) const
		{
			if (!h.IsValid()) return false;
			if (h.Index >= mEntries.size()) return false;
			const index_entry& entry = mEntries[h.Index];
			return entry.NextFree == USED && entry.Counter == h.Counter;
		}

		void Clear()
		{
			mFirstFree = NONE;
			mLastFree = NONE;
			mEntries.clear();
		}

		Handle GetHandle(index_type i) const
		{
			if (i >= 0 && i < mEntries.size())
				return Handle(i, mEntries[i].Counter);
			return {};
		}

	private:
		struct index_entry
		{
			value_type Counter;
			index_type NextFree;

			index_entry() : Counter(0), NextFree(NONE) {}
		};

		value_type get_free_entry()
		{
			if (mFirstFree != NONE)
			{
				value_type result = mFirstFree;
				mFirstFree = mEntries[result].NextFree;
				mEntries[result].NextFree = USED;
				if (mFirstFree == NONE) mLastFree = NONE;
				return result;
			}
			mEntries.emplace_back();
			return value_type(mEntries.size() - 1);
		}

		index_type mFirstFree;
		index_type mLastFree;
		std::vector< index_entry > mEntries;
	};

}