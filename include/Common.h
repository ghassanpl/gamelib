#pragma once

#include <type_traits>
#include <iosfwd>
#include <span>
#include "../../error_handler/include/ghassanpl/error_handler.h"

using std::byte;
using seconds_t = double;
using herz_t = double;
using progress_t = double; /// 0.0 - 1.0

///************************************************************************/
/// Annotations
///************************************************************************/

template <typename T, typename Parameter>
struct named
{
	using base_type = T;
	using tag_type = Parameter;

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

	template <class _Ty>
	class shared_ptr;
}

using std::filesystem::path;

namespace gamelib
{
	struct IErrorReporter;
}

template <typename IT>
auto make_span(IT start, IT end)
{
	return std::span<typename std::iterator_traits<IT>::value_type>{ std::to_address(start), std::distance(start, end) };
}

template <typename T>
std::shared_ptr<T> make_raw_shared(T* ptr) { return { std::shared_ptr<T>{}, ptr }; }
template <typename T, typename = std::enable_if_t<!std::is_pointer_v<T>>>
std::shared_ptr<T> make_raw_shared(T& ptr) { return { std::shared_ptr<T>{}, ptr }; }

template <typename INVOCABLE_TYPE, typename RESULT, typename... ARG_TYPES>
concept invocable_with_result = std::invocable<INVOCABLE_TYPE, ARG_TYPES...> && std::constructible_from<RESULT, std::invoke_result_t<INVOCABLE_TYPE, ARG_TYPES...>>;

template <typename INVOCABLE_TYPE, typename RESULT, typename... ARG_TYPES>
concept invocable_with_exact_result = std::invocable<INVOCABLE_TYPE, ARG_TYPES...> && std::is_same<std::invoke_result_t<INVOCABLE_TYPE, ARG_TYPES...>, RESULT>;

template <class T, class... Types>
concept is_any_of_v = std::disjunction_v<std::is_same<T, Types>...>;