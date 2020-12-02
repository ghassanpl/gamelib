#pragma once
//#define GLM_FORCE_SSE2 1
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/color_space.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/extend.hpp>
#include <iostream>

using glm::vec2;
using glm::ivec2;
using glm::uvec2;
using Color = glm::vec4;

namespace gamelib
{

	/// Shamelessly stolen from: https://www.embeddeduse.com/2019/08/26/qt-compare-two-floats/
	template <std::floating_point T>
	inline constexpr bool epsilonEqual(T a, T b, T epsilon)
	{
		const auto absA = std::abs(a);
		const auto absB = std::abs(b);
		const auto diff = std::abs(a - b);

		return (diff <= epsilon) ? true : (diff <= (epsilon * std::max(absA, absB)));
	}

	enum class point_relationship
	{
		outside,
		on_edge,
		inside,
	};

	inline std::ostream& operator<<(std::ostream& strm, point_relationship rel)
	{
		switch (rel)
		{
		case point_relationship::outside: strm << "outside"; return strm;
		case point_relationship::on_edge: strm << "on_edge"; return strm;
		case point_relationship::inside: strm << "inside"; return strm;
		}
		strm << "(invalid value for point_relationship: " << int(rel) << ")";
		return strm;
	}

	template <typename T>
	struct trec2
	{
		glm::tvec2<T> p1 = { 0,0 };
		glm::tvec2<T> p2 = { 0,0 };

		using tvec = glm::tvec2<T>;
		using value_type = T;

		trec2() noexcept = default;
		trec2(tvec a, tvec b) noexcept : p1(a), p2(b) { }
		trec2(T x1, T y1, T x2, T y2) noexcept : p1(x1, y1), p2(x2, y2) { }
		trec2(const trec2&) noexcept = default;
		trec2(trec2&&) noexcept = default;
		template <typename U>
		trec2(const trec2<U>& other) noexcept : p1(glm::tvec2<U>(other.p1)), p2(glm::tvec2<U>(other.p2)) {}
		template <typename U>
		trec2(trec2<U>&& other) noexcept : p1(glm::tvec2<U>(other.p1)), p2(glm::tvec2<U>(other.p2)) {}
		trec2& operator=(const trec2&) noexcept = default;
		trec2& operator=(trec2&&) noexcept = default;

		static trec2 from_size(tvec p, tvec s) noexcept { return { p, p + s }; };
		static trec2 from_size(T x, T y, T w, T h) noexcept { return { x, y, x + w, y + h }; };

		trec2 operator+(tvec op) const noexcept { return { p1 + op, p2 + op }; }
		trec2& operator+=(tvec op) noexcept { p1 += op; p2 += op; return *this; };
		trec2 operator-(tvec op) const noexcept { return { p1 - op, p2 - op }; }
		trec2& operator-=(tvec op) noexcept { p1 -= op; p2 -= op; return *this; };
		trec2 operator*(T op) const noexcept { return { p1 * op, p2 * op }; }
		trec2 operator/(T op) const noexcept { return { p1 / op, p2 / op }; }

		tvec size() const noexcept { return p2 - p1; }
		tvec position() const noexcept { return p1; }
		trec2& set_position(tvec pos) noexcept { p2 += pos - p1; p1 = pos; return *this; }
		trec2& set_position(T x, T y) noexcept { p2.x += x - p1.x; p2.y += y - p1.y; p1 = { x, y }; return *this; }
		trec2& set_size(tvec size) noexcept { p2 = p1 + size; return *this; }
		trec2& set_size(T w, T h) noexcept { p2.x = p1.x + w; p2.y = p1.y + h; return *this; }
		trec2& grow(T by) noexcept { p1.x -= by; p1.y -= by; p2.x += by; p2.y += by; return *this; }
		trec2& shrink(T by) noexcept { return grow(-by); }
		trec2& grow(tvec by) noexcept { p1 -= by; p2 += by; return *this; }
		trec2& shrink(tvec by) noexcept { return grow(-by); }
		trec2 grown(T by) const noexcept { auto copy = *this; copy.p1.x -= by; copy.p1.y -= by; copy.p2.x += by; copy.p2.y += by; return copy; }
		trec2 shrunk(T by) const noexcept { return grown(-by); }
		trec2 grown(tvec by) const noexcept { auto copy = *this; copy.p1 -= by; copy.p2 += by; return copy; }
		trec2 shrunk(tvec by) const noexcept { return grown(-by); }
		trec2 at_position(tvec pos) const noexcept { auto copy = *this; copy.set_position(pos); return copy; }
		trec2 at_position(T x, T y) const noexcept { auto copy = *this; copy.set_position(x, y); return copy; }
		T width() const noexcept { return p2.x - p1.x; };
		T height() const noexcept { return p2.y - p1.y; };
		trec2& set_width(T w) noexcept { p2.x = p1.x + w; return *this; };
		trec2& set_height(T h) noexcept { p2.y = p1.y + h; return *this; };
		T x() const noexcept { return p1.x; }
		T y() const noexcept { return p1.y; }
		T left() const noexcept { return p1.x; }
		T top() const noexcept { return p1.y; }
		T right() const noexcept { return p2.x; }
		T bottom() const noexcept { return p2.y; }
		tvec left_top() const noexcept { return p1; }
		tvec left_bottom() const noexcept { return { p1.x, p2.y }; }
		tvec right_top() const noexcept { return { p2.x, p1.y }; }
		tvec right_bottom() const noexcept { return p2; }
		tvec half_size() const noexcept { return (p2 - p1) / T{ 2 }; }
		tvec center() const noexcept { return p1 + half_size(); }
		trec2& set_center(tvec pos) noexcept { const auto hw = half_size(); p1 = pos - hw; p2 = pos + hw; return *this; }
		trec2 at_center(tvec pos) const noexcept { auto copy = *this; copy.set_center(pos); return copy; }

