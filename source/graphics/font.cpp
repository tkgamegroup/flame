#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>
#include <flame/graphics/device.h>
#include "image_private.h"
#include "font_private.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace flame
{
	namespace graphics
	{
		FontPrivate::FontPrivate(const std::wstring& filename)
		{
			_filename = filename;
			_file = get_file_content(filename);

			_stbtt_info = new stbtt_fontinfo;
			stbtt_InitFont(_stbtt_info, (uchar*)_file.data(), stbtt_GetFontOffsetForIndex((uchar*)_file.data(), 0));

			report_used_file(filename.c_str());
		}

		FontPrivate::~FontPrivate()
		{
			delete _stbtt_info;
		}

		Font* Font::create(const wchar_t* filename)
		{
			if (!std::filesystem::exists(filename))
				return nullptr;
			return new FontPrivate(filename);
		}

		const auto font_atlas_size = Vec2u(1024);

		FontAtlasPrivate::FontAtlasPrivate(DevicePrivate* d, std::span<FontPrivate*> fonts)
		{
			for (auto f : fonts)
				_fonts.push_back(f);

			_bin_pack_root.reset(new BinPackNode(font_atlas_size));

			_image.reset(new ImagePrivate(d, Format_R8_UNORM, font_atlas_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst, false));
			_image->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, Vec4c(0, 0, 0, 255));
			_view.reset(new ImageviewPrivate(_image.get(), Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR));
		}

		GlyphPrivate* FontAtlasPrivate::_get_glyph(wchar_t code, uint size)
		{
			if (size == 0)
				return nullptr;

			auto hash = hash_update(code, size);

			if (!_map[hash])
			{
				auto g = new GlyphPrivate;
				g->_code = code;
				g->_size = size;
				_map[hash].reset(g);

				for (auto font : _fonts)
				{
					auto info = font->_stbtt_info;
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
					g->_size = Vec2u(w, h);
					g->_off = Vec2u(x, ascent + h + y);
					g->_advance = adv;
					if (bitmap)
					{
						auto n = _bin_pack_root->find(g->_size);
						if (n)
						{
							auto& atlas_pos = n->pos;

							_image->set_pixels(atlas_pos, g->_size, bitmap);

							g->_uv = Vec4f(Vec2f(atlas_pos.x(), atlas_pos.y() + g->_size.y()) / _image->_size,
											Vec2f(atlas_pos.x() + g->_size.x(), atlas_pos.y()) / _image->_size);
						}
						else
							printf("font atlas is full\n");
						delete[]bitmap;
					}

					break;
				}
			}

			return _map[hash].get();
		}

		FontAtlas* FontAtlas::create(Device* d, uint font_count, Font* const* fonts)
		{
			return new FontAtlasPrivate((DevicePrivate*)d, { (FontPrivate**)fonts, font_count });
		}
	}
}

