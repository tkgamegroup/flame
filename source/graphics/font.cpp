#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/foundation/bitmap.h>
#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftlcdfil.h>
#include FT_OUTLINE_H

#include <flame/reflect_macros.h>

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

		struct Font
		{
			std::wstring filename;
			std::string font_file;
			FT_Face ft_face;
			uint ref_count;

			Font(const std::wstring& _filename)
			{
				filename = _filename;
				font_file = get_file_content(filename);

				if (!ft_library)
				{
					FT_Init_FreeType(&ft_library);
					FT_Library_SetLcdFilter(ft_library, FT_LCD_FILTER_DEFAULT);
				}

				FT_New_Memory_Face(ft_library, (uchar*)font_file.data(), font_file.size(), 0, &ft_face);

				ref_count = 0;
			}

			~Font()
			{
				FT_Done_Face(ft_face);
			}
		};

		static std::vector<std::unique_ptr<Font>> loaded_fonts;

		Glyph* new_glyph()
		{
			auto g = new Glyph;
			g->unicode = 0;
			g->size = 0;
			g->off = 0;
			g->advance = 0;
			g->uv0 = Vec2f(0.f);
			g->uv1 = Vec2f(0.f);
			return g;
		}

		struct FontAtlasPrivate : FontAtlas
		{
			std::vector<Font*> fonts;

			std::unique_ptr<Glyph> empty_glyph;
			std::unordered_map<uint, std::unique_ptr<Glyph>> map;
			std::unique_ptr<BinPackNode> bin_pack_root;

			Image* image;
			Imageview* imageview;

			FontAtlasPrivate(Device* d, uint font_count, const wchar_t* const* _fonts)
			{
				for (auto i = 0; i < font_count; i++)
				{
					auto fn = _fonts[i];
					if (!std::filesystem::exists(fn))
						continue;

					id = hash_update(id, FLAME_HASH(fn));

					Font* f = nullptr;
					auto filename = std::filesystem::canonical(fn).wstring();
					for (auto& _f : loaded_fonts)
					{
						if (_f->filename == filename)
						{
							f = _f.get();
							break;
						}
					}
					if (!f)
					{
						f = new Font(filename);
						loaded_fonts.emplace_back(f);
					}
					if (f)
					{
						f->ref_count++;
						fonts.push_back(f);
					}
				}

				bin_pack_root.reset(new BinPackNode(font_atlas_size));

				image = Image::create(d, Format_R8_UNORM, font_atlas_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst);
				image->init(Vec4c(0, 0, 0, 255));
				imageview = Imageview::create(image, Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR);

				empty_glyph.reset(new_glyph());
			}

			~FontAtlasPrivate()
			{
				for (auto f : fonts)
				{
					f->ref_count--;
					if (f->ref_count == 0)
					{
						for (auto it = loaded_fonts.begin(); it != loaded_fonts.end(); it++)
						{
							if (it->get() == f)
							{
								loaded_fonts.erase(it);
								break;
							}
						}
					}
				}

				Imageview::destroy(imageview);
				Image::destroy(image);
			}

			Glyph* get_glyph(wchar_t unicode, uint font_size)
			{
				if (font_size == 0)
					return empty_glyph.get();

				auto hash = hash_update(unicode, font_size);

				if (!map[hash])
				{
					auto g = new_glyph();
					g->unicode = unicode;
					g->font_size = font_size;
					map[hash].reset(g);

					for (auto font : fonts)
					{
						auto ft_face = font->ft_face;
						auto glyph_index = FT_Get_Char_Index(ft_face, unicode);
						if (glyph_index == 0)
							continue;

						FT_Size_RequestRec ft_req = {};
						ft_req.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
						ft_req.height = font_size * 64;
						FT_Request_Size(ft_face, &ft_req);
						auto ascender = ft_face->size->metrics.ascender / 64;

						FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT);

						auto ft_glyph = ft_face->glyph;
						auto size = Vec2u(ft_glyph->bitmap.width, ft_glyph->bitmap.rows);
						g->size = size;
						g->off = Vec2u(ft_glyph->bitmap_left, ascender + g->size.y() - ft_glyph->metrics.horiBearingY / 64.f);
						g->advance = ft_glyph->advance.x / 64;

						if (size > 0U)
						{
							auto n = bin_pack_root->find(size);
							if (n)
							{
								auto& atlas_pos = n->pos;

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

								g->uv0 = Vec2f(atlas_pos.x(), atlas_pos.y() + size.y()) / image->size;
								g->uv1 = Vec2f(atlas_pos.x() + size.x(), atlas_pos.y()) / image->size;
							}
							else
								printf("font atlas is full\n");
						}

						break;
					}
				}

				return map[hash].get();
			}
		};

		FontAtlas* FontAtlas::create(Device* d, uint font_count, const wchar_t* const* fonts)
		{
			return new FontAtlasPrivate(d, font_count, fonts);
		}

		void FontAtlas::destroy(FontAtlas* f)
		{
			delete (FontAtlasPrivate*)f;
		}

		Glyph* FontAtlas::get_glyph(wchar_t unicode, uint font_size)
		{
			return ((FontAtlasPrivate*)this)->get_glyph(unicode, font_size);
		}

		Imageview* FontAtlas::imageview() const
		{
			return ((FontAtlasPrivate*)this)->imageview;
		}

		struct R(R_FontAtlas)
		{
			BP::Node* n;

			BASE0;
			RV(Array<StringW>*, fonts, i);

			BASE1;
			RV(FontAtlas*, out, o);

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				auto out_frame = out_s()->frame();
				if (fonts_s()->frame() > out_frame)
				{
					if (out)
						FontAtlas::destroy(out);
					auto d = Device::default_one();
					std::vector<const wchar_t*> _fonts(fonts ? fonts->s : 0);
					for (auto i = 0; i < _fonts.size(); i++)
						_fonts[i] = fonts->at(i).v;
					if (d && !_fonts.empty())
						out = FontAtlas::create(d, _fonts.size(), _fonts.data());
					else
						printf("cannot create fontatlas\n");
					out_s()->set_frame(frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS RF(~R_FontAtlas)()
			{
				if (out)
					FontAtlas::destroy(out);
			}
		};
	}
}

