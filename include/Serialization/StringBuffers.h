#pragma once

#include "../Buffers.h"

namespace gamelib
{

	template <>
	struct output_buffer_traits<std::string> : base_buffer_traits<char>
	{
		static constexpr bool can_reserve = true;
	};

	template <>
	inline bool buffer_append<std::string&, char const&>(std::string& buffer, char const& c)
	{
		buffer += c;
		return true;
	}

	template <>
	bool buffer_reserve<std::string&>(std::string& buffer, long long int additional)
	{
		buffer.reserve(buffer.size() + additional);
		return true;
	}

	template <typename IT>
	size_t buffer_append_range(std::string& buffer, IT&& begin, IT&& end)
	{
		auto size_before = buffer.size();
		return buffer.append(std::forward<IT>(begin), std::forward<IT>(end)).size() - size_before;
	}

	template <>
	struct input_buffer_traits<std::string_view> : base_buffer_traits<char>
	{
		static constexpr bool is_random_access = true;
	};

	template <>
	auto buffer_get_iterator<std::string_view&>(std::string_view& buffer, intptr_t at_element)
	{
		return buffer.begin() + std::clamp(at_element, intptr_t{}, static_cast<intptr_t>(buffer.size()));
	}

	template <>
	inline bool buffer_read_to<std::string_view&, char>(std::string_view& buffer, char& c)
	{
		if (buffer.empty()) return false;
		c = buffer[0];
		buffer.remove_suffix(1);
		return true;
	}

}