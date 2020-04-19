#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>
#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace flame
{
	namespace graphics
	{
		void get_latin_code_range(wchar_t& out_begin, wchar_t& out_end)
		{
			out_begin = 0x20;
			out_end = 0xff;
		}

		struct Font
		{
			std::filesystem::path filename;
			std::string font_file;
			stbtt_fontinfo info;
			uint ref_count;

			Font(const std::wstring& _filename)
			{
				filename = _filename;
				font_file = get_file_content(filename);

				stbtt_InitFont(&info, (uchar*)font_file.data(), stbtt_GetFontOffsetForIndex((uchar*)font_file.data(), 0));

				ref_count = 0;
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
				canvas_slot_ = -1;

				for (auto i = 0; i < font_count; i++)
				{
					auto fn = _fonts[i];
					if (!std::filesystem::exists(fn))
					{
						wprintf(L"cannot find font: %s\n", fn);
						continue;
					}

					id = hash_update(id, FLAME_HASH(fn));

					Font* f = nullptr;
					auto filename = std::filesystem::canonical(fn);
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
						report_used_file(filename.c_str());
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
				image->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, Vec4c(0, 0, 0, 255));
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
						auto info = &font->info;
						auto index = stbtt_FindGlyphIndex(info, unicode);
						if (index == 0)
							continue;

						auto scale = stbtt_ScaleForPixelHeight(info, font_size);
						auto x = 0, y = 0, w = 0, h = 0, ascent = 0, adv = 0;
						auto bitmap = stbtt_GetGlyphBitmap(info, scale, scale, index, &w, &h, &x, &y);
						stbtt_GetFontVMetrics(info, &ascent, 0, 0);
						ascent *= scale;
						stbtt_GetGlyphHMetrics(info, index, &adv, nullptr);
						adv *= scale;
						g->size = Vec2u(w, h);
						g->off = Vec2u(x, ascent + h + y);
						g->advance = adv;
						if (bitmap)
						{
							auto n = bin_pack_root->find(g->size);
							if (n)
							{
								auto& atlas_pos = n->pos;

								image->set_pixels(atlas_pos, g->size, bitmap);

								g->uv0 = Vec2f(atlas_pos.x(), atlas_pos.y() + g->size.y()) / image->size;
								g->uv1 = Vec2f(atlas_pos.x() + g->size.x(), atlas_pos.y()) / image->size;
							}
							else
								printf("font atlas is full\n");
							delete[]bitmap;
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
	}
}

