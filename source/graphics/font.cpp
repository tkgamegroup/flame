#include "../json.h"
#include "../foundation/bitmap.h"
#include "device_private.h"
#include "buffer_private.h"
#include "command_private.h"
#include "shader_private.h"
#include "image_private.h"
#include "font_private.h"
#include "extension.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#ifdef USE_MSDFGEN
#define MSDFGEN_PUBLIC
#include <msdfgen.h>
#endif

namespace flame
{
	namespace graphics
	{
		static std::vector<std::unique_ptr<Font>> fonts;
		static std::vector<std::unique_ptr<FontAtlasT>> atlases;
		const auto general_font_size = 14U;
		const auto sdf_font_size = 32U;

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

		static ivec2 _icons_range = ivec2(0);
		static std::unordered_map<uint, wchar_t> _icons;

		ivec2 FontAtlas::icons_range()
		{
#ifdef USE_FONT_AWESOME
			if (_icons.empty())
			{
				_icons_range[0] = 0xffff;
				_icons_range[1] = 0;

				auto icons_txt_path = std::filesystem::path(FONT_AWESOME_DIR) / L"metadata/icons.txt";
				if (std::filesystem::exists(icons_txt_path))
				{
					std::ifstream icons_file(icons_txt_path);
					while (!icons_file.eof())
					{
						std::string hash_s, code_s;
						std::getline(icons_file, hash_s);
						std::getline(icons_file, code_s);
						if (!hash_s.empty() && !code_s.empty())
						{
							auto code = s2t<uint>(code_s);
							_icons[s2t<uint>(hash_s)] = code;
							_icons_range[0] = min(_icons_range[0], (int)code);
							_icons_range[1] = max(_icons_range[1], (int)code);
						}
					}
					icons_file.close();
				}
				else
				{
					auto icons_json_path = std::filesystem::path(FONT_AWESOME_DIR) / L"metadata/icons.json";
					nlohmann::json icons_json;
					std::ifstream icons_json_file(icons_json_path);
					icons_json_file >> icons_json;
					icons_json_file.close();
					for (auto it = icons_json.begin(); it != icons_json.end(); it++)
					{
						auto name = it.key();
						auto code = s2u_hex<uint>(it.value()["unicode"].get<std::string>());
						if (code > 255)
						{
							_icons[sh(name.c_str())] = code;
							_icons_range[0] = min(_icons_range[0], (int)code);
							_icons_range[1] = max(_icons_range[1], (int)code);
						}
					}

					std::ofstream icons_file(icons_txt_path);
					for (auto it : _icons)
					{
						icons_file << it.first << std::endl;
						icons_file << (uint)it.second << std::endl;
					}
					icons_file.close();
				}
			}
#endif
			return _icons_range;
		}

		wchar_t FontAtlas::icon(uint hash)
		{
			auto it = _icons.find(hash);
			if (it != _icons.end())
				return it->second;
			return 0;
		}

		const auto font_atlas_size = uvec2(1024);

