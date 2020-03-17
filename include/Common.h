#pragma once

#include <type_traits>
#include <iosfwd>
#include <gsl/span>

using seconds_t = double;
using herz_t = double;

///************************************************************************/
/// Annotations
///************************************************************************/

template <typename T, typename Parameter>
struct named
{
	T Value;

	template <typename... ARGS>
	constexpr explicit named(ARGS&&... args) : Value(std::forward<ARGS>(args)...) {}

	constexpr T* operator->() noexcept { return &Value; }
	constexpr T const* operator->() const noexcept { return &Value; }

	constexpr T& get() noexcept { return Value; }
	constexpr T const& get() const noexcept { return Value; }

	template <typename U, typename = std::enable_if_t<std::is_convertible_v<T, U>>>
	constexpr explicit operator U() const noexcept(noexcept((U)Value)) { return (U)Value; }

	constexpr auto drop() noexcept(std::is_nothrow_move_constructible_v<T>) { return std::move(Value); }
};

template <typename T, typename Parameter>
inline std::ostream& operator<<(std::ostream& strm, named<T, Parameter> const& val) { return strm << val.Value; }

using bool_t = named<bool, struct NamedBool>;

using degrees_t = named<float, struct Degrees>;
using radians_t = named<float, struct Radians>;

namespace std
{
	namespace filesystem
	{
		class path;
	}
}

using std::filesystem::path;

template <typename IT>
auto make_span(IT start, IT end)
{
	return gsl::span<std::iterator_traits<IT>::value_type>{ std::to_address(start), std::distance(start, end) };
}