// MIT License
// 
// Copyright (c) 2019 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/foundation/foundation.h>
#include <flame/foundation/bitmap.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftlcdfil.h>
#include FT_OUTLINE_H

#include <msdfgen.h>

namespace flame
{
	namespace graphics
	{
		FT_Library ft_library = 0;

		void get_latin_code_range(wchar_t &out_begin, wchar_t &out_end)
		{
			out_begin = 0x20;
			out_end = 0xff;
		}

		struct GlyphPrivate : Glyph
		{
			ushort unicode;
			int grid_x, grid_y;
			GlyphPrivate* next;
		};

		const int atlas_width = 512;
		const int atlas_height = 512;
		const int sdf_range = 4;

		struct FontPrivate : Font
		{
			std::pair<std::unique_ptr<char[]>, long long> font_file;
			FT_Face ft_face;

			FontPrivate(const wchar_t *filename, int _pixel_height)
			{
				pixel_height = _pixel_height;

				if (!ft_library)
				{
					FT_Init_FreeType(&ft_library);
					FT_Library_SetLcdFilter(ft_library, FT_LCD_FILTER_DEFAULT);
				}

				font_file = get_file_content(filename);
				FT_New_Memory_Face(ft_library, (unsigned char*)font_file.first.get(), font_file.second, 0, &ft_face);
				FT_Size_RequestRec ft_req = {};
				ft_req.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
				ft_req.height = pixel_height * 64;
				FT_Request_Size(ft_face, &ft_req);
				max_width = ft_face->size->metrics.max_advance / 64;
				ascender = ft_face->size->metrics.ascender / 64;
			}

			~FontPrivate()
			{
				FT_Done_Face(ft_face);
			}
		};

		Font *Font::create(const wchar_t *filename, int pixel_height)
		{
			return new FontPrivate(filename, pixel_height);
		}

		void Font::destroy(Font *f)
		{
			delete (FontPrivate*)f;
		}

		struct FontAtlasPrivate : FontAtlas
		{
			Device* d;

			std::vector<Font*> fonts;

			Glyph* map[65536];
			GlyphPrivate* glyph_head;
			GlyphPrivate* glyph_tail;
			int grid_cx;
			int grid_cy;
			int grid_curr_x;
			int grid_curr_y;

			Image* atlas;

			FontAtlasPrivate(Device* _d, int _pixel_height, bool _sdf, const std::vector<Font*>& _fonts) :
				d(_d)
			{
				sdf = _sdf;
				pixel_height = _pixel_height;

				memset(map, 0, sizeof(map));

				atlas = Image::create(d, Format_R8G8B8A8_UNORM, Ivec2(atlas_width, atlas_height), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst, MemPropDevice);
				atlas->init(Vec4c(0));

				max_width = 0;
				for (auto src : _fonts)
				{
					if (src->pixel_height == pixel_height)
					{
						fonts.push_back(src);
						max_width = max(src->max_width, max_width);
					}
				}

				glyph_head = glyph_tail = nullptr;

				grid_cx = atlas_width / (max_width + (sdf ? sdf_range : 0));
				grid_cy = atlas_height / (pixel_height + (sdf ? sdf_range : 0));
				grid_curr_x = grid_curr_y = 0;
			}

			~FontAtlasPrivate()
			{
				Image::destroy(atlas);
			}

