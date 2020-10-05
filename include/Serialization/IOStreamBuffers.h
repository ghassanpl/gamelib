#pragma once

#include <iostream>
#include "../Buffers.h"

namespace gamelib
{

	template <>
	inline bool buffer_append<std::ostream&, char const&>(std::ostream& buffer, char const& c)
	{
		buffer.put(c);
		return !buffer.fail();
	}

	template <>
	inline bool buffer_read_to<std::istream&, char>(std::istream& buffer, char& c)
	{
		buffer.read(&c, 1);
		return !buffer.fail();
	}

}