#pragma once

#include "../Common.h"
#include "../Includes/Allegro.h"
#include <allegro5/allegro_font.h>
#include "../Includes/GLM.h"
#include "../Colors.h"
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
		ALLEGRO_COLOR Color{ 1,1,1,1 };
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

		/// TODO: Alignment
		/// TODO: Line spacing
		/// TODO: Margins

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
		/// TODO: Alignment, Paragraph spacing

		void SetBounds(rec2 const& bounds) { mBounds = bounds; mNeedsReflow = true; }
		rec2 const& GetBounds() const { return mBounds; }

		void SetDefaultStyle(Style const& style) { mDefaultStyle = style; mNeedsReflow = true; }
		Style const& GetDefaultStyle() const { return mDefaultStyle; }

		void SetImageResolver(std::function<ALLEGRO_GLYPH(std::string_view)> resolver) { mImageResolver = resolver; mNeedsReflow = true; }
		void SetImageResolver(std::function<ALLEGRO_BITMAP* (std::string_view)> resolver);
		void SetFontResolver(std::function<ALLEGRO_FONT*(std::string_view)> resolver) { mFontResolver = resolver; mNeedsReflow = true; }

		void Clear()
		{
			mGlyphs.clear();
			mParagraphs.clear();
			mNeedsReflow = false;
		}
		void SetText(std::string_view str);
		void AddParagraph(std::string_view str);

		void Draw();

		gsl::span<Glyph const> Glyphs() const { return mGlyphs; }

	private:

		rec2 mBounds;
		std::vector<Paragraph> mParagraphs;
		std::vector<Glyph> mGlyphs;

		std::function<ALLEGRO_GLYPH(std::string_view)> mImageResolver = [](std::string_view) { return ALLEGRO_GLYPH{}; };
		std::function<ALLEGRO_FONT*(std::string_view)> mFontResolver = [](std::string_view) { return nullptr; };

		Style mDefaultStyle;

		bool mNeedsReflow = true;

		void Reflow();

		std::optional<std::string_view> ConsumeArg(std::string_view& str);

	};

}