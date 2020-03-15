#pragma once

#include "../Common.h"
#include "../Includes/Allegro.h"
#include <allegro5/allegro_font.h>
#include "../Includes/GLM.h"
#include "../Colors.h"
#include <gsl/span>
#include <vector>
#include <functional>
#include <optional>

namespace gamelib
{

	struct Glyph
	{
		rec2 Bounds{};
		ALLEGRO_GLYPH Glyph;
		char32_t Codepoint = 0;
		ALLEGRO_COLOR Color;
		float LineHeight = 0;
	};

	struct Line
	{
		rec2 Bounds{};
		gsl::span<Glyph> Glyphs;
	};

	struct Paragraph
	{
		Paragraph(intptr_t sg) : StartGlyph(sg) {}

		rec2 Bounds{};
		std::vector<Line> Lines;

		intptr_t StartGlyph = 0;
		intptr_t EndGlyph = 0;

		void Reflow(gsl::span<Glyph> glyphs, float max_width);
	};

	struct Style
	{
		ALLEGRO_FONT* Font = nullptr;
		vec4 Color = Colors::White;
		/// TODO: flags for text-decoration/subscript,etc
		/// TODO: letter-spacing
	};

	struct Page
	{
		rec2 Bounds;
		std::vector<Paragraph> Paragraphs;
		std::vector<Glyph> Glyphs;

		std::function<ALLEGRO_BITMAP*(std::string_view)> ImageResolver;
		std::function<ALLEGRO_FONT*(std::string_view)> FontResolver;

		Style DefaultStyle;

		void SetText(std::string_view str);

	private:

		void Reflow();

		std::optional<std::string_view> ConsumeArg(std::string_view& str);

	};

}