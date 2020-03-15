#include "TextField.h"
#include "../Includes/Format.h"
#include "../../../string_ops/include/string_ops.h"
#include <glm/ext.hpp>
#include <glm/vec4.hpp>
#include <charconv>

namespace gamelib
{
	void Page::SetText(std::string_view str)
	{
		using namespace string_ops;

		Glyphs.clear();
		Paragraphs.clear();

		Paragraph* current_paragraph = nullptr;
		std::vector<Style> style_stack;
		size_t index = 0;

		current_paragraph = &Paragraphs.emplace_back(0);
		style_stack.push_back(DefaultStyle);

		char32_t prev_cp = 0, cp = consume_utf8(str);
		auto beg = str.begin();
		while (cp)
		{
			if (cp == '<') /// Parse tag
			{
				auto tag_name = consume_while(str, string_ops::isalnum);
				if (tag_name.empty())
				{
					SetText(fmt::format("#Error at character {}: tag name is empty#", str.begin() - beg));
					return;
				}
				trim_whitespace_left(str);

				if (tag_name == "p")
				{
					current_paragraph->EndGlyph = index;
					current_paragraph = &Paragraphs.emplace_back(index);
				}
				else if (tag_name == "img")
				{
					if (auto name = ConsumeArg(str))
					{
						auto& glyph = Glyphs.emplace_back();
						glyph.Glyph.bitmap = ImageResolver(name.value());
						/// img height = al_get_font_line_height(style_stack.back().Font)
						index++;
					}
					else
					{
						SetText(fmt::format("#Error at character {}: expected '='<image name>#", str.begin() - beg));
						return;
					}
				}
				else if (tag_name == "pop")
				{
					style_stack.pop_back();
				}
				else if (tag_name == "font")
				{
					if (auto font_name = ConsumeArg(str))
					{
						auto font = FontResolver(font_name.value());
						if (!font)
						{
							SetText(fmt::format("#Error at character {}: font name '{}' invalid#", str.begin() - beg, font_name.value()));
							return;
						}
						style_stack.emplace_back(style_stack.back()).Font = font;
					}
					else
					{
						SetText(fmt::format("#Error at character {}: expected '='<font name>#", str.begin() - beg));
						return;
					}
				}
				else if (tag_name == "color")
				{
					if (auto color_str = ConsumeArg(str))
					{
						auto color = color_str.value();
						if (color.size() != 8)
						{
							SetText(fmt::format("#Error at character {}: expected RGB color (RRGGBBAA)#", str.begin() - beg));
							return;
						}
						uint32_t val = 0;
						auto res = std::from_chars(color.data(), color.data() + color.size(), val, 16);
						if (res.ec != std::errc{} || res.ptr != color.data() + 8)
						{
							SetText(fmt::format("#Error at character {}: expected RGB color (RRGGBBAA)#", str.begin() - beg));
							return;
						}
						style_stack.emplace_back(style_stack.back()).Color = vec4{
							((val >> 24) & 0xFF) / 255.0f, 
							((val >> 16) & 0xFF) / 255.0f, 
							((val >> 8) & 0xFF) / 255.0f, 
							(val & 0xFF) / 255.0f
						};
					}
					else
					{
						SetText(fmt::format("#Error at character {}: expected '='<color>#", str.begin() - beg));
						return;
					}
				}
				else
				{
					SetText(fmt::format("#Error at character {}: expected tag name, got '{}'#", str.begin() - beg, tag_name));
					return;
				}

				trim_whitespace_left(str);
				if (!consume(str, '>'))
				{
					SetText(fmt::format("#Error at character {}: expected '>'#", str.begin() - beg));
					return;
				}

				prev_cp = 0;
				cp = consume_utf8(str);
				continue;
			}

			auto& glyph = Glyphs.emplace_back(Glyph{
				.Codepoint = (char32_t)cp,
				.Color = ToAllegro(style_stack.back().Color),
				.LineHeight = (float)al_get_font_line_height(style_stack.back().Font)
			});
			al_get_glyph(style_stack.back().Font, prev_cp, cp, &glyph.Glyph);

			index++;
			prev_cp = cp;
			cp = consume_utf8(str);
		}

		current_paragraph->EndGlyph = index;

		Reflow();
	}
	
	void Page::Reflow()
	{
		float y = 0;
		for (auto& paragraph : Paragraphs)
		{
			auto paragraph_glyphs = gsl::span<Glyph>{ Glyphs }.subspan(paragraph.StartGlyph, paragraph.EndGlyph - paragraph.StartGlyph);
			paragraph.Reflow(paragraph_glyphs, Bounds.width());
			paragraph.Bounds.set_position(0, y);
			y += paragraph.Bounds.height();
		}

		for (auto& p : Paragraphs)
		{
			vec2 ppos = p.Bounds.p1 + Bounds.p1;
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
	}

	std::optional<std::string_view> Page::ConsumeArg(std::string_view& str)
	{
		if (string_ops::consume(str, '='))
			if (auto ret = string_ops::consume_while(str, string_ops::isalnum); !ret.empty())
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

			glyph.Bounds.p1.x = pos.x;
			glyph.Bounds.p1 += vec2{ glyph.Glyph.offset_x + glyph.Glyph.kerning, glyph.Glyph.offset_y };
			glyph.Bounds.set_size(glyph.Glyph.w, glyph.Glyph.h);
			pos.x += glyph.Glyph.advance;
			line_height = std::max(line_height, glyph.LineHeight);
		}

		current_line->Bounds.set_size(pos.x, line_height);
		Bounds.set_size(max_width, current_line->Bounds.p2.y);
	}
}