#pragma once

#include"graphics.h"

namespace flame
{
	namespace graphics
	{
		struct FontAtlas
		{
			FontAtlasType type;
			ImageViewPtr view;

			std::vector<std::filesystem::path> font_names;
			uint ref = 0;

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

			FLAME_GRAPHICS_API static ivec2 icons_range();
			FLAME_GRAPHICS_API static wchar_t icon(uint hash);
			static inline std::string icon_s(uint hash)
			{
				return w2s(std::wstring(1, icon(hash)));
			}

			struct Get
			{
				virtual FontAtlasPtr operator()(const std::vector<std::filesystem::path>& font_names, FontAtlasType type = FontAtlasBitmap) = 0;
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
