#pragma once

#include "Includes/GLM.h"
#include <glm/gtx/color_encoding.hpp>

namespace gamelib
{
	using glm::vec4;

	namespace Colors
	{
#define DEF_COLOR(name, r, g, b) \
	inline constexpr vec4 Get##name(float alpha) { return vec4{ float(r), float(g), float(b), alpha }; } \
	inline constexpr vec4 name = Get##name(1.0f); \
	inline constexpr vec4 GetDark##name(float alpha) { return vec4{ float(r) * 0.5f, float(g) * 0.5f, float(b) * 0.5f, alpha }; } \
	inline constexpr vec4 Dark##name = GetDark##name(1.0f);
		DEF_COLOR(Red, 1, 0, 0)
		DEF_COLOR(Green, 0, 1, 0)
		DEF_COLOR(Blue, 0, 0, 1)
		DEF_COLOR(Yellow, 1, 1, 0)
		DEF_COLOR(Magenta, 1, 0, 1)
		DEF_COLOR(Cyan, 0, 1, 1)
		DEF_COLOR(Black, 0, 0, 0)
		DEF_COLOR(White, 1, 1, 1)
		DEF_COLOR(Gray, 0.5f, 0.5f, 0.5f)
		DEF_COLOR(Grey, 0.5f, 0.5f, 0.5f)
		DEF_COLOR(LightGray, 0.75f, 0.75f, 0.75f)
		DEF_COLOR(LightGrey, 0.75f, 0.75f, 0.75f)
		inline constexpr vec4 Transparent = GetBlack(0.0f);
		#undef DEF_COLOR
	}

	inline vec4 Saturated(vec4 const& color)
	{
		return glm::clamp(vec4(color.r, color.g, color.b, color.a), vec4{ 0,0,0,0 }, vec4{ 1,1,1,1 });
	}

	inline vec4 Lighten(vec4 const& color, float coef)
	{
		const auto rgb_max = glm::max(color.r, glm::max(color.g, color.b));
		const auto lighter = color * (1.0f / rgb_max);
		const auto dif = rgb_max;
		return Saturated(vec4(lighter.r + dif * coef, lighter.g + dif * coef, lighter.b + dif * coef, 1.0f) * rgb_max);
	}

	/// NOTE: `contrast` here is between 0.0 and 1.0
	inline vec4 Contrast(vec4 const& color, float contrast)
	{
		const auto t = (1.0f - contrast) * 0.5f;
		return vec4(color.r*contrast + t, color.g*contrast + t, color.b*contrast + t, color.a);
	}

	/// NOTE: `contrast` here is between -1.0 and 1.0
	inline vec4 Contrast2(vec4 const& color, float contrast)
	{
		static const auto m = 1.0156862745098039215686274509804;
		const auto t = (m * (contrast + 1.0f)) / (m - contrast);
		return vec4(t * (color.r - 0.5f) + 0.5f, t * (color.g - 0.5f) + 0.5f, t * (color.b - 0.5f) + 0.5f, color.a);
	}

	inline vec4 GammaCorrect(vec4 const& color, const float gamma)
	{
		const auto gamma_correct = 1.0f / gamma;
		return { std::pow(color.r, gamma_correct), std::pow(color.g, gamma_correct),std::pow(color.b, gamma_correct), color.a };
	}

	inline vec4 Inverted(vec4 const& color)
	{
		return vec4(1.0f - color.r, 1.0f - color.g, 1.0f - color.b, color.a);
	}

	inline vec4 Contrasting(vec4 const& color)
	{
		return vec4(fmod(color.r + 0.5f, 1.0f), fmod(color.g + 0.5f, 1.0f), fmod(color.b + 0.5f, 1.0f), color.a);
	}

}