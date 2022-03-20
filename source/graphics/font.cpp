#include "../foundation/bitmap.h"
#include "device_private.h"
#include "buffer_private.h"
#include "command_private.h"
#include "image_private.h"
#include "font_private.h"
#include "extension.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace flame
{
	namespace graphics
	{
		static std::vector<std::unique_ptr<Font>> fonts;
		static std::vector<std::unique_ptr<FontAtlasT>> atlases;

		Font::~Font()
		{
			delete stbtt_info;
		}

		FontAtlasPrivate::~FontAtlasPrivate()
		{
			for (auto ft : myfonts)
			{
				if (ft->ref == 1)
				{
					std::erase_if(fonts, [&](const auto& i) {
						return i.get() == ft;
					});
				}
				else
					ft->ref--;
			}
		}

		const auto font_atlas_size = uvec2(1024);

		const Glyph& FontAtlasPrivate::get_glyph(wchar_t code, uint size)
		{
			static Glyph empty_glyph;

			if (size == 0)
				return empty_glyph;

			auto key = GlyphKey(code, size);

			auto it = map.find(key);
			if (it == map.end())
			{
				Glyph g;
				g.code = code;

				for (auto& font : fonts)
				{
					auto info = font->stbtt_info;
					auto index = stbtt_FindGlyphIndex(info, code);
					if (index == 0)
						continue;

					auto scale = stbtt_ScaleForPixelHeight(info, size);
					auto x = 0, y = 0, w = 0, h = 0, ascent = 0, adv = 0;
					auto bitmap = stbtt_GetGlyphBitmap(info, scale, scale, index, &w, &h, &x, &y);
					stbtt_GetFontVMetrics(info, &ascent, 0, 0);
					ascent *= scale;
					stbtt_GetGlyphHMetrics(info, index, &adv, nullptr);
					adv *= scale;
					g.size = uvec2(w, h);
					g.off = uvec2(x, ascent + h + y);
					g.advance = adv;
					if (bitmap)
					{
						auto n = bin_pack_root->find(g.size);
						if (n)
						{
							auto& atlas_pos = n->pos;

							StagingBuffer stag(image_pitch(g.size.x) * g.size.y, bitmap);
							InstanceCB cb;

							auto old_layout = image->levels[0].layers[0].layout;
							cb->image_barrier(image.get(), {}, ImageLayoutTransferDst);
							BufferImageCopy cpy;
							cpy.img_off = atlas_pos;
							cpy.img_ext = g.size;
							cb->copy_buffer_to_image(stag.get(), image.get(), { &cpy, 1 });
							cb->image_barrier(image.get(), {}, old_layout);

							g.uv = vec4(atlas_pos.x / (float)font_atlas_size.x, (atlas_pos.y + g.size.y) / (float)font_atlas_size.y,
								(atlas_pos.x + g.size.x) / (float)font_atlas_size.x, atlas_pos.y / (float)font_atlas_size.y);
						}
						else
							printf("font atlas is full\n");
						delete[]bitmap;
					}

					break;
				}

				it = map.insert(std::make_pair(key, g)).first;
				return it->second;
			}
			else
				return it->second;

			return empty_glyph;
		}

		struct FontAtlasGet : FontAtlas::Get
		{
			FontAtlasPtr operator()(const std::vector<std::filesystem::path>& _font_names) override
			{
				std::vector<std::filesystem::path> font_names;
				for (auto& _fn : _font_names)
				{
					auto fn = Path::get(_fn);
					if (std::filesystem::exists(_fn))
						font_names.push_back(fn);
					else
						wprintf(L"cannot find font: %s\n", fn.c_str());
				}
				std::sort(font_names.begin(), font_names.end());

				for (auto& a : atlases)
				{
					if (a->font_names == font_names)
					{
						a->ref++;
						return a.get();
					}
				}

				std::vector<Font*> myfonts;

				for (auto& fn : font_names)
				{
					Font* font = nullptr;
					for (auto& ft : fonts)
					{
						if (ft->filename == fn)
						{
							font = ft.get();
							ft->ref++;
							break;
						}
					}

					if (!font)
					{
						auto stbtt_info = new stbtt_fontinfo;
						auto content = get_file_content(fn);
						if (stbtt_InitFont(stbtt_info, (uchar*)content.data(), stbtt_GetFontOffsetForIndex((uchar*)content.data(), 0)))
						{
							font = new Font;
							font->filename = fn;
							font->content = content;

							font->ref = 1;
							fonts.emplace_back(font);
						}
						else
							wprintf(L"cannot load font: %s\n", fn.c_str());
					}

					if (font)
						myfonts.push_back(font);
				}

				auto ret = new FontAtlasPrivate;
				ret->font_names = font_names;
				ret->myfonts = myfonts;

				ret->bin_pack_root.reset(new BinPackNode(font_atlas_size));

				ret->image.reset(Image::create(Format_R8_UNORM, font_atlas_size, ImageUsageSampled | ImageUsageTransferDst));
				ret->image->clear(vec4(0, 0, 0, 1), ImageLayoutShaderReadOnly);
				ret->view = ret->image->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR });

				ret->ref = 1;
				atlases.emplace_back(ret);
				return ret;
			}
		}FontAtlas_get;
		FontAtlas::Get& FontAtlas::get = FontAtlas_get;

		struct FontAtlasRelease : FontAtlas::Release
		{
			void operator()(FontAtlasPtr atlas) override
			{
				if (atlas->ref == 1)
				{
					std::erase_if(atlases, [&](const auto& i) {
						return i.get() == atlas;
					});
				}
				else
					atlas->ref--;
			}
		}FontAtlas_release;
		FontAtlas::Release& FontAtlas::release = FontAtlas_release;
	}
}

