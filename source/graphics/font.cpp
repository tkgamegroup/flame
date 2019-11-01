#include <flame/foundation/bitmap.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftlcdfil.h>
#include FT_OUTLINE_H

#include <msdfgen.h>
#include <msdfgen-ext.h>

namespace flame
{
	namespace graphics
	{
		FT_Library ft_library = 0;

		void get_latin_code_range(wchar_t& out_begin, wchar_t& out_end)
		{
			out_begin = 0x20;
			out_end = 0xff;
		}

		struct GlyphPrivate : Glyph
		{
			ushort unicode;
			Vec2u grid_pos;
			GlyphPrivate* next;
		};

		struct FontPrivate : Font
		{
			std::pair<std::unique_ptr<char[]>, long long> font_file;
			FT_Face ft_face;

			FontPrivate(const std::wstring& filename, uint _pixel_height)
			{
				max_height = _pixel_height;

				if (!ft_library)
				{
					FT_Init_FreeType(&ft_library);
					FT_Library_SetLcdFilter(ft_library, FT_LCD_FILTER_DEFAULT);
				}

				font_file = get_file_content(filename);
				FT_New_Memory_Face(ft_library, (uchar*)font_file.first.get(), font_file.second, 0, &ft_face);

				FT_Size_RequestRec ft_req = {};
				ft_req.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
				ft_req.height = max_height * 64;
				FT_Request_Size(ft_face, &ft_req);
				max_width = ft_face->size->metrics.max_advance / 64;
				ascender = ft_face->size->metrics.ascender / 64;
			}

			~FontPrivate()
			{
				FT_Done_Face(ft_face);
			}
		};

		Font* Font::create(const std::wstring& filename, uint pixel_height)
		{
			return new FontPrivate(filename, pixel_height);
		}

		void Font::destroy(Font* f)
		{
			delete (FontPrivate*)f;
		}

