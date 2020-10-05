#include "game.h"

#include <gtest/gtest.h>

#include "Serialization/IOStreamBuffers.h"
#include "Serialization/StringBuffers.h"
#include "Machine/IMachine.h"

TEST(Buffers, buffer_string_read)
{
	using std::begin;
	(void)begin("hello");
	std::string source = "hello";
	std::string result;
	buffer_copy(std::string_view{ source }, result);
	buffer_copy(std::string_view{ source }, std::cout);
	buffer_append_range(source, "hello");
	EXPECT_EQ(source, result);
}