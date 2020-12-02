#pragma once

#include <random>
#include <concepts>
#include "Includes/GLM.h"
#include "Common.h"

namespace gamelib::random
{
	/// TODO: Tests

	template <typename RANDOM>
	uint64_t Integer(RANDOM& rng)
	{
		static const std::uniform_int_distribution<uint64_t> dist;
		return dist(rng);
	}

	template <typename REAL = double, typename RANDOM>
	REAL Percentage(RANDOM& rng)
	{
		static const std::uniform_real_distribution<REAL> dist;
		return dist(rng);
	}

	template <typename RANDOM>
	uint64_t Dice(RANDOM& rng, uint64_t n_sided)
	{
		if (n_sided < 2) return 0;
		std::uniform_int_distribution<uint64_t> dist{ 0, n_sided - 1 };
		return dist(rng) + 1;
	}

	template <typename RANDOM, uint64_t N_SIDED>
	requires (N_SIDED >= 2)
	uint64_t Dice(RANDOM& rng)
	{
		static const std::uniform_int_distribution<uint64_t> dist{ 0, N_SIDED - 1 };
		return dist(rng) + 1;
	}

	namespace
	{
		inline static std::default_random_engine DefaultRandomEngine;
	}

	inline int operator""_d2(unsigned long long int) { return (int)Dice(DefaultRandomEngine, 2); }
	inline int operator""_d4(unsigned long long int) { return (int)Dice(DefaultRandomEngine, 4); }
	inline int operator""_d6(unsigned long long int) { return (int)Dice(DefaultRandomEngine, 6); }
	inline int operator""_d8(unsigned long long int) { return (int)Dice(DefaultRandomEngine, 8); }
	inline int operator""_d10(unsigned long long int) { return (int)Dice(DefaultRandomEngine, 10); }
	inline int operator""_d12(unsigned long long int) { return (int)Dice(DefaultRandomEngine, 12); }
	inline int operator""_d20(unsigned long long int) { return (int)Dice(DefaultRandomEngine, 20); }
	inline int operator""_d100(unsigned long long int) { return (int)Dice(DefaultRandomEngine, 100); }

	template <typename RANDOM, typename T>
	requires is_any_of_v<T, short, int, long, long long, unsigned short, unsigned int, unsigned long, unsigned long long>
	T IntegerRange(RANDOM& rng, T from, T to)
	{
		if (from >= to) return T{};
		std::uniform_int_distribution<T> dist{ from, to };
		return dist(rng);
	}

	template <typename RANDOM, std::floating_point T>
	T RealRange(RANDOM& rng, T from, T to)
	{
		if (from >= to) return T{};
		std::uniform_real_distribution<T> dist{ from, to };
		return dist(rng);
	}

	template <typename RANDOM, typename T>
	T Range(RANDOM& rng, T from, T to)
	{
		if (from >= to) return T{};

		if constexpr (std::is_enum_v<T>)
		{
			std::uniform_int_distribution<std::underlying_type_t<T>> dist(from, to);
			return (T)dist(rng);
		}
		else if constexpr (std::is_floating_point_v<T>)
			return RealRange(rng, from, to);
		else if constexpr (is_any_of_v<T, short, int, long, long long, unsigned short, unsigned int, unsigned long, unsigned long long>)
			return IntegerRange(rng, from, to);
		else
		{
			static_assert(false, "Range only works on arithmetic types (non-char-ones) and enums");
		}
	}

	template <typename RANDOM>
	radians_t Radians(RANDOM& rng)
	{
		static const inline std::uniform_real_distribution<radians_t::base_type> dist{ radians_t::base_type{}, glm::two_pi<radians_t::base_type>() };
		return radians_t{ dist(rng) };
	}

	template <std::floating_point T, typename RANDOM>
	T Radians(RANDOM& rng)
	{
		static const std::uniform_real_distribution<T> dist{ T{}, glm::two_pi<T>() };
		return dist(rng);
	}

	template <typename RANDOM>
	degrees_t Degrees(RANDOM& rng)
	{
		static const std::uniform_real_distribution<degrees_t::base_type> dist{ degrees_t::base_type{0}, degrees_t::base_type{360} };
		return degrees_t{ dist(rng) };
	}

	template <std::floating_point T, typename RANDOM>
	T Degrees(RANDOM& rng)
	{
		static const std::uniform_real_distribution<T> dist{ T{}, T{360} };
		return dist(rng);
	}

	template <std::floating_point T = float, typename RANDOM>
	glm::tvec2<T> UnitVector(RANDOM& rng)
	{
		return glm::rotate(glm::tvec2<T>{ T{ 1 }, T{ 0 } }, Radians<T>(rng));
	}

	template <typename RANDOM>
	ivec2 Neighbor(RANDOM& rng)
	{
		static const inline std::uniform_int_distribution<RANDOM::result_type> dist{ 0, 3 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 0.0f };
		case 1: return { 0.0f, 1.0f };
		case 2: return { -1.0f, 0.0f };
		case 3: return { 0.0f, -1.0f };
		}
		return {};
	}

	template <typename RANDOM>
	ivec2 DiagonalNeighbor(RANDOM& rng)
	{
		static const inline std::uniform_int_distribution<RANDOM::result_type> dist{ 0, 3 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 1.0f };
		case 1: return { -1.0f, 1.0f };
		case 2: return { -1.0f, -1.0f };
		case 3: return { 1.0f, -1.0f };
		}
		return {};
	}

	template <typename RANDOM>
	ivec2 Surrounding(RANDOM& rng)
	{
		static const inline std::uniform_int_distribution<RANDOM::result_type> dist{ 0, 7 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 0.0f };
		case 1: return { 0.0f, 1.0f };
		case 2: return { -1.0f, 0.0f };
		case 3: return { 0.0f, -1.0f };
		case 4: return { 1.0f, 1.0f };
		case 5: return { -1.0f, 1.0f };
		case 6: return { -1.0f, -1.0f };
		case 7: return { 1.0f, -1.0f };
		}
		return {};
	}
	
	template <typename RANDOM>
	bool WithProbability(RANDOM& rng, double probability)
	{
		return Percentage(rng) < glm::saturate(probability);
	}

	template <typename RANDOM>
	bool OneIn(RANDOM& rng, size_t n)
	{
		if (n == 0) return false;
		return WithProbability(rng, 1.0 / double(n));
	}

	template <typename RANDOM, typename T>
	void Shuffle(RANDOM& rng, T& cont)
	{
		using std::begin;
		using std::end;
		std::shuffle(begin(cont), end(cont), rng);
	}

	template <typename RANDOM, typename T>
	auto Iterator(RANDOM& rng, T& cont)
	{
		using std::size;
		using std::begin;
		return begin(cont) + IntegerRange(rng, 0, (int64_t)size(cont) - 1);
	}

	template <typename RANDOM, typename T>
	auto* Element(RANDOM& rng, T& cont)
	{
		using std::end;
		auto result = Iterator(rng, cont);
		return (result != end(cont)) ? std::to_address(result) : nullptr;
	}
}
