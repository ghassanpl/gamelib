#pragma once

#include <array>       // for array
#include <cstddef>     // for ptrdiff_t, size_t, nullptr_t
#include <iterator>    // for reverse_iterator, distance, random_access_...
#include <type_traits> // for enable_if_t, declval, is_convertible, inte...

namespace ghassanpl
{
	constexpr const std::size_t dynamic_extent = static_cast<std::size_t>(-1);

	template <class ElementType, class ParentClass, ElementType ParentClass::* MemberPointer>
	class member_span;

	// implementation details
	namespace details
	{
		template <class T>
		struct is_span_oracle : std::false_type
		{
		};

		template <class ElementType, class ParentClass, ElementType ParentClass::* MemberPointer>
		struct is_span_oracle<ghassanpl::member_span<ElementType, ParentClass, MemberPointer>> : std::true_type
		{
		};

		template <class T>
		struct is_span : public is_span_oracle<std::remove_cv_t<T>>
		{
		};

		template <class T>
		struct is_std_array_oracle : std::false_type
		{
		};

		template <class ElementType, std::size_t Extent>
		struct is_std_array_oracle<std::array<ElementType, Extent>> : std::true_type
		{
		};

		template <class T>
		struct is_std_array : is_std_array_oracle<std::remove_cv_t<T>>
		{
		};

		template <std::size_t From, std::size_t To>
		struct is_allowed_extent_conversion
			: std::integral_constant<bool, From == To || To == dynamic_extent>
		{
		};

		template <class From, class To>
		struct is_allowed_element_type_conversion
			: std::integral_constant<bool, std::is_convertible<From(*)[], To(*)[]>::value>
		{
		};

		template <class Type, class ParentClass, Type ParentClass::*MemberPointer>
		class member_span_iterator
		{
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = std::remove_cv_t<ParentClass>;
			using difference_type = std::ptrdiff_t;
			using pointer = ParentClass*;
			using member_pointer = Type*;
			using reference = ParentClass&;
			using member_reference = Type&;

			constexpr member_span_iterator() = default;

			constexpr member_span_iterator(pointer begin, pointer end, pointer current)
				: begin_(begin), end_(end), current_(current)
			{}

			constexpr operator member_span_iterator<const Type, ParentClass, MemberPointer>() const noexcept
			{
				return { begin_, end_, current_ };
			}

			constexpr member_reference operator*() const noexcept
			{
				return current_->*MemberPointer;
			}

			constexpr member_pointer operator->() const noexcept
			{
				return &(current_->*MemberPointer);
			}
			constexpr member_span_iterator& operator++() noexcept
			{
				++current_;
				return *this;
			}

			constexpr member_span_iterator operator++(int) noexcept
			{
				member_span_iterator ret = *this;
				++* this;
				return ret;
			}

			constexpr member_span_iterator& operator--() noexcept
			{
				--current_;
				return *this;
			}

			constexpr member_span_iterator operator--(int) noexcept
			{
				member_span_iterator ret = *this;
				--* this;
				return ret;
			}

			constexpr member_span_iterator& operator+=(const difference_type n) noexcept
			{
				current_ += n;
				return *this;
			}

			constexpr member_span_iterator operator+(const difference_type n) const noexcept
			{
				member_span_iterator ret = *this;
				ret += n;
				return ret;
			}

			friend constexpr member_span_iterator operator+(const difference_type n, const member_span_iterator& rhs) noexcept
			{
				return rhs + n;
			}

			constexpr member_span_iterator& operator-=(const difference_type n) noexcept
			{
				current_ -= n;
				return *this;
			}

			constexpr member_span_iterator operator-(const difference_type n) const noexcept
			{
				member_span_iterator ret = *this;
				ret -= n;
				return ret;
			}

			template <class Type2, Type2 ParentClass::* OtherMemberPointer>
			requires std::is_same_v<std::remove_cv_t<Type2>, value_type>
			constexpr difference_type operator-(const member_span_iterator<Type2, ParentClass, OtherMemberPointer>& rhs) const noexcept
			{
				return current_ - rhs.current_;
			}

			constexpr reference operator[](const difference_type n) const noexcept
			{
				return *(*this + n);
			}

			template <class Type2, Type2 ParentClass::* OtherMemberPointer>
			requires std::is_same_v<std::remove_cv_t<Type2>, value_type>
			constexpr bool operator==(const member_span_iterator<Type2, ParentClass, OtherMemberPointer>& rhs) const noexcept
			{
				return current_ == rhs.current_;
			}

			template <class Type2, Type2 ParentClass::* OtherMemberPointer>
			requires std::is_same_v<std::remove_cv_t<Type2>, value_type>
			constexpr bool operator!=(const member_span_iterator<Type2, ParentClass, OtherMemberPointer>& rhs) const noexcept
			{
				return !(*this == rhs);
			}

			template <class Type2, Type2 ParentClass::* OtherMemberPointer>
			requires std::is_same_v<std::remove_cv_t<Type2>, value_type>
			constexpr bool operator<(const member_span_iterator<Type2, ParentClass, OtherMemberPointer>& rhs) const noexcept
			{
				return current_ < rhs.current_;
			}

			template <class Type2, Type2 ParentClass::* OtherMemberPointer>
			requires std::is_same_v<std::remove_cv_t<Type2>, value_type>
			constexpr bool operator>(const member_span_iterator<Type2, ParentClass, OtherMemberPointer>& rhs) const noexcept
			{
				return rhs < *this;
			}

