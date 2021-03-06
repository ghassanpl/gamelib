#pragma once

#include <X:\Code\Native\enum_flags\include\enum_flags.h>
#include <iosfwd>

using ghassanpl::enum_flags;

namespace ghassanpl
{
	template <typename E, typename B>
	inline std::ostream& operator<<(std::ostream& strm, enum_flags<E, B> b)
	{
		OutputFlagsFor(strm, E{}, b.bits);
		return strm;
	}
}

#define GAMELIB_ENUM_FLAGS
#ifdef GAMELIB_JSON
#include "../Combos/EnumFlags+JSON.h"
#endif