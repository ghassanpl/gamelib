#pragma once

#include <random>
#include "Includes/GLM.h"
#include "Common.h"

namespace gamelib::random
{
	template <typename RANDOM>
	uint64_t GetRandomInt(RANDOM& rng)
	{
		static const inline std::uniform_int_distribution<uint64_t> dist;
		return dist(rng);
	}

	template <typename RANDOM>
	double GetRandom01(RANDOM& rng)
	{
		static const inline std::uniform_real_distribution<double> dist;
		return dist(rng);
	}

	template <typename RANDOM>
	uint64_t Dice(RANDOM& rng, size_t n_sided)
	{
		if (n_sided < 2) return 0;
		std::uniform_int_distribution<uint64_t> dist{ 0, n_sided - 1 };
		return dist(rng);
	}

	template <typename RANDOM>
	int64_t GetRandomRange(RANDOM& rng, int64_t from, int64_t to)
	{
		if (from >= to) return 0;
		std::uniform_int_distribution<int64_t> dist{ from, to };
		return dist(rng);
	}

	template <typename RANDOM>
	double GetRandomRange(RANDOM& rng, double from, double to)
	{
		if (from >= to) return 0;
		std::uniform_real_distribution<double> dist{ from, to };
		return dist(rng);
	}

	template <typename RANDOM>
	radians_t GetRandomRadians(RANDOM& rng)
	{
		static const inline std::uniform_real_distribution<float> dist{ 0.0f, 3.14159265358979323846f };
		return radians_t{ dist(rng) };
	}

	template <typename RANDOM>
	degrees_t GetRandomDegrees(RANDOM& rng)
	{
		static const inline std::uniform_real_distribution<float> dist{ 0.0f, 360.0f };
		return degrees_t{ dist(rng) };
	}

	template <typename RANDOM>
	vec2 GetRandomUnitVector(RANDOM& rng)
	{
		return glm::rotate(vec2{ 1.0f, 0.0f }, GetRandomRotation(rng));
	}

	template <typename RANDOM>
	ivec2 GetRandomNeighbor(RANDOM& rng)
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
	ivec2 GetRandomDiagonalNeighbor(RANDOM& rng)
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
	ivec2 GetRandomSurrounding(RANDOM& rng)
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
		return GetRandom01(rng) < glm::saturate(probability);
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
		std::shuffle(std::begin(cont), std::end(cont), rng);
	}

	template <typename RANDOM, typename T>
	auto RandomIt(RANDOM& rng, T& cont)
	{
		return std::begin(cont) + GetRandomRange(rng, 0, (int64_t)std::size(cont) - 1);
	}
}