			template <class Type2, Type2 ParentClass::* OtherMemberPointer>
			requires std::is_same_v<std::remove_cv_t<Type2>, value_type>
			constexpr bool operator<=(const member_span_iterator<Type2, ParentClass, OtherMemberPointer>& rhs) const noexcept
			{
				return !(rhs < *this);
			}

			template <class Type2, Type2 ParentClass::* OtherMemberPointer>
			requires std::is_same_v<std::remove_cv_t<Type2>, value_type>
			constexpr bool operator>=(const member_span_iterator<Type2, ParentClass, OtherMemberPointer>& rhs) const noexcept
			{
				return !(*this < rhs);
			}

			pointer begin_ = nullptr;
			pointer end_ = nullptr;
			pointer current_ = nullptr;
		};

	} // namespace details

	template <class ElementType, class ParentClass, ElementType ParentClass::*MemberPointer>
	class member_span
	{
	public:
		// constants and types
		using element_type = ParentClass;
		using value_type = std::remove_cv_t<ParentClass>;
		using size_type = std::size_t;
		using pointer = element_type*;
		using const_pointer = const element_type*;
		using reference = ElementType&;
		using const_reference = const ElementType&;
		using difference_type = std::ptrdiff_t;

		using iterator = details::member_span_iterator<ElementType, ParentClass, MemberPointer>;
		using reverse_iterator = std::reverse_iterator<iterator>;

		constexpr member_span() noexcept : data_{ nullptr }, size_{0}
		{}

		constexpr explicit member_span(pointer ptr, size_type count) noexcept : data_(ptr), size_(count)
		{}

		constexpr explicit member_span(pointer firstElem, pointer lastElem) noexcept
			: data_(firstElem), size_(static_cast<std::size_t>(lastElem - firstElem))
		{}

		template <std::size_t N>
		constexpr member_span(element_type(&arr)[N]) noexcept
			: data_(arr + 0), size_(N)
		{}

		template <class T, std::size_t N>
		requires details::is_allowed_element_type_conversion<T, element_type>::value
		constexpr member_span(std::array<T, N>& arr) noexcept
			: data_(arr.data()), size_(N)
		{}

		template <class T, std::size_t N>
		requires details::is_allowed_element_type_conversion<const T, element_type>::value
		constexpr member_span(const std::array<T, N>& arr) noexcept
			: data_(arr.data()), size_(N)
		{}

		template <class Container>
		requires !details::is_span<Container>::value && !details::is_std_array<Container>::value &&
			std::is_pointer_v<decltype(std::declval<Container&>().data())> &&
			std::is_convertible_v<std::remove_pointer_t<decltype(std::declval<Container&>().data())>(*)[], element_type(*)[]>
		constexpr member_span(Container& cont) noexcept : data_(cont.data()), size_(cont.size())
		{}

		template <class Container>
		requires std::is_const<element_type>::value && !details::is_span<Container>::value &&
			!details::is_std_array<Container>::value &&
			std::is_pointer_v<decltype(std::declval<const Container&>().data())> &&
			std::is_convertible_v<std::remove_pointer_t<decltype(std::declval<const Container&>().data())>(*)[], element_type(*)[]>
		constexpr member_span(const Container& cont) noexcept : data_(cont.data()), size_(cont.size())
		{}

		constexpr member_span(const member_span& other) noexcept = default;

		template <ElementType ParentClass::*OtherMemberPointer>
		constexpr member_span(const member_span<ElementType, ParentClass, OtherMemberPointer>& other) noexcept
			: data_(other.data()), size_(other.size())
		{}

		~member_span() noexcept = default;
		constexpr member_span& operator=(const member_span& other) noexcept = default;

		constexpr auto first(size_type count) const noexcept
		{
			return { data(), count };
		}

		constexpr auto last(size_type count) const noexcept
		{
			return make_subspan(size() - count, -1);
		}

		constexpr auto subspan(size_type offset, size_type count = -1) const noexcept
		{
			return make_subspan(offset, count);
		}

		constexpr size_type size() const noexcept { return size_; }

		constexpr size_type size_bytes() const noexcept
		{
			return size() * sizeof(element_type);
		}

		constexpr bool empty() const noexcept { return size() == 0; }

		constexpr reference from(pointer p) const noexcept { return p->*MemberPointer; }

		constexpr reference operator[](size_type idx) const noexcept
		{
			return from(data() + idx);
		}

		constexpr reference front() const noexcept
		{
			return from(data() + 0);
		}

		constexpr reference back() const noexcept
		{
			return from(data() + (size() - 1));
		}

		constexpr pointer data() const noexcept { return data_; }

		// [span.iter], span iterator support
		constexpr iterator begin() const noexcept
		{
			const auto data = data_;
			return { data, data + size(), data };
		}

		constexpr iterator end() const noexcept
		{
			const auto data = data_;
			const auto endData = data + size_;
			return { data, endData, endData };
		}

		constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator{ end() }; }
		constexpr reverse_iterator rend() const noexcept { return reverse_iterator{ begin() }; }

	private:
		
		pointer data_;
		size_type size_;

		constexpr auto make_subspan(size_type offset, size_type count) const noexcept
		{
			if (count == dynamic_extent) { return member_span{ data() + offset, size() - offset }; }

			return member_span{ data() + offset, count };
		}
	};

}