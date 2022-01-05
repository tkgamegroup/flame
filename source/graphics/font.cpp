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
		Font::Font()
		{
			stbtt_info = new stbtt_fontinfo;
		}

		Font::~Font()
		{
			delete stbtt_info;
		}

		const auto font_atlas_size = uvec2(1024);

		const Glyph& FontAtlasPrivate::get_glyph(wchar_t code, uint size)
		{
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

							StagingBuffer stag(device, image_pitch(g.size.x) * g.size.y, bitmap);
							InstanceCB cb(device);

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

		static std::vector<std::pair<std::vector<std::wstring>, std::unique_ptr<FontAtlasT>>> loaded_atlas;

		struct FontAtlasGet : FontAtlas::Get
		{
			FontAtlasPtr operator()(DevicePtr device, const std::wstring& font_names) override
			{
				if (!device)
					device = current_device;

				auto sp = SUW::split(font_names, L';');
				std::sort(sp.begin(), sp.end());
				for (auto& a : loaded_atlas)
				{
					if (a.first == sp)
						return a.second.get();
				}

				std::vector<std::unique_ptr<Font>> fonts;
				for (auto& s : sp)
				{
					auto fn = Path::get(s);
					if (!std::filesystem::exists(fn))
					{
						wprintf(L"cannot find font: %s\n", s.c_str());
						return nullptr;
					}

					auto font = new Font;
					font->file = get_file_content(fn);
					if (font->file.empty() || !stbtt_InitFont(font->stbtt_info, (uchar*)font->file.data(), stbtt_GetFontOffsetForIndex((uchar*)font->file.data(), 0)))
					{
						wprintf(L"cannot load font: %s\n", s.c_str());
						return nullptr;
					}

					fonts.emplace_back(font);
				}

				auto ret = new FontAtlasPrivate;
				ret->device = device;
				ret->fonts = std::move(fonts);

				ret->bin_pack_root.reset(new BinPackNode(font_atlas_size));

				ret->image.reset(Image::create(device, Format_R8_UNORM, font_atlas_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst));
				ret->image->clear(vec4(0, 0, 0, 1), ImageLayoutShaderReadOnly);
				ret->view = ret->image->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR });

				loaded_atlas.emplace_back(sp, ret);
				return ret;
			}
		}FontAtlas_get;
		FontAtlas::Get& FontAtlas::get = FontAtlas_get;
	}
}