			const Glyph* get_glyph(wchar_t unicode)
			{
				if (!map[unicode])
				{
					GlyphPrivate* g;
					if (grid_curr_y == grid_cy)
					{
						g = glyph_head;
						glyph_head = g->next;

						map[g->unicode] = nullptr;

						glyph_tail->next = g;
						glyph_tail = g;
					}
					else
					{
						g = new GlyphPrivate;
						g->grid_x = grid_curr_x;
						g->grid_y = grid_curr_y;

						grid_curr_x++;
						if (grid_curr_x == grid_cx)
						{
							grid_curr_x = 0;
							grid_curr_y++;
						}

						if (glyph_tail)
						{
							glyph_tail->next = g;
							glyph_tail = g;
						}
						else
							glyph_head = glyph_tail = g;
					}

					g->unicode = unicode;
					g->next = nullptr;

					map[unicode] = g;

					for (auto _font : fonts)
					{
						auto font = (FontPrivate*)_font;
						auto ft_face = font->ft_face;
						auto ascender = font->ascender;
						auto glyph_index = FT_Get_Char_Index(ft_face, unicode);
						if (glyph_index == 0)
							continue;
						FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_TARGET_LCD);

						auto ft_glyph = ft_face->glyph;
						auto width = ft_glyph->bitmap.width / 3;
						auto height = ft_glyph->bitmap.rows;
						g->size = Vec2f(width, height);
						g->off = Vec2f(ft_glyph->bitmap_left, ascender + g->size.y() - ft_glyph->metrics.horiBearingY / 64.f);
						g->advance = ft_glyph->advance.x() / 64;

						if (!sdf)
						{
							FT_Render_Glyph(ft_glyph, FT_RENDER_MODE_LCD);

							auto x = g->grid_x * max_width;
							auto y = g->grid_y * pixel_height;

							if (width > 0 && height > 0)
							{
								auto pitch_ft = ft_glyph->bitmap.pitch;
								auto pitch_temp = width * 4;
								auto temp = new uchar[pitch_temp * height];
								for (auto y = 0; y < height; y++)
								{
									for (auto x = 0; x < width; x++)
									{
										temp[y * pitch_temp + x * 4 + 0] = ft_glyph->bitmap.buffer[y * pitch_ft + x * 3 + 0];
										temp[y * pitch_temp + x * 4 + 1] = ft_glyph->bitmap.buffer[y * pitch_ft + x * 3 + 1];
										temp[y * pitch_temp + x * 4 + 2] = ft_glyph->bitmap.buffer[y * pitch_ft + x * 3 + 2];
										temp[y * pitch_temp + x * 4 + 3] = 255;
									}
								}

								atlas->set_pixels(x, y, width, height, temp);

								delete[] temp;
							}

							g->uv0 = Vec2f(x, y + height) / atlas->size;
							g->uv1 = Vec2f(x + width, y) / atlas->size;
						}
						else
						{
							void* ptr = ft_face;

							msdfgen::Shape shape;
							msdfgen::loadGlyph(shape, (msdfgen::FontHandle*) & ptr, unicode);

							auto size = g->size;
							size += sdf_range * 2.f;

							shape.normalize();
							msdfgen::edgeColoringSimple(shape, 3.f);
							msdfgen::Bitmap<msdfgen::FloatRGB> bmp(size.x(), size.y());
							msdfgen::generateMSDF(bmp, shape, sdf_range, 1.f, msdfgen::Vector2(-g->off.x(), g->off.y() - ascender) + sdf_range);

							auto pitch = Bitmap::get_pitch(size.x() * 4);
							auto temp = new uchar[pitch * size.y()];
							for (auto y = 0; y < size.y(); y++)
							{
								for (auto x = 0; x < size.x(); x++)
								{
									auto& src = bmp(x, y);
									temp[y * pitch + x * 4 + 0] = clamp(src.r * 255.f, 0.f, 255.f);
									temp[y * pitch + x * 4 + 1] = clamp(src.g * 255.f, 0.f, 255.f);
									temp[y * pitch + x * 4 + 2] = clamp(src.b * 255.f, 0.f, 255.f);
									temp[y * pitch + x * 4 + 3] = 255.f;
								}
							}

							auto x = g->grid_x * (max_width + sdf_range);
							auto y = g->grid_y * (pixel_height + sdf_range);

							atlas->set_pixels(x, y, size.x(), size.y(), temp);

							delete temp;

							g->uv0 = Vec2f(x + sdf_range, y + sdf_range) / atlas->size;
							g->uv1 = Vec2f(x + size.x() - sdf_range, y + size.y() - sdf_range) / atlas->size;
						}

						break;
					}
				}

				return map[unicode];
			}

			int get_text_width(const wchar_t* text_beg, const wchar_t* text_end)
			{
				auto w = 0;
				auto s = text_beg;
				if (text_end == nullptr)
				{
					while (*s)
					{
						auto g = get_glyph(*s);
						w += g->advance;
						s++;
					}
				}
				else
				{
					while (s != text_end)
					{
						auto g = get_glyph(*s);
						w += g->advance;
						s++;
					}
				}
				return w;
			}
		};

		FontAtlas* FontAtlas::create(Device* d, int pixel_height, bool sdf, const std::vector<Font*>& fonts)
		{
			return new FontAtlasPrivate(d, pixel_height, sdf, fonts);
		}

		void FontAtlas::destroy(FontAtlas* f)
		{
			delete (FontAtlasPrivate*)f;
		}

		const Glyph* FontAtlas::get_glyph(wchar_t unicode)
		{
			return ((FontAtlasPrivate*)this)->get_glyph(unicode);
		}

		int FontAtlas::get_text_width(const wchar_t* text_beg, const wchar_t* text_end)
		{
			return ((FontAtlasPrivate*)this)->get_text_width(text_beg, text_end);
		}

		Image* FontAtlas::atlas() const
		{
			return ((FontAtlasPrivate*)this)->atlas;
		}
	}
}