		struct Font$
		{
			AttributeV<std::wstring> filename$i;
			AttributeV<uint> pixel_height$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Font$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (filename$i.frame > out$o.frame || pixel_height$i.frame > out$o.frame)
				{
					if (out$o.v)
						Font::destroy((Font*)out$o.v);
					if (std::filesystem::exists(filename$i.v))
						out$o.v = Font::create(filename$i.v, pixel_height$i.v);
					else
						printf("cannot create font\n");
					out$o.frame = max(filename$i.frame, pixel_height$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Font$()
			{
				if (out$o.v)
					Font::destroy((Font*)out$o.v);
			}
		};

		struct FontAtlasPrivate : FontAtlas
		{
			Device* d;

			std::vector<FontPrivate*> fonts;

			GlyphPrivate* map[65536];
			GlyphPrivate* glyph_head;
			GlyphPrivate* glyph_tail;
			Vec2u grid_size;
			Vec2u grid_count;
			Vec2u grid_curr_pos;

			Image* image;
			Imageview* imageview;

			FontAtlasPrivate(Device* d, FontDrawType$ _draw_type, const std::vector<void*>& _fonts) :
				d(d)
			{
				for (auto f : _fonts)
					fonts.push_back((FontPrivate*)f);

				draw_type = _draw_type;
				max_height = fonts[0]->max_height;

				memset(map, 0, sizeof(map));

				image = Image::create(d, draw_type == FontDrawPixel ? Format_R8_UNORM : Format_R8G8B8A8_UNORM, font_atlas_size, 1, 1, SampleCount_1, ImageUsage$(ImageUsageSampled | ImageUsageTransferDst));
				image->init(Vec4c(0, 0, 0, 255));
				if (draw_type == FontDrawPixel)
					imageview = Imageview::create(image, Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR);
				else
					imageview = Imageview::create(image);

				max_width = 0;
				for (auto f : fonts)
				{
					assert(f->max_height == max_height);
					max_width = max(f->max_width, max_width);
				}

				glyph_head = glyph_tail = nullptr;

				if (draw_type != FontDrawSdf)
				{
					grid_size.x() = max_width;
					grid_size.y() = max_height;
				}
				else
				{
					grid_size.x() = sdf_grid_size + sdf_range * 2;
					grid_size.y() = sdf_grid_size + sdf_range * 2;
				}
				grid_count.x() = font_atlas_size.x() / grid_size.x();
				grid_count.y() = font_atlas_size.y() / grid_size.y();
				grid_curr_pos = 0;
			}

			~FontAtlasPrivate()
			{
				Imageview::destroy(imageview);
				Image::destroy(image);
			}

			const Glyph* get_glyph(wchar_t unicode)
			{
				if (!map[unicode])
				{
					GlyphPrivate* g;
					if (grid_curr_pos.y() == grid_size.y())
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
						g->grid_pos = grid_curr_pos;

						grid_curr_pos.x()++;
						if (grid_curr_pos.x() == grid_count.x())
						{
							grid_curr_pos.x() = 0;
							grid_curr_pos.y()++;
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

					for (auto font : fonts)
					{
						auto ft_face = font->ft_face;
						auto ascender = font->ascender;
						auto glyph_index = FT_Get_Char_Index(ft_face, unicode);
						if (glyph_index == 0)
							continue;
						FT_Load_Glyph(ft_face, glyph_index, draw_type == FontDrawLcd ? FT_LOAD_TARGET_LCD : FT_LOAD_DEFAULT);

						auto ft_glyph = ft_face->glyph;
						auto width = ft_glyph->bitmap.width;
						if (draw_type == FontDrawLcd)
							width /= 3;
						auto height = ft_glyph->bitmap.rows;
						g->size = Vec2u(width, height);
						g->off = Vec2u(ft_glyph->bitmap_left, ascender + g->size.y() - ft_glyph->metrics.horiBearingY / 64.f);
						g->advance = ft_glyph->advance.x / 64;

						switch (draw_type)
						{
						case FontDrawPixel:
						{
							FT_Render_Glyph(ft_glyph, FT_RENDER_MODE_NORMAL);

							auto x = g->grid_pos.x() * grid_size.x();
							auto y = g->grid_pos.y() * grid_size.y();

							if (width > 0 && height > 0)
							{
								auto pitch_ft = ft_glyph->bitmap.pitch;
								auto pitch_temp = width;
								auto temp = new uchar[pitch_temp * height];
								for (auto y = 0; y < height; y++)
								{
									for (auto x = 0; x < width; x++)
										temp[y * pitch_temp + x] = ft_glyph->bitmap.buffer[y * pitch_ft + x];
								}

								image->set_pixels(x, y, width, height, temp);

								delete[] temp;
							}

							g->uv0 = Vec2f(x, y + height) / image->size;
							g->uv1 = Vec2f(x + width, y) / image->size;
						}
							break;
						case FontDrawLcd:
						{
							FT_Render_Glyph(ft_glyph, FT_RENDER_MODE_LCD);

							auto x = g->grid_pos.x() * grid_size.x();
							auto y = g->grid_pos.y() * grid_size.y();

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

								image->set_pixels(x, y, width, height, temp);

								delete[] temp;
							}

							g->uv0 = Vec2f(x, y + height) / image->size;
							g->uv1 = Vec2f(x + width, y) / image->size;
						}
							break;
						case FontDrawSdf:
						{
							void* ptr = ft_face;

							msdfgen::Shape shape;
							msdfgen::loadGlyph(shape, (msdfgen::FontHandle*) & ptr, unicode);

							auto size = g->size;
							size += sdf_range * 2.f;

							shape.normalize();
							msdfgen::edgeColoringSimple(shape, 3.f);
							msdfgen::Bitmap<float, 3> bmp(size.x(), size.y());
							msdfgen::generateMSDF(bmp, shape, sdf_range, 1.f, msdfgen::Vector2(-g->off.x(), g->off.y() - ascender) + sdf_range);

							auto temp_pitch = size.x() * 4;
							auto temp = new uchar[temp_pitch * size.y()];
							for (auto y = 0; y < size.y(); y++)
							{
								for (auto x = 0; x < size.x(); x++)
								{
									auto src = bmp(x, y);
									temp[y * temp_pitch + x * 4 + 0] = clamp(src[0] * 255.f, 0.f, 255.f);
									temp[y * temp_pitch + x * 4 + 1] = clamp(src[1] * 255.f, 0.f, 255.f);
									temp[y * temp_pitch + x * 4 + 2] = clamp(src[2] * 255.f, 0.f, 255.f);
									temp[y * temp_pitch + x * 4 + 3] = 255.f;
								}
							}

							auto x = g->grid_pos.x() * grid_size.x();
							auto y = g->grid_pos.y() * grid_size.y();

							image->set_pixels(x, y, size.x(), size.y(), temp);

							delete temp;

							g->uv0 = Vec2f(x + sdf_range, y + sdf_range) / image->size;
							g->uv1 = Vec2f(x + size.x() - sdf_range, y + size.y() - sdf_range) / image->size;
						}
							break;
						}

						break;
					}
				}

				return map[unicode];
			}

			Vec2f get_text_offset(const std::wstring_view& text, uint font_size)
			{
				auto w = 0.f;
				auto h = 0.f;
				for (auto ch : text)
				{
					if (!ch)
						break;
					if (ch == '\n')
					{
						w = 0.f;
						h += font_size;
					}
					else if (ch != '\r' && ch != '\t')
						w += get_advance(ch, font_size);
				}
				return Vec2f(w, h);
			}

			Vec2f get_text_size(const std::wstring_view& text, uint font_size)
			{
				auto w = 0.f;
				auto h = (float)font_size;
				auto lw = 0.f;
				for (auto ch : text)
				{
					if (ch == '\n')
					{
						h += font_size;
						lw = 0.f;
					}
					else if (ch != '\r')
					{
						if (ch == '\t')
							ch = ' ';
						lw += get_advance(ch, font_size);
						if (lw > w)
							w = lw;
					}
				}
				return Vec2f(w, h);
			}

			Mail<std::wstring> slice_text_by_width(const std::wstring_view& text, uint width)
			{
				assert(width > max_width);

				auto ret = new_mail<std::wstring>();
				auto w = 0;
				for (auto ch : text)
				{
					if (!ch)
						break;
					switch (ch)
					{
					case '\n':
						w = 0;
						*ret.p += '\n';
						break;
					case '\r':
						break;
					case '\t':
						ch = ' ';
					default:
						auto adv = get_glyph(ch)->advance;
						if (w + adv >= width)
						{
							w = adv;
							*ret.p += '\n';
						}
						else
							w += adv;
						*ret.p += ch;
					}
				}
				return ret;
			}
		};

		FontAtlas* FontAtlas::create(Device* d, FontDrawType$ draw_type, const std::vector<void*>& fonts)
		{
			return new FontAtlasPrivate(d, draw_type, fonts);
		}

		void FontAtlas::destroy(FontAtlas* f)
		{
			delete (FontAtlasPrivate*)f;
		}

		const Glyph* FontAtlas::get_glyph(wchar_t unicode)
		{
			return ((FontAtlasPrivate*)this)->get_glyph(unicode);
		}

		float FontAtlas::get_advance(wchar_t unicode, uint font_size)
		{
			float advance = ((FontAtlasPrivate*)this)->get_glyph(unicode)->advance;
			if (draw_type == FontDrawSdf)
				advance *= (float)font_size / sdf_grid_size;
			return advance;
		}

		Vec2f FontAtlas::get_text_offset(const std::wstring_view& text, uint font_size)
		{
			return ((FontAtlasPrivate*)this)->get_text_offset(text, font_size);
		}

		Vec2f FontAtlas::get_text_size(const std::wstring_view& text, uint font_size)
		{
			return ((FontAtlasPrivate*)this)->get_text_size(text, font_size);
		}

		Mail<std::wstring> FontAtlas::slice_text_by_width(const std::wstring_view& text, uint width)
		{
			return ((FontAtlasPrivate*)this)->slice_text_by_width(text, width);
		}

		Image* FontAtlas::image() const
		{
			return ((FontAtlasPrivate*)this)->image;
		}

		Imageview* FontAtlas::imageview() const
		{
			return ((FontAtlasPrivate*)this)->imageview;
		}

		struct FontAtlas$
		{
			AttributeE<FontDrawType$> draw_type$i;
			AttributeP<std::vector<void*>> fonts$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (draw_type$i.frame > out$o.frame || fonts$i.frame > out$o.frame)
				{
					if (out$o.v)
						FontAtlas::destroy((FontAtlas*)out$o.v);
					auto d = (Device*)bp_env().graphics_device;
					auto fonts = get_attribute_vec(fonts$i);
					if (d && !fonts.empty())
						out$o.v = FontAtlas::create(d, draw_type$i.v, fonts);
					else
						printf("cannot create fontatlas\n");
					out$o.frame = max(draw_type$i.frame, fonts$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~FontAtlas$()
			{
				if (out$o.v)
					FontAtlas::destroy((FontAtlas*)out$o.v);
			}
		};
	}
}

