#pragma once

#include <cstdint>
#include <type_traits>

namespace gamelib
{
	template <typename ELEMENT_TYPE>
	struct base_buffer_traits
	{
		using element_type = char;
	};

	template <typename BUFFER>
	struct output_buffer_traits : base_buffer_traits<char>
	{
		static constexpr bool can_reserve = false;
	};

	/// ////////////////////////////////////////////////////////////// ///
	/// Output buffer concept
	/// ////////////////////////////////////////////////////////////// ///

	template <typename T>
	concept simple_range = requires (T range) {
		cbegin(range);
		cend(range);
	} || requires (T range) {
		std::cbegin(range);
		std::cend(range);
	};

	template <typename BUFFER, typename T>
	bool buffer_append(BUFFER&& buffer, T&& val)
	{
		static_assert(false, "buffer_append needs to be specialized for the BUFFER type");
	}

	template <typename BUFFER>
	bool buffer_reserve(BUFFER& /*buffer*/, long long int /*additional*/)
	{
		return false;
	}

	template <typename BUFFER, typename IT>
	size_t buffer_append_range(BUFFER& buffer, IT&& begin, IT&& end)
	{
		const auto begin_start = begin;
		buffer_reserve(buffer, end - begin);
		while (begin < end)
			if (!buffer_append(buffer, *begin)) break; else ++begin;
		return std::distance(begin_start, begin);
	}

	template <typename RANGE>
	concept sizeable = requires (RANGE range) {
		size(range);
	} || requires (RANGE range) {
		std::size(range);
	};

	template <typename BUFFER, simple_range RANGE>
	size_t buffer_append_range(BUFFER& buffer, RANGE&& range)
	{
		using std::size;
		if constexpr (sizeable<RANGE>)
			buffer_reserve(buffer, size(range));
		
		size_t result = 0;
		for (auto&& el : range)
			if (!buffer_append(buffer, el)) break; else result++;

		return result;
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

	template <typename BUFFER, typename T>
	size_t buffer_string_append(BUFFER& buffer, T&& val)
	{
		using std::cbegin;
		using std::cend;
		if constexpr (simple_range<T>)
			return buffer_append_range(buffer, cbegin(val), cend(val));
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

	template <typename BUFFER>
	struct input_buffer_traits : base_buffer_traits<char>
	{
		static constexpr bool is_random_access = false;
	};

	template <typename BUFFER>
	auto buffer_get_iterator(BUFFER&& buffer, intptr_t at_element)
	{
		static_assert(false, "buffer_get_iterator needs to be specialized for the BUFFER type");
		return nullptr;
	}
	
	template <typename BUFFER, typename T>
	bool buffer_read_to(BUFFER&& buffer, T& val)
	{
		static_assert(false, "buffer_read_to needs to be specialized for the BUFFER type");
	}

	template <typename T, typename BUFFER>
	T buffer_read(BUFFER&& buffer)
	{
		T result;
		buffer_read_to(std::forward<BUFFER>(buffer), result);
		return result;
	}

	template <typename BUFFER>
	size_t buffer_available_data(BUFFER&& buffer)
	{
		return 1;
	}

	template <typename INPUT_BUFFER, typename OUTPUT_BUFFER, typename ELEMENT_TYPE>
	size_t buffer_copy(INPUT_BUFFER&& input, OUTPUT_BUFFER&& output, size_t try_elements)
	{
		/// TODO: If input buffer supports buffering, do this in batches, or make a buffer_copy_buffered function
		if constexpr (input_buffer_traits<INPUT_BUFFER>::is_random_access)
		{
			auto start = buffer_get_iterator(input, 0);
			auto end = buffer_get_iterator(input, (intptr_t)try_elements);
			return buffer_append_range(std::forward<OUTPUT_BUFFER>(output), start, end);
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

	template <typename INPUT_BUFFER, typename OUTPUT_BUFFER>
	size_t buffer_copy(INPUT_BUFFER&& input, OUTPUT_BUFFER&& output)
	{
		/// TODO: If input buffer supports buffering, do this in batches, or make a buffer_copy_buffered function
		if constexpr (input_buffer_traits<INPUT_BUFFER>::is_random_access)
		{
			auto start = buffer_get_iterator(input, 0);
			auto end = buffer_get_iterator(input, std::numeric_limits<intptr_t>::max());
			return buffer_append_range(std::forward<OUTPUT_BUFFER>(output), start, end);
		}
		else
		{
			size_t result = 0;
			while (true)
			{
				if (buffer_available_data(input) == 0)
					break;
				if (!buffer_append(output, buffer_read<input_buffer_traits<INPUT_BUFFER>::element_type>(input)))
					break;
				result++;
			}
			return result;
		}
	}

	template <typename INPUT_BUFFER, typename OUTPUT_IT>
	size_t buffer_read_range(INPUT_BUFFER&& input, OUTPUT_IT first, OUTPUT_IT last)
	{
		/// TODO: If input buffer supports buffering, do this in batches, or make a buffer_copy_buffered function
		if constexpr (input_buffer_traits<INPUT_BUFFER>::is_random_access)
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

	template <typename INPUT_BUFFER, typename OUTPUT_IT, typename SENTINEL>
	size_t buffer_read_until(INPUT_BUFFER&& input, OUTPUT_IT output, SENTINEL sentinel)
	{
		size_t result = 0;
		while (true)
		{
			if (buffer_available_data(input) == 0)
				break;

			typename input_buffer_traits<INPUT_BUFFER>::element_type element = {};
			if (!buffer_read_to(input, element))
				break;

			result++;

			if (element == sentinel)
				break;

			*output++ = element;
		}
		return result;
	}

	template <typename BUFFER, typename CHAR_TYPE>
	size_t buffer_read_cstring_ptr(BUFFER&& buffer, CHAR_TYPE* cstr, size_t max_len)
	{
		return buffer_read_range(std::forward<BUFFER>(buffer), cstr, cstr + max_len);
	}

	template <typename BUFFER, size_t N, typename CHAR_TYPE>
	size_t buffer_read_cstring(BUFFER&& buffer, CHAR_TYPE(&cstr)[N])
	{
		return buffer_read_range(std::forward<BUFFER>(buffer), cstr, cstr + (N - 1));
	}

	template <typename BUFFER, size_t N, typename CHAR_TYPE>
	size_t buffer_read_cstring(BUFFER&& buffer, CHAR_TYPE(&cstr)[N], size_t max_len)
	{
		return buffer_read_range(std::forward<BUFFER>(buffer), cstr, cstr + std::min(max_len, N - 1));
	}

	template <typename T>
	concept simple_mutable_range = requires (T range) {
		begin(range);
		end(range);
	} || requires (T range) {
		std::begin(range);
		std::end(range);
	};

	template <typename BUFFER, typename T>
	size_t buffer_string_read(BUFFER&& buffer, T& val)
	{
		using std::begin;
		using std::end;
		if constexpr (simple_mutable_range<T>)
			return buffer_read_range(std::forward<BUFFER>(buffer), begin(val), end(val));
		else if constexpr (std::is_pointer_v<T>)
			return buffer_read_range(std::forward<BUFFER>(buffer), val, val + std::char_traits<decltype(*val)>::length(val));
		else if constexpr (std::is_array_v<T>)
			return buffer_read_cstring(std::forward<BUFFER>(buffer), std::forward<T>(val));
		else
			return buffer_read(std::forward<BUFFER>(buffer), std::forward<T>(val));
	}

	template <typename BUFFER>
	char32_t buffer_read_utf8(BUFFER&& buffer)
	{

	}
}