		trec2 local() const noexcept { return { tvec{}, size() }; }

		vec2 to_rect_space(tvec world_space) const noexcept { return vec2{ world_space - p1 } / vec2{ size() }; }
		tvec to_world_space(vec2 rect_space) const noexcept { return tvec{ rect_space * vec2{ size() } } + p1; }

		trec2 include(tvec pt) const noexcept { return { glm::min(p1, pt), glm::max(p2, pt) }; };
		trec2 include(trec2 rec) const noexcept { return this->include(rec.p1).include(rec.p2); };

		bool intersects(trec2 const& other) const noexcept
		{
			return (left() <= other.right() && other.left() <= right() && top() <= other.bottom() && other.top() <= bottom());
		}

		bool contains(glm::vec<2, T> const& other) const noexcept
		{
			return other.x >= p1.x && other.y >= p1.y && other.x < p2.x && other.y < p2.y;
		}

		bool is_valid() const noexcept
		{
			return p1.x <= p2.x && p1.y <= p2.y;
		}

		trec2 valid() const noexcept
		{
			auto copy = *this;
			if (copy.p1.x > copy.p2.x) std::swap(copy.p1.x, copy.p2.x);
			if (copy.p1.y > copy.p2.y) std::swap(copy.p1.y, copy.p2.y);
			return copy;
		}

		trec2& make_valid() noexcept
		{
			if (p1.x > p2.x) std::swap(p1.x, p2.x);
			if (p1.y > p2.y) std::swap(p1.y, p2.y);
			return *this;
		}


		point_relationship classify(vec2 point, T edge_epsilon = T{ 0 }) const
		{
			if (epsilonEqual(point.x, p1.x, edge_epsilon) || epsilonEqual(point.y, p1.y, edge_epsilon) || epsilonEqual(point.x, p2.x, edge_epsilon) || epsilonEqual(point.y, p2.y, edge_epsilon))
				return point_relationship::on_edge;
			else if (!this->contains(point))
				return point_relationship::outside;
			else
				return point_relationship::inside;
		}

		T calculate_area() const noexcept { return width() * height(); }

		T edge_length() const noexcept { return (width() + height()) * 2; }

		vec2 edge_point_alpha(double edge_progress) const
		{
			edge_progress = glm::fract(edge_progress);
			const auto w = width();
			const auto h = height();
			const auto el = (w + h) * 2;
			const auto d = static_cast<T>(edge_progress * el);
			if (d < w)
				return glm::lerp(this->left_top(), this->right_top(), d / w);
			else if (d < w + h)
				return glm::lerp(this->right_top(), this->right_bottom(), (d - w) / h);
			else if (d < w + h + w)
				return glm::lerp(this->right_bottom(), this->left_bottom(), (d - (w + h)) / w);
			else
				return glm::lerp(this->left_bottom(), this->left_top(), (d - (w + h + w)) / h);
		}

		vec2 edge_point(double edge_pos) const
		{
			const auto w = width();
			const auto h = height();
			edge_pos = fmod(edge_pos, (w + h) * 2);
			if (edge_pos < w)
				return glm::lerp(this->left_top(), this->right_top(), edge_pos / w);
			else if (edge_pos < w + h)
				return glm::lerp(this->right_top(), this->right_bottom(), (edge_pos - w) / h);
			else if (edge_pos < w + h + w)
				return glm::lerp(this->right_bottom(), this->left_bottom(), (edge_pos - (w + h)) / w);
			else
				return glm::lerp(this->left_bottom(), this->left_top(), (edge_pos - (w + h + w)) / h);
		}

		trec2 bounding_box() const noexcept
		{
			return *this;
		}

		vec2 projected(vec2 pt) const
		{
			const auto d = (pt - p1) / size();
			const auto c = glm::round(glm::saturate(d));
			return p1 + c * size();
		}
	};

	template <typename T>
	inline trec2<T> operator+(glm::tvec2<T> op, trec2<T> rec) noexcept { return { rec.p1 + op, rec.p2 + op }; }

	template <typename T>
	inline std::ostream& operator<<(std::ostream& strm, trec2<T> const& b) { return strm << '(' << b.p1.x << ',' << b.p1.y << ',' << b.p2.x << ',' << b.p2.y << ')'; }
}

using rec2 = gamelib::trec2<float>;
using irec2 = gamelib::trec2<int>;

namespace glm
{
	template <typename T>
	inline std::ostream& operator<<(std::ostream& strm, glm::vec<2, T> b) { return strm << "(" << b.x << "," << b.y << ")"; }

	inline auto manhattan(ivec2 a, ivec2 b)
	{
		const auto d = glm::abs(a - b);
		return d.x + d.y;
	}

	inline bool operator<(ivec2 a, ivec2 b) noexcept
	{
		return a.x < b.x || (!(b.x < a.x) && (a.y < b.y || (!(b.y < a.y))));
	}
}

inline float Angle(vec2 vec)
{
	return std::atan2(vec.y, vec.x);
}

struct ivec_hash {
	size_t operator()(ivec2 v) const noexcept {
		return *reinterpret_cast<size_t*>(&v.x);
	}
};

#define GAMELIB_GLM
#ifdef GAMELIB_JSON
#include "../Combos/GLM+JSON.h"
#endif
#ifdef GAMELIB_FORMAT
#include "../Combos/Format+GLM.h"
#endif