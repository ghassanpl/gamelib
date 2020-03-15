#include "TextField.h"
#include "../Includes/Format.h"
#include "../../../string_ops/include/string_ops.h"
#include <glm/ext.hpp>
#include <glm/vec4.hpp>
#include <charconv>
#include <allegro5/allegro_color.h>

namespace gamelib
{
	using namespace string_ops;

	void Page::AddParagraph(std::string_view str)
	{
		Paragraph* current_paragraph = nullptr;
		std::vector<Style> style_stack;
		size_t index = 0;

		current_paragraph = &mParagraphs.emplace_back(0);
		style_stack.push_back(mDefaultStyle);

		char32_t prev_cp = 0, cp = consume_utf8(str);
		auto beg = str.data();
		while (cp)
		{
			if (cp == '<') /// Parse tag
			{
				auto tag_name = consume_while(str, string_ops::isalnum);
				if (tag_name.empty())
				{
					SetText(fmt::format("#Error at character {}: tag name is empty#", str.data() - beg));
					return;
				}
				trim_whitespace_left(str);

				if (tag_name == "p")
				{
					current_paragraph->EndGlyph = index;
					current_paragraph = &mParagraphs.emplace_back(index);
				}
				else if (tag_name == "img")
				{
					if (auto name = ConsumeArg(str))
					{
						const auto line_height = (float)al_get_font_line_height(style_stack.back().Font);
						auto& glyph = mGlyphs.emplace_back(Glyph{
							.Glyph = mImageResolver(name.value()),
							.LineHeight = line_height
						});
						if (glyph.Glyph.bitmap == nullptr)
						{
							SetText(fmt::format("#Error at character {}: image name '{}' does not resolve to an image#", str.data() - beg, name.value()));
							return;
						}
						const auto original_size = vec2{ glyph.Glyph.w, glyph.Glyph.h };
						const auto ratio = line_height / original_size.y;
						glyph.Bounds.set_size(original_size * ratio);
						index++;
					}
					else
					{
						SetText(fmt::format("#Error at character {}: expected '=imagename'#", str.data() - beg));
						return;
					}
				}
				else if (tag_name == "pop")
				{
					if (style_stack.size() == 1)
					{
						SetText(fmt::format("#Error at character {}: one pop too many#", str.data() - beg));
						return;
					}
					style_stack.pop_back();
				}
				else if (tag_name == "font")
				{
					if (auto font_name = ConsumeArg(str))
					{
						auto font = mFontResolver(font_name.value());
						if (!font)
						{
							SetText(fmt::format("#Error at character {}: font name '{}' invalid#", str.data() - beg, font_name.value()));
							return;
						}
						style_stack.emplace_back(style_stack.back()).Font = font;
					}
					else
					{
						SetText(fmt::format("#Error at character {}: expected '=fontname'#", str.data() - beg));
						return;
					}
				}
				else if (tag_name == "color")
				{
					if (auto color_str = ConsumeArg(str))
					{
						auto color = std::string{ color_str.value() };
						vec4 col{ 1,1,1,1 };
						/// al_color_name_to_rgb
						/// al_color_html_to_rgb
						if (color[0] == '#')
						{
							uint32_t val = 0;
							auto res = std::from_chars(color.data() + 1, color.data() + color.size() - 1, val, 16);
							auto len = res.ptr - color.data();
							if (res.ec != std::errc{})
								len = 0;
							switch (len)
							{
							case 3:
								col = { ((val >> 8) & 0xF) / 15.0f, ((val >> 4) & 0xF) / 15.0f, (val & 0xF) / 15.0f, 1.0f };
								break;
							case 6:
								col = { ((val >> 16) & 0xFF) / 255.0f, ((val >> 8) & 0xFF) / 255.0f, (val & 0xFF) / 255.0f, 1.0f };
								break;
							case 8:
								col = { ((val >> 24) & 0xFF) / 255.0f, ((val >> 16) & 0xFF) / 255.0f, ((val >> 8) & 0xFF) / 255.0f, (val & 0xFF) / 255.0f };
								break;
							default:
								SetText(fmt::format("#Error at character {}: expected RGB color (#RRGGBBAA/#RRGGBB/#RGB/colorname)#", str.data() - beg));
								return;
							}
						}
						else
							al_color_name_to_rgb(color.c_str(), &col.r, &col.g, &col.b);

						style_stack.emplace_back(style_stack.back()).Color = col;
					}
					else
					{
						SetText(fmt::format("#Error at character {}: expected '=color'#", str.data() - beg));
						return;
					}
				}
				else
				{
					SetText(fmt::format("#Error at character {}: expected tag name, got '{}'#", str.data() - beg, tag_name));
					return;
				}

				trim_whitespace_left(str);
				if (!consume(str, '>'))
				{
					SetText(fmt::format("#Error at character {}: expected '>'#", str.data() - beg));
					return;
				}

				prev_cp = 0;
				cp = consume_utf8(str);
				continue;
			}

			auto& glyph = mGlyphs.emplace_back(Glyph{
				.Codepoint = (char32_t)cp,
				.Color = ToAllegro(style_stack.back().Color),
				.LineHeight = (float)al_get_font_line_height(style_stack.back().Font)
			});
			al_get_glyph(style_stack.back().Font, prev_cp, cp, &glyph.Glyph);
			glyph.Bounds.p1 = vec2{ glyph.Glyph.offset_x + glyph.Glyph.kerning, glyph.Glyph.offset_y };
			glyph.Bounds.set_size(glyph.Glyph.w, glyph.Glyph.h);

			index++;
			prev_cp = cp;
			cp = consume_utf8(str);
		}

		current_paragraph->EndGlyph = index;

		mNeedsReflow = true;
	}

