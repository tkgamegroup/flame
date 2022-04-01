#pragma once

#include"graphics.h"

namespace flame
{
	namespace graphics
	{
		struct FontAtlas
		{
			ImageViewPtr view;

			virtual ~FontAtlas() {}

			virtual const Glyph& get_glyph(wchar_t unicode, uint font_size) = 0;

			inline uvec2 text_offset(uint font_size, std::wstring_view str)
			{
				auto off = uvec2(0);

				for (auto sh : str)
				{
					if (sh == '\n')
					{
						off.x = 0.f;
						off.y += font_size;
					}
					else if (sh != '\r')
					{
						if (sh == '\t')
							sh = ' ';
						off.x += get_glyph(sh, font_size).advance;
					}
				}
				return off;
			}

			inline uvec2 text_size(uint font_size, std::wstring_view str)
			{
				auto size = uvec2(0, font_size);
				auto x = 0U;

				for (auto sh : str)
				{
					if (sh == '\n')
					{
						size.y += font_size;
						x = 0;
					}
					else if (sh != '\r')
					{
						if (sh == '\t')
							sh = ' ';
						x += get_glyph(sh, font_size).advance;
						size.x = max(size.x, x);
					}
				}
				return size;
			}

			inline std::wstring wrap_text(uint font_size, uint width, std::wstring_view str)
			{
				if (font_size > width)
				{
					assert(0);
					return L"";
				}

				auto ret = std::wstring();
				auto w = 0U;

				for (auto sh : str)
				{
					switch (sh)
					{
					case '\n':
						w = 0;
						ret += '\n';
						break;
					case '\r':
						continue;
					case '\t':
						sh = ' ';
					default:
						auto adv = get_glyph(sh, font_size).advance;
						if (w + adv >= width)
						{
							w = adv;
							ret += '\n';
						}
						else
							w += adv;
						ret += sh;
					}
				}

				return ret;
			}

			inline std::vector<GlyphDraw> get_draw_glyphs(uint size, std::wstring_view str, const vec2& pos = vec2(0.f), const mat2& axes = mat2(1.f))
			{
				std::vector<GlyphDraw> ret;
				ret.resize(str.size());

				auto i = 0;
				auto p = vec2(0.f);
				for (i = 0; i < str.size(); i++)
				{
					auto sh = str[i];
					if (sh == '\n')
					{
						p.y += size;
						p.x = 0.f;
					}
					else if (sh != '\r')
					{
						if (sh == '\t')
							sh = ' ';

						auto& g = get_glyph(sh, size);
						auto o = p + vec2(g.off);
						auto s = vec2(g.size);

						auto& dst = ret[i];
						dst.uvs = g.uv;
						dst.points[0] = pos + o * axes;
						dst.points[1] = pos + o.x * axes[0] + (o.y - s.y) * axes[1];
						dst.points[2] = pos + (o.x + s.x) * axes[0] + (o.y - s.y) * axes[1];
						dst.points[3] = pos + (o.x + s.x) * axes[0] + o.y * axes[1];

						p.x += g.advance;
					}
				}

				ret.resize(i);
				return ret;
			}

			static ivec2 icons_range();
			static wchar_t icon(uint hash);

			struct Get
			{
				virtual FontAtlasPtr operator()(const std::vector<std::filesystem::path>& font_names) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(FontAtlasPtr atlas) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};
	}
}