		const Glyph& FontAtlasPrivate::get_glyph(wchar_t code, uint font_size)
		{
			static Glyph empty_glyph;

			if (font_size == 0)
				return empty_glyph;

			if (type == FontAtlasSDF) font_size = sdf_font_size;
			auto key = GlyphKey(code, font_size);

			auto it = map.find(key);
			if (it == map.end())
			{
				Glyph g;
				g.code = code;
				if (code == 'S')
					int cut = 1;

				for (auto& font : fonts)
				{
					auto stbtt_info = font->stbtt_info;
					auto index = stbtt_FindGlyphIndex(stbtt_info, code);
					if (index == 0)
						continue;

					auto x = 0, y = 0, w = 0, h = 0;
					auto scale = stbtt_ScaleForPixelHeight(stbtt_info, font_size);
					auto ascent = 0, adv = 0;
					stbtt_GetFontVMetrics(stbtt_info, &ascent, 0, 0);
					ascent *= scale;
					stbtt_GetGlyphHMetrics(stbtt_info, index, &adv, nullptr);
					adv *= scale;
					switch (type)
					{
					case FontAtlasBitmap:
						if (auto bitmap_data = stbtt_GetGlyphBitmap(stbtt_info, scale, scale, index, &w, &h, &x, &y); bitmap_data)
						{
							if (auto n = bin_pack_root->find(uvec2(w, h)); n)
							{
								auto& atlas_pos = n->pos;

								StagingBuffer stag(image_pitch(w) * h, bitmap_data);

								InstanceCommandBuffer cb;
								auto old_layout = image->get_layout();
								cb->image_barrier(image.get(), {}, ImageLayoutTransferDst);
								BufferImageCopy cpy;
								cpy.img_off = uvec3(atlas_pos, 0);
								cpy.img_ext = uvec3(w, h, 1);
								cb->copy_buffer_to_image(stag.get(), image.get(), { &cpy, 1 });
								cb->image_barrier(image.get(), {}, old_layout);
								cb.excute();

								g.uv = vec4(atlas_pos.x / (float)font_atlas_size.x, (atlas_pos.y + h) / (float)font_atlas_size.y,
									(atlas_pos.x + w) / (float)font_atlas_size.x, atlas_pos.y / (float)font_atlas_size.y);
							}
							else
								printf("font atlas is full\n");
							delete[]bitmap_data;
						}
						break;
					case FontAtlasSDF:
					{
						int x0, y0, x1, y1;
						stbtt_GetGlyphBitmapBox(stbtt_info, index, scale, scale, &x0, &y0, &x1, &y1);
						x = x0; y = y0;
						w = x1 - x0; h = y1 - y0;

						stbtt_vertex* stbtt_verts = nullptr;
						auto n = stbtt_GetGlyphShape(stbtt_info, index, &stbtt_verts);
						msdfgen::Shape msdf_shape;
						msdf_shape.inverseYAxis = true;
						auto contour = &msdf_shape.addContour();
						auto position = msdfgen::Point2(0.f);
						for (auto i = 0; i < n; i++)
						{
							auto& v = stbtt_verts[i];
							switch (v.type)
							{
							case STBTT_vmove:
								if (!contour->edges.empty())
									contour = &msdf_shape.addContour();
								position = msdfgen::Point2(v.x, v.y) * scale;
								break;
							case STBTT_vline:
							{
								auto end_point = msdfgen::Point2(v.x, v.y) * scale;
								contour->addEdge(new msdfgen::LinearSegment(position, end_point));
								position = end_point;
							}
								break;
							case STBTT_vcurve:
							{
								auto control_point = msdfgen::Point2(v.cx, v.cy) * scale;
								auto end_point = msdfgen::Point2(v.x, v.y) * scale;
								contour->addEdge(new msdfgen::QuadraticSegment(position, control_point, end_point));
								position = end_point;
							}
								break;
							case STBTT_vcubic:
							{
								auto control_point1 = msdfgen::Point2(v.cx, v.cy) * scale;
								auto control_point2 = msdfgen::Point2(v.cx1, v.cy1) * scale;
								auto end_point = msdfgen::Point2(v.x, v.y) * scale;
								contour->addEdge(new msdfgen::CubicSegment(position, control_point1, control_point2, end_point));
								position = end_point;
							}
								break;
							}
						}
						stbtt_FreeShape(stbtt_info, stbtt_verts);

						if (contour->edges.empty())
							msdf_shape.contours.pop_back();
						if (!msdf_shape.contours.empty())
						{
							msdf_shape.normalize();
							msdfgen::Bitmap<float, 3> bitmap(sdf_font_size, sdf_font_size);
							msdfgen::edgeColoringSimple(msdf_shape, 3.0);
							msdfgen::generateMSDF(bitmap, msdf_shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));

							if (auto n = bin_pack_root->find(uvec2(bitmap.width(), bitmap.height())); n)
							{
								auto& atlas_pos = n->pos;

								StagingBuffer stag(image_pitch(bitmap.width() * 4) * bitmap.height());
								for (auto y = 0; y < bitmap.height(); y++)
								{
									auto dst = (uchar*)stag->mapped + image_pitch(bitmap.width() * 4) * y;
									for (auto x = 0; x < bitmap.width(); x++)
									{
										auto pixel = bitmap(x, y);
										dst[0] = uchar(clamp(pixel[0], 0.f, 1.f) * 255.f);
										dst[1] = uchar(clamp(pixel[1], 0.f, 1.f) * 255.f);
										dst[2] = uchar(clamp(pixel[2], 0.f, 1.f) * 255.f);
										dst[3] = 255;
										dst += 4;
									}
								}

								InstanceCommandBuffer cb;
								auto old_layout = image->get_layout();
								cb->image_barrier(image.get(), {}, ImageLayoutTransferDst);
								BufferImageCopy cpy;
								cpy.img_off = uvec3(atlas_pos, 0);
								cpy.img_ext = uvec3(bitmap.width(), bitmap.height(), 1);
								cb->copy_buffer_to_image(stag.get(), image.get(), { &cpy, 1 });
								cb->image_barrier(image.get(), {}, old_layout);
								cb.excute();

								auto uv0 = vec2(atlas_pos.x + x + 4, atlas_pos.y + ascent + h + y + 4);
								auto uv1 = uv0 + vec2(w, -h - 4);
								g.uv = vec4(uv0 / (vec2)font_atlas_size, uv1 / (vec2)font_atlas_size);
							}
							else
								printf("font atlas is full\n");
						}
					}
						break;
					}

					g.size = uvec2(w, h);
					g.off = uvec2(x, ascent + h + y);
					g.advance = adv;

					break;
				}

				it = map.insert(std::make_pair(key, g)).first;
				return it->second;
			}
			else
				return it->second;

