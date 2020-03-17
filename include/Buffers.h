#pragma once

#include <cstdint>
#include <type_traits>

namespace gamelib
{
	/// ////////////////////////////////////////////////////////////// ///
	/// Output buffer concept
	/// ////////////////////////////////////////////////////////////// ///

	template <typename BUFFER>
	bool buffer_reserve(BUFFER& /*buffer*/, long long int /*additional*/)
	{
		return false;
	}

	template <typename BUFFER, typename IT>
	size_t buffer_append_range(BUFFER& buffer, IT begin, IT end)
	{
		const auto begin_start = begin;
		buffer_reserve(buffer, end - begin);
		while (begin < end)
			if (!buffer_append(buffer, *begin)) break; else ++begin;
		return std::distance(begin_start, begin);
	}

	template <typename BUFFER, typename CHAR_TYPE>
	size_t buffer_append_cstring_ptr(BUFFER& buffer, const CHAR_TYPE* cstr)
	{
		return buffer_append_range(buffer, cstr, cstr + std::char_traits<CHAR_TYPE>::length(cstr));
	}

	template <typename BUFFER, typename CHAR_TYPE>
	size_t buffer_append_cstring_ptr(BUFFER& buffer, const CHAR_TYPE* cstr, size_t max_len)
	{
		return buffer_append_range(buffer, cstr, cstr + std::min(max_len, std::char_traits<CHAR_TYPE>::length(cstr)));
	}

	template <typename BUFFER, size_t N, typename CHAR_TYPE>
	size_t buffer_append_cstring(BUFFER& buffer, const CHAR_TYPE(&cstr)[N])
	{
		return buffer_append_range(buffer, cstr, cstr + (N - 1));
	}

	template <typename BUFFER, size_t N, typename CHAR_TYPE>
	size_t buffer_append_cstring(BUFFER& buffer, const CHAR_TYPE(&cstr)[N], size_t max_len)
	{
		return buffer_append_range(buffer, cstr, cstr + std::min(max_len, N - 1));
	}

	template <typename T, typename _ = void>
	struct is_range : std::false_type {};

	template <typename T>
	struct is_range<T, std::conditional_t<false, std::void_t<decltype(std::cbegin(std::declval<T>())), decltype(std::cend(std::declval<T>()))>, void>> : std::true_type {};
	
	template <typename BUFFER, typename T>
	size_t buffer_string_append(BUFFER& buffer, T&& val)
	{
		if constexpr (is_range<T>::value)
			return buffer_append_range(buffer, std::begin(val), std::end(val));
		else if constexpr (std::is_pointer_v<T>)
			return buffer_append_range(buffer, val, val + std::char_traits<decltype(*val)>::length(val));
		else if constexpr (std::is_array_v<T>)
			return buffer_append_cstring(buffer, std::forward<T>(val));
		else
			return buffer_append(buffer, std::forward<T>(val));
	}

	template <typename BUFFER>
	size_t buffer_append_utf8(BUFFER& buffer, char32_t cp)
	{
		size_t result = 0;
		/// Assuming codepoint is valid
		if (cp < 0x80)
		{
			result += buffer_append(buffer, static_cast<uint8_t>(cp));
		}
		else if (cp < 0x800) {
			result += buffer_append(buffer, static_cast<uint8_t>((cp >> 6) | 0xc0));
			result += buffer_append(buffer, static_cast<uint8_t>((cp & 0x3f) | 0x80));
		}
		else if (cp < 0x10000) {
			result += buffer_append(buffer, static_cast<uint8_t>((cp >> 12) | 0xe0));
			result += buffer_append(buffer, static_cast<uint8_t>(((cp >> 6) & 0x3f) | 0x80));
			result += buffer_append(buffer, static_cast<uint8_t>((cp & 0x3f) | 0x80));
		}
		else {
			result += buffer_append(buffer, static_cast<uint8_t>((cp >> 18) | 0xf0));
			result += buffer_append(buffer, static_cast<uint8_t>(((cp >> 12) & 0x3f) | 0x80));
			result += buffer_append(buffer, static_cast<uint8_t>(((cp >> 6) & 0x3f) | 0x80));
			result += buffer_append(buffer, static_cast<uint8_t>((cp & 0x3f) | 0x80));
		}
		return result;
	}