	void Page::SetText(std::string_view str)
	{
		Clear();
		AddParagraph(str);
	}
	
	void Page::Reflow()
	{
		float y = 0;
		for (auto& paragraph : mParagraphs)
		{
			auto paragraph_glyphs = gsl::span<Glyph>{ mGlyphs }.subspan(paragraph.StartGlyph, paragraph.EndGlyph - paragraph.StartGlyph);
			paragraph.Reflow(paragraph_glyphs, mBounds.width());
			paragraph.Bounds.set_position(0, y);
			y += paragraph.Bounds.height();
		}

		for (auto& p : mParagraphs)
		{
			vec2 ppos = p.Bounds.p1 + mBounds.p1;
			for (auto& l : p.Lines)
			{
				vec2 lpos = l.Bounds.p1 + ppos;
				for (auto& g : l.Glyphs)
				{
					if (!g.Glyph.bitmap) continue;

					g.Bounds += lpos;
				}
			}
		}

		mNeedsReflow = false;
	}

	std::optional<std::string_view> Page::ConsumeArg(std::string_view& str)
	{
		if (string_ops::consume(str, '='))
			if (auto ret = string_ops::consume_while(str, [](int cp) { return !string_ops::isspace(cp) && cp!='>'; }); !ret.empty())
				return ret;
		return std::nullopt;
	}
	
	void Paragraph::Reflow(gsl::span<Glyph> glyphs, float max_width)
	{
		Lines.clear();

		vec2 pos = {};
		float line_height = 0;
		auto current_line = &Lines.emplace_back();
		current_line->Glyphs = glyphs;
		for (intptr_t i=0;i<glyphs.size(); i++)
		{
			auto& glyph = glyphs[i];
			if (glyph.Codepoint == '\n' || pos.x + glyph.Glyph.advance >= max_width)
			{
				current_line->Bounds.set_size(pos.x, line_height);
				pos.x = 0;
				pos.y += line_height;
				line_height = 0;
				current_line->Glyphs = { current_line->Glyphs.data(), &glyph };
				current_line = &Lines.emplace_back();
				current_line->Bounds.set_position(0, pos.y);

				if (glyph.Codepoint != '\n')
				{
					current_line->Glyphs = { &glyph, glyphs.data()+glyphs.size() };
				}
				else
				{
					current_line->Glyphs = { (&glyph) + 1, glyphs.data() + glyphs.size()};
					continue;
				}
			}

			glyph.Bounds.p1.x += pos.x;
			glyph.Bounds.p2.x += pos.x;
			pos.x += glyph.Glyph.advance;
			line_height = std::max(line_height, glyph.LineHeight);
		}

		current_line->Bounds.set_size(pos.x, line_height);
		Bounds.set_size(max_width, current_line->Bounds.p2.y);
	}

	void Page::Draw()
	{
		if (mNeedsReflow)
			Reflow();

		for (auto& g : mGlyphs)
		{
			if (!g.Glyph.bitmap) continue;

			al_draw_tinted_scaled_bitmap(
				g.Glyph.bitmap,
				g.Color,
				g.Glyph.x, g.Glyph.y, g.Glyph.w, g.Glyph.h,
				g.Bounds.p1.x, g.Bounds.p1.y, g.Bounds.width(), g.Bounds.height(),
				0
			);
		}
	}

}