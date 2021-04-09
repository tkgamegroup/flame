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
		Font::~Font()
		{
			delete stbtt_info;
		}

		const auto font_atlas_size = uvec2(1024);

		FontAtlasPrivate::FontAtlasPrivate(DevicePrivate* device, const std::vector<Font*>& _fonts) :
			device(device)
		{
			for (auto& f : _fonts)
				fonts.emplace_back(f);

			bin_pack_root.reset(new BinPackNode(font_atlas_size));

			image.reset(new ImagePrivate(device, Format_R8_UNORM, font_atlas_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst));
			image->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, cvec4(0, 0, 0, 255));

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

							cb->image_barrier(image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
							BufferImageCopy cpy;
							cpy.image_offset = atlas_pos;
							cpy.image_extent = g.size;
							cb->copy_buffer_to_image((BufferPrivate*)stag.get(), image.get(), 1, &cpy);
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

		static std::vector<std::pair<std::vector<std::wstring>, UniPtr<FontAtlasPrivate>>> loaded_atlas;

		FontAtlasPtr FontAtlasPrivate::get(DevicePtr device, const std::wstring& res)
		{
			auto sp = SUW::split(res, L';');
			std::sort(sp.begin(), sp.end());
			for (auto& a : loaded_atlas)
			{
				if (a.first == sp)
					return a.second.get();
			}

			std::vector<Font*> fonts;
			for (auto& s : sp)
			{
				auto fn = std::filesystem::path(s);
				if (!std::filesystem::exists(fn))
				{
					if (!get_engine_path(fn, L"assets"))
						fn = L"c:\\windows\\fonts" / std::filesystem::path(s);
				}

				auto font = new Font;
				font->file = get_file_content(fn);
				if (font->file.empty() ||
					!stbtt_InitFont(font->stbtt_info, (uchar*)font->file.data(), stbtt_GetFontOffsetForIndex((uchar*)font->file.data(), 0)))
				{
					wprintf(L"cannot load font: %s\n", s.c_str());
					return nullptr;
				}

				fonts.push_back(font);
			}

			auto ret = new FontAtlasPrivate(device, fonts);
			loaded_atlas.emplace_back(sp, ret);
			return ret;
		}

		FontAtlas* FontAtlas::get(Device* device, const wchar_t* res)
		{
			return FontAtlasPrivate::get((DevicePrivate*)device, res);
		}
	}
}

