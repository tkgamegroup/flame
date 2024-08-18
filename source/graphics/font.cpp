#include "../json.h"
#include "../foundation/bitmap.h"
#include "device_private.h"
#include "buffer_private.h"
#include "command_private.h"
#include "shader_private.h"
#include "image_private.h"
#include "font_private.h"
#include "auxiliary.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#ifdef USE_MSDFGEN
#define MSDFGEN_PUBLIC __declspec(dllimport)
#include <msdfgen.h>
#endif

namespace flame
{
	namespace graphics
	{
		static std::vector<std::unique_ptr<Font>> loaded_fonts;
		static std::vector<std::unique_ptr<FontAtlasT>> loaded_atlases;

		Font::~Font()
		{
			delete stbtt_info;
		}

		FontAtlasPrivate::~FontAtlasPrivate()
		{
			for (auto ft : fonts)
			{
				if (ft->ref == 1)
				{
					std::erase_if(loaded_fonts, [&](const auto& i) {
						return i.get() == ft;
					});
				}
				else
					ft->ref--;
			}
		}

		const auto font_atlas_size = uvec2(1024);

		Glyph FontAtlasPrivate::pack_glyph(wchar_t code, uint font_size)
		{
			Glyph g;
			g.code = code;

			for (auto& font : loaded_fonts)
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
						if (auto n = bin_pack_root->find(ivec2(w + 1, h + 1)); n)
						{
							auto dst_pitch = image_pitch(w,
#if USE_D3D12
								256
#else
								4
#endif
							);
							StagingBuffer stag(dst_pitch * h, nullptr);
#if USE_D3D12
							for (auto l = 0; l < h; l++)
								memcpy((char*)stag->mapped + l * dst_pitch, (char*)bitmap_data + l * w, w);
#else
							memcpy(stag->mapped, bitmap_data, stag->size);
#endif

							InstanceCommandBuffer cb;
							auto old_layout = image->get_layout();
							cb->image_barrier(image.get(), {}, ImageLayoutTransferDst);
							BufferImageCopy cpy;
							cpy.img_off = uvec3(n->pos, 0);
							cpy.img_ext = uvec3(w, h, 1);
							cb->copy_buffer_to_image(stag.get(), image.get(), { &cpy, 1 });
							cb->image_barrier(image.get(), {}, old_layout);
							cb.excute();

							auto uv0 = vec2(n->pos.x, n->pos.y + h);
							auto uv1 = uv0 + vec2(w, -h);
							g.uv = vec4(uv0 / (vec2)font_atlas_size, uv1 / (vec2)font_atlas_size.y);
						}
						else
							printf("font atlas is full\n");
						delete[]bitmap_data;
					}

					g.size = uvec2(w, h);
					g.off = uvec2(x, ascent + h + y);
					g.advance = adv;
					break;
				case FontAtlasSDF:
				{
#ifdef USE_MSDFGEN
					const auto pxrange = 4;

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
						auto bbox = msdf_shape.getBounds();
						auto pad = pxrange >> 1;
						w = round(bbox.r - bbox.l) + pad + pad;
						h = round(bbox.t - bbox.b) + pad + pad;
						x = round(bbox.l) - pad;
						y = round(-bbox.b) + pad;
						auto xoff = round(-bbox.l) + pad;
						auto yoff = round(-bbox.b) + pad;

						msdfgen::Bitmap<float, 3> bitmap(w, h);
						msdfgen::edgeColoringSimple(msdf_shape, 3.0);
						msdfgen::generateMSDF(bitmap, msdf_shape, pxrange, 1.0, msdfgen::Vector2(xoff, yoff));

						if (auto n = bin_pack_root->find(ivec2(w + 1, h + 1)); n)
						{
							StagingBuffer stag(image_pitch(w * 4) * h);
							for (auto y = 0; y < h; y++)
							{
								auto dst = (uchar*)stag->mapped + image_pitch(w * 4) * y;
								for (auto x = 0; x < w; x++)
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
							cpy.img_off = uvec3(n->pos, 0);
							cpy.img_ext = uvec3(w, h, 1);
							cb->copy_buffer_to_image(stag.get(), image.get(), { &cpy, 1 });
							cb->image_barrier(image.get(), {}, old_layout);
							cb.excute();

							auto uv0 = vec2(n->pos.x, n->pos.y + h);
							auto uv1 = uv0 + vec2(w, -h);
							g.uv = vec4(uv0 / (vec2)font_atlas_size, uv1 / (vec2)font_atlas_size);
						}
						else
							printf("font atlas is full\n");
					}
#endif

					g.size = uvec2(w, h);
					g.off = uvec2(x, ascent + y);
					g.advance = adv;
				}
					break;
				}
			}

			return g;
		}

		const Glyph& FontAtlasPrivate::get_glyph(wchar_t code, uint font_size)
		{
			static Glyph empty_glyph;

			if (font_size == 0)
				return empty_glyph;

			if (type == FontAtlasSDF) 
				font_size = SDF_FONT_SIZE;

			if (font_size >= glyphs.size())
				glyphs.resize(font_size + 1);
			auto& array = glyphs[font_size];
			if (array.empty())
			{
				array.resize(0xFF + 1);
				for (auto ch = 0x0020; ch <= 0x00FF; ch++)
					array[ch] = pack_glyph(ch, font_size);
			}
			assert(code < array.size());

			return array[code];
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

				for (auto& a : loaded_atlases)
				{
					if (a->font_names == font_names && a->type == type)
					{
						a->ref++;
						return a.get();
					}
				}

				std::vector<Font*> fonts;

				for (auto& fn : font_names)
				{
					Font* font = nullptr;
					for (auto& ft : loaded_fonts)
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
							loaded_fonts.emplace_back(font);
						}
						else
						{
							delete font;
							wprintf(L"cannot load font: %s\n", fn.c_str());
						}
					}

					if (font)
						fonts.push_back(font);
				}

				auto ret = new FontAtlasPrivate;
				ret->type = type;
				ret->font_names = font_names;
				ret->fonts = fonts;

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

				ret->bin_pack_root->find(ivec2(1));

				ret->ref = 1;
				loaded_atlases.emplace_back(ret);
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
					std::erase_if(loaded_atlases, [&](const auto& i) {
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

