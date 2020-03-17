#pragma once

#include <iostream>

namespace gamelib
{

	inline bool buffer_read_to(std::istream& stream, char& c)
	{
		stream.read(&c, 1);
		return !stream.fail();
	}

}