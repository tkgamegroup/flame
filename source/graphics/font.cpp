#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>
#include "device_private.h"
#include "command_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "font_private.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace flame
{
	namespace graphics
	{
		FontPrivate::FontPrivate(const std::wstring& _filename)
		{
			filename = _filename;
			file = get_file_content(filename);

			stbtt_info = new stbtt_fontinfo;
			stbtt_InitFont(stbtt_info, (uchar*)file.data(), stbtt_GetFontOffsetForIndex((uchar*)file.data(), 0));
		}

		FontPrivate::~FontPrivate()
		{
			delete stbtt_info;
		}

		Font* Font::create(const wchar_t* filename)
		{
			if (!std::filesystem::exists(filename))
				return nullptr;
			return new FontPrivate(filename);
		}

		const auto font_atlas_size = uvec2(1024);

		FontAtlasPrivate::FontAtlasPrivate(DevicePrivate* device, std::span<FontPrivate*> _fonts) :
			device(device)
		{
			for (auto f : _fonts)
				fonts.push_back(f);

			bin_pack_root.reset(new BinPackNode(font_atlas_size));

			image.reset(new ImagePrivate(device, Format_R8_UNORM, font_atlas_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst));
			ImmediateCommandBuffer icb(device);
			auto cb = icb.cb.get();
			cb->image_barrier(image.get(), {}, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->clear_color_image(image.get(), cvec4(0, 0, 0, 255));
			cb->image_barrier(image.get(), {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			ImageSwizzle swz;
			swz.r = SwizzleOne;
			swz.g = SwizzleOne;
			swz.b = SwizzleOne;
			swz.a = SwizzleR;
			view = new ImageViewPrivate(image.get(), true, ImageView2D, {}, swz);
		}

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

				for (auto font : fonts)
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

							ImmediateStagingBuffer stag(device, image_pitch(g.size.x) * g.size.y, bitmap);
							ImmediateCommandBuffer icb(device);
							auto cb = icb.cb.get();
							cb->image_barrier(image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
							BufferImageCopy cpy;
							cpy.image_offset = atlas_pos;
							cpy.image_extent = g.size;
							cb->copy_buffer_to_image(stag.buf.get(), image.get(), { &cpy, 1 });
							cb->image_barrier(image.get(), {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

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

		FontAtlas* FontAtlas::create(Device* device, uint font_count, Font* const* fonts)
		{
			return new FontAtlasPrivate((DevicePrivate*)device, { (FontPrivate**)fonts, font_count });
		}
	}
}

