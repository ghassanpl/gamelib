#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
/// https://glm.g-truc.net/0.9.9/api/modules.html

namespace gamelib
{

	///************************************************************************/
	/// Maths
	///************************************************************************/
	
	/// shhhhhhhh (in radians)
	template <typename T>
	constexpr T DeltaAngle(const T& rad1, const T& rad2)
	{
		auto delta = glm::mod(rad2, glm::two_pi<T>()) - glm::mod(rad1, glm::two_pi<T>());
		if (delta > glm::pi<T>())
			delta = delta - glm::two_pi<T>();
		else if (delta < -glm::pi<T>())
			delta = delta + glm::two_pi<T>();
		return delta;
	}

	/// Ensures angle (in radians) is always between 0 and 2*PI
	template <typename T>
	constexpr T EnsurePositiveAngle(T radians) { return glm::mod(glm::mod(radians, glm::two_pi<T>()) + glm::two_pi<T>(), glm::two_pi<T>()); }


	///************************************************************************/
	/// Simple animations
	///************************************************************************/
	
	/// TODO: Shake, Flicker (White noise)

	/// Wave
	template <typename P, typename V, typename S, typename T>
	auto Wave(V amplitude, P phase_shift, S speed, T t)
	{
		return amplitude * glm::sin(phase_shift + speed*t*glm::pi());
	}

	template <typename P, typename V, typename S, typename T>
	auto TriangleWave(V amplitude, P phase_shift, S speed, T t)
	{
		const auto x = T(1) - std::abs(std::fmod(t * speed - phase_shift, T(2)) - T(1));
		return amplitude * x;
	}

	/// Smoothly moves the current value towards the destination value, at the specified speed.
	template <typename T, typename A_T>
	constexpr inline const T& Approach(T& current, const T dest, const A_T speed)
	{
		return current += (dest - current) * std::min(A_T(1), speed);
	}

	template <typename T, typename A_T>
	constexpr inline const T& ApproachMax(T& current, const T dest, const A_T speed, const T max_change)
	{
		return current += glm::clamp(-max_change, max_change, (dest - current) * std::min(A_T(1), speed));
	}

	/// Linearly moves the current value towards the destination value, at the specified speed.
	template <typename T, typename S>
	constexpr inline bool MoveTowards(T& value, const T towards, const S speed)
	{
		if (value < towards)
			value = glm::min(T(value + speed), towards);
		else if (value > towards)
			value = glm::max(T(value - speed), towards);
		else return false;
		return true;
	}

	///************************************************************************/
	/// Interpolation
	///************************************************************************/

	enum class InterpolationFunction
	{
		None,
		Round,
		Linear,
		Quadratic,
		Cubic,
		Cosine,
		InverseQuadratic,
		Smoothstep,
		Smoothstep2,
		Smoothstep3,
		Overshoot
	};

	template <typename T>
	inline T Interpolate(InterpolationFunction function, T t)
	{
		switch (function)
		{
		case InterpolationFunction::None:
			return T(0);
		case InterpolationFunction::Round:
			return glm::round(t);
		case InterpolationFunction::Linear:
			return t;
		case InterpolationFunction::Quadratic:
			return t * t;
		case InterpolationFunction::Cubic:
			return t * t*t;
		case InterpolationFunction::Cosine:
			return (T(1) - glm::cos(t*glm::pi<T>())) / T(2);
		case InterpolationFunction::InverseQuadratic:
			return (T(1) - (T(1) - t)*(T(1) - t));
		case InterpolationFunction::Smoothstep:
			return glm::smoothstep(T(0), T(1), t);
		case InterpolationFunction::Smoothstep2:
			return glm::smoothstep(T(0), T(1), glm::smoothstep(T(0), T(1), t));
		case InterpolationFunction::Smoothstep3:
			return glm::smoothstep(T(0), T(1), glm::smoothstep(T(0), T(1), glm::smoothstep(T(0), T(1), t)));
		case InterpolationFunction::Overshoot:
			return (T(0.5) * T(
				T(11) * t +
				(T(-20) - T(5) * T(0) + T(4) * T(1) - T(1)) * t * t +
				(T(10) + T(0) - T(3) * T(1) + T(1)) * t * t * t)
				);
		default:
			return t;
		}
	}

	template <InterpolationFunction FUNCTION, typename T>
	constexpr inline T Interpolate(T t)
	{
		return Interpolate(FUNCTION, t);
	}

	template <typename T, typename DT>
	constexpr inline T InterpolateBetween(InterpolationFunction function, const T& first, const T& second, DT t)
	{
		return first + (second - first) * Interpolate(function, t);
	}

	template <InterpolationFunction FUNCTION, typename T, typename DT>
	constexpr inline T InterpolateBetween(const T& first, const T& second, DT t)
	{
		return first + (second - first) * Interpolate<FUNCTION>(t);
	}

	///************************************************************************/
	/// Displayable
	/// Interpolates a value between it's current value and the displayed one,
	/// so we can see a change in value.
	/// TODO: Incorporate an interpolation method
	///************************************************************************/

	template <typename T, bool ANIMATE_INCREASE = true, bool ANIMATE_DECREASE = true>
	struct Displayable
	{
		T Value;
		double DisplayValue;

		Displayable() = default;
		Displayable(const T& v) : Value(v), DisplayValue(v) {}
		Displayable(const Displayable& v) = default;
		Displayable& operator=(const Displayable& other) = default;
		Displayable& operator=(const T& other) { Value = other; return *this; }
		Displayable& operator=(T&& other) noexcept(std::is_nothrow_move_assignable<T>::value) { Value = std::move(other); return *this; }

		void SetValue(const T& val) { Value = val; }

		operator T() const { return Value; }

		Displayable& operator-=(const T& val) { Value -= val; return *this; }
		Displayable& operator+=(const T& val) { Value -= val; return *this; }

		int DisplayRound() const { return int(glm::round(DisplayValue)); }
		double Display() const { return (DisplayValue); }
		template <typename AS>
		AS DisplayAs() const { return AS(DisplayValue); }

		void Set(const T& value)
		{
			Value = value;
			DisplayValue = double(value);
		}

		void Update(double dt, double speed)
		{
			double value = double(Value);

			if (value > DisplayValue)
				Change<ANIMATE_DECREASE>(dt, speed);
			else if (value < DisplayValue)
				Change<ANIMATE_INCREASE>(dt, speed);
		}

		template <bool ANIMATE> inline void Change(double dt, double speed)
		{
			if (ANIMATE)
				Approach(DisplayValue, double(Value), dt*speed);
			else
				DisplayValue = double(Value);
		}

		Displayable& GetSelf() const { return (Displayable&)*this; }
	};

}