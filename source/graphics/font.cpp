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
			std::vector<FontPrivate*> fonts;

			std::unordered_map<ushort, std::unique_ptr<Glyph>> map;
			std::unique_ptr<BinPackNode> bin_pack_root;

			Image* image;
			Imageview* imageview;

			FontAtlasPrivate(Device* d, FontDrawType$ _draw_type, const std::vector<void*>& _fonts)
			{
				for (auto f : _fonts)
					fonts.push_back((FontPrivate*)f);

				draw_type = _draw_type;
				max_height = fonts[0]->max_height;

				max_width = 0;
				for (auto f : fonts)
				{
					assert(f->max_height == max_height);
					max_width = max(f->max_width, max_width);
				}

				bin_pack_root.reset(new BinPackNode(font_atlas_size));

				image = Image::create(d, draw_type == FontDrawPixel ? Format_R8_UNORM : Format_R8G8B8A8_UNORM, font_atlas_size, 1, 1, SampleCount_1, ImageUsage$(ImageUsageSampled | ImageUsageTransferDst));
				image->init(Vec4c(0, 0, 0, 255));
				if (draw_type == FontDrawPixel)
					imageview = Imageview::create(image, Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR);
				else
					imageview = Imageview::create(image);
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
					auto g = new Glyph;
					g->unicode = unicode;
					g->size = 0;
					g->off = 0;
					g->advance = 0;
					g->uv0 = Vec2f(0.f);
					g->uv1 = Vec2f(0.f);
					map[unicode].reset(g);

					for (auto font : fonts)
					{
						auto ft_face = font->ft_face;
						auto glyph_index = FT_Get_Char_Index(ft_face, unicode);
						if (glyph_index == 0)
							continue;
						FT_Load_Glyph(ft_face, glyph_index, draw_type == FontDrawLcd ? FT_LOAD_TARGET_LCD : FT_LOAD_DEFAULT);

						auto ft_glyph = ft_face->glyph;
						auto size = Vec2u(ft_glyph->bitmap.width, ft_glyph->bitmap.rows);
						if (draw_type == FontDrawLcd)
							size.x() /= 3;
						auto ascender = font->ascender;
						g->size = size;
						g->off = Vec2u(ft_glyph->bitmap_left, ascender + g->size.y() - ft_glyph->metrics.horiBearingY / 64.f);
						g->advance = ft_glyph->advance.x / 64;

						if (size > 0U)
						{
							auto n = bin_pack_root->find(size + (draw_type == FontDrawSdf ? sdf_range * 2 : 0));
							if (n)
							{
								auto& atlas_pos = n->pos;

								switch (draw_type)
								{
								case FontDrawPixel:
								{
									FT_Render_Glyph(ft_glyph, FT_RENDER_MODE_NORMAL);

									auto pitch_ft = ft_glyph->bitmap.pitch;
									auto pitch_temp = size.x();
									auto temp = new uchar[pitch_temp * size.y()];
									for (auto y = 0; y < size.y(); y++)
									{
										for (auto x = 0; x < size.x(); x++)
											temp[y * pitch_temp + x] = ft_glyph->bitmap.buffer[y * pitch_ft + x];
									}

									image->set_pixels(atlas_pos, size, temp);

									delete[] temp;
								}
									break;
								case FontDrawLcd:
								{
									FT_Render_Glyph(ft_glyph, FT_RENDER_MODE_LCD);

									auto pitch_ft = ft_glyph->bitmap.pitch;
									auto pitch_temp = size.x() * 4;
									auto temp = new uchar[pitch_temp * size.y()];
									for (auto y = 0; y < size.y(); y++)
									{
										for (auto x = 0; x < size.x(); x++)
										{
											temp[y * pitch_temp + x * 4 + 0] = ft_glyph->bitmap.buffer[y * pitch_ft + x * 3 + 0];
											temp[y * pitch_temp + x * 4 + 1] = ft_glyph->bitmap.buffer[y * pitch_ft + x * 3 + 1];
											temp[y * pitch_temp + x * 4 + 2] = ft_glyph->bitmap.buffer[y * pitch_ft + x * 3 + 2];
											temp[y * pitch_temp + x * 4 + 3] = 255;
										}
									}

									image->set_pixels(atlas_pos, size, temp);

									delete[] temp;
								}
									break;
								case FontDrawSdf:
								{
									void* ptr = ft_face;

									msdfgen::Shape shape;
									msdfgen::loadGlyph(shape, (msdfgen::FontHandle*) & ptr, unicode);

									size += sdf_range * 2;

									shape.normalize();
									msdfgen::edgeColoringSimple(shape, 3.f);
									msdfgen::Bitmap<float, 3> bmp(size.x(), size.y());
									msdfgen::generateMSDF(bmp, shape, sdf_range, 1.f, msdfgen::Vector2(-g->off.x(), g->off.y() - ascender) + sdf_range);

									auto pitch_temp = size.x() * 4;
									auto temp = new uchar[pitch_temp * size.y()];
									for (auto y = 0; y < size.y(); y++)
									{
										for (auto x = 0; x < size.x(); x++)
										{
											auto src = bmp(x, y);
											temp[y * pitch_temp + x * 4 + 0] = clamp(src[0] * 255.f, 0.f, 255.f);
											temp[y * pitch_temp + x * 4 + 1] = clamp(src[1] * 255.f, 0.f, 255.f);
											temp[y * pitch_temp + x * 4 + 2] = clamp(src[2] * 255.f, 0.f, 255.f);
											temp[y * pitch_temp + x * 4 + 3] = 255.f;
										}
									}

									image->set_pixels(atlas_pos, size, temp);

									delete[] temp;
								}
									break;
								}

								if (draw_type == FontDrawSdf)
								{
									g->uv0 = (Vec2f(atlas_pos.x(), atlas_pos.y()) + (float)sdf_range) / image->size;
									g->uv1 = (Vec2f(atlas_pos.x() + size.x(), atlas_pos.y() + size.y()) - (float)sdf_range) / image->size;
								}
								else
								{
									g->uv0 = Vec2f(atlas_pos.x(), atlas_pos.y() + size.y()) / image->size;
									g->uv1 = Vec2f(atlas_pos.x() + size.x(), atlas_pos.y()) / image->size;
								}
							}
						}

						break;
					}
				}

				return map[unicode].get();
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