	/// ////////////////////////////////////////////////////////////// ///
	/// Input buffer concept
	/// ////////////////////////////////////////////////////////////// ///

	/// template <typename BUFFER>
	/// auto buffer_get_iterator(BUFFER& buffer, intptr_t at_element);
	/// template <typename BUFFER, typename T>
	/// bool buffer_read_to(BUFFER& buffer, T& val);

	template <typename BUFFER>
	constexpr bool buffer_is_random_access()
	{
		return false;
	}

	template <typename T, typename BUFFER>
	T buffer_read(BUFFER& buffer)
	{
		T result;
		buffer_read_to(buffer, result);
		return result;
	}

	template <typename BUFFER>
	size_t buffer_available_data(BUFFER& buffer)
	{
		return 1;
	}

	template <typename INPUT_BUFFER, typename OUTPUT_BUFFER, typename ELEMENT_TYPE>
	size_t buffer_copy(INPUT_BUFFER& input, OUTPUT_BUFFER& output, size_t try_elements)
	{
		/// TODO: If input buffer supports buffering, do this in batches, or make a buffer_copy_buffered function
		if constexpr (buffer_is_random_access<INPUT_BUFFER>())
		{
			auto start = buffer_get_iterator(input, 0);
			auto end = buffer_get_iterator(input, (intptr_t)try_elements);
			return buffer_append_range(output, start, end);
		}
		else
		{
			auto count = try_elements;
			while (try_elements)
			{
				if (buffer_available_data(input) == 0)
					break;
				if (!buffer_append(output, buffer_read<ELEMENT_TYPE>(input)))
					break;
				try_elements--;
			}
			return count - try_elements;
		}
	}

	template <typename INPUT_BUFFER, typename OUTPUT_IT>
	size_t buffer_read_range(INPUT_BUFFER& input, OUTPUT_IT first, OUTPUT_IT last)
	{
		/// TODO: If input buffer supports buffering, do this in batches, or make a buffer_copy_buffered function
		if constexpr (buffer_is_random_access<INPUT_BUFFER>())
		{
			auto start = buffer_get_iterator(input, 0);
			auto end = buffer_get_iterator(input, std::distance(first, last));
			return std::distance(std::copy(start, end, first, last), first);
		}
		else
		{
			while (first < last)
			{
				if (buffer_available_data(input) == 0)
					break;
				if (!buffer_read_to(input, *first++))
					break;
			}
			return std::distance(first, last);
		}
	}

	template <typename BUFFER, typename CHAR_TYPE>
	size_t buffer_read_cstring_ptr(BUFFER& buffer, CHAR_TYPE* cstr, size_t max_len)
	{
		return buffer_read_range(buffer, cstr, cstr + max_len);
	}

	template <typename BUFFER, size_t N, typename CHAR_TYPE>
	size_t buffer_read_cstring(BUFFER& buffer, CHAR_TYPE(&cstr)[N])
	{
		return buffer_read_range(buffer, cstr, cstr + (N - 1));
	}

	template <typename BUFFER, size_t N, typename CHAR_TYPE>
	size_t buffer_append_cstring(BUFFER& buffer, CHAR_TYPE(&cstr)[N], size_t max_len)
	{
		return buffer_read_range(buffer, cstr, cstr + std::min(max_len, N - 1));
	}

	template <typename T, typename _ = void>
	struct is_mutable_range : std::false_type {};

	template <typename T>
	struct is_mutable_range<T, std::conditional_t<false, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>, void>> : std::true_type {};

	template <typename BUFFER, typename T>
	size_t buffer_string_read(BUFFER& buffer, T& val)
	{
		if constexpr (is_mutable_range<T>::value)
			return buffer_read_range(buffer, std::begin(val), std::end(val));
		else if constexpr (std::is_pointer_v<T>)
			return buffer_read_range(buffer, val, val + std::char_traits<decltype(*val)>::length(val));
		else if constexpr (std::is_array_v<T>)
			return buffer_read_cstring(buffer, std::forward<T>(val));
		else
			return buffer_read(buffer, std::forward<T>(val));
	}

	template <typename BUFFER>
	char32_t buffer_read_utf8(BUFFER& buffer)
	{

	}
}