			return empty_glyph;
		}

		float FontAtlasPrivate::get_scale()
		{
			if (type == FontAtlasBitmap)
				return 1.f;
			if (type == FontAtlasSDF)
				return 1.f / sdf_font_size;
			return 1.f;
		}

		struct FontAtlasGet : FontAtlas::Get
		{
			FontAtlasPtr operator()(const std::vector<std::filesystem::path>& _font_names, FontAtlasType type) override
			{
				std::vector<std::filesystem::path> font_names;
				for (auto& _fn : _font_names)
				{
					auto fn = Path::get(_fn);
					if (std::filesystem::exists(fn))
						font_names.push_back(fn);
					else
						wprintf(L"cannot find font: %s\n", _fn.c_str());
				}
				std::sort(font_names.begin(), font_names.end());

				for (auto& a : atlases)
				{
					if (a->font_names == font_names && a->type == type)
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
						font = new Font;
						font->filename = fn;
						font->content = get_file_content(fn);
						font->stbtt_info = new stbtt_fontinfo;
						if (stbtt_InitFont(font->stbtt_info, (uchar*)font->content.data(), stbtt_GetFontOffsetForIndex((uchar*)font->content.data(), 0)))
						{
							font->ref = 1;
							fonts.emplace_back(font);
						}
						else
						{
							delete font;
							wprintf(L"cannot load font: %s\n", fn.c_str());
						}
					}

					if (font)
						myfonts.push_back(font);
				}

				auto ret = new FontAtlasPrivate;
				ret->type = type;
				ret->font_names = font_names;
				ret->myfonts = myfonts;

				ret->bin_pack_root.reset(new BinPackNode(font_atlas_size));

				if (type == FontAtlasBitmap)
				{
					ret->image.reset(Image::create(Format_R8_UNORM, uvec3(font_atlas_size, 1), ImageUsageSampled | ImageUsageTransferSrc | ImageUsageTransferDst));
					ret->image->clear(vec4(0, 0, 0, 1), ImageLayoutShaderReadOnly);
					ret->view = ret->image->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR });
				}
				else if (type == FontAtlasSDF)
				{
					ret->image.reset(Image::create(Format_R8G8B8A8_UNORM, uvec3(font_atlas_size, 1), ImageUsageSampled | ImageUsageTransferSrc | ImageUsageTransferDst));
					ret->image->clear(vec4(0, 0, 0, 1), ImageLayoutShaderReadOnly);
					ret->view = ret->image->get_view();
				}
				else
					assert(0);

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

