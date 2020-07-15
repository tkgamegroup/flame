#include <flame/serialize.h>
#include "device_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "font_private.h"
#include "renderpass_private.h"
#include "shader_private.h"
#include "command_private.h"
#include "swapchain_private.h"
#include "canvas_private.h"

namespace flame
{
	namespace graphics
	{
		static RenderpassPrivate* rp = nullptr;
		static DescriptorSetLayoutPrivate* dsl = nullptr;
		static PipelineLayoutPrivate* pll = nullptr;
		static ShaderPrivate* vert = nullptr;
		static ShaderPrivate* frag = nullptr;
		static PipelinePrivate* pl = nullptr;

		CanvasPrivate::CanvasPrivate(DevicePrivate* d) :
			device(d)
		{
			if (!rp)
			{
				auto fmt = Swapchain::get_format();
				RenderpassAttachmentInfo att;
				att.format = fmt;
				att.clear = true;
				RenderpassSubpassInfo sp;
				uint col_refs[] = {
					0
				};
				sp.color_attachments_count = 1;
				sp.color_attachments = col_refs;
				rp = new RenderpassPrivate(d, { &att, 1 }, { &sp,1 });
			}
			if (!dsl)
			{
				DescriptorBindingInfo db;
				db.type = DescriptorSampledImage;
				db.count = resources_count;
				db.name = "images";
				dsl = new DescriptorSetLayoutPrivate(d, { &db, 1});
			}
			if (!pll)
				pll = new PipelineLayoutPrivate(d, { &dsl, 1 }, 16);
			if (!pl)
			{
				VertexAttributeInfo vias[3];
				auto& via1 = vias[0];
				via1.format = Format_R32G32_SFLOAT;
				via1.name = "pos";
				auto& via2 = vias[1];
				via2.format = Format_R32G32_SFLOAT;
				via2.name = "uv";
				auto& via3 = vias[2];
				via3.format = Format_R8G8B8A8_UNORM;
				via3.name = "color";
				VertexBufferInfo vib;
				vib.attributes_count = size(vias);
				vib.attributes = vias;
				VertexInfo vi;
				vi.buffers_count = 1;
				vi.buffers = &vib;
				vert = new ShaderPrivate(L"element.vert");
				frag = new ShaderPrivate(L"element.frag");
				ShaderPrivate* shaders[] = {
					vert,
					frag
				};
				wchar_t engine_path[260];
				get_engine_path(engine_path);
				pl = PipelinePrivate::create(d, std::filesystem::path(engine_path) / L"shaders", shaders, pll, rp, 0, &vi);
			}

			buf_vtx.reset(new BufferPrivate(d, 3495200, BufferUsageVertex, MemoryPropertyHost | MemoryPropertyCoherent));
			buf_vtx->map();
			buf_idx.reset(new BufferPrivate(d, 1048576, BufferUsageIndex, MemoryPropertyHost | MemoryPropertyCoherent));
			buf_idx->map();

			img_white.reset(new ImagePrivate(d, Format_R8G8B8A8_UNORM, Vec2u(4), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
			img_white->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, Vec4c(255));
			resources.resize(resources_count);
			auto iv_white = img_white->default_view.get();
			for (auto i = 0; i < resources_count; i++)
			{
				auto r = new CanvasResourcePrivate;
				r->view = iv_white;
				resources[i].reset(r);
			}

			ds.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), dsl));
			auto sp = d->sampler_linear.get();
			for (auto i = 0; i < resources_count; i++)
				ds->set_image(0, i, iv_white, sp);
		}

		void CanvasBridge::set_target(uint views_count, ImageView* const* views)
		{ 
			((CanvasPrivate*)this)->set_target({ (ImageViewPrivate**)views, views_count });
		}

		void CanvasPrivate::set_target(std::span<ImageViewPrivate*> views)
		{
			fbs.clear();

			if (views.empty())
				target_size = 0.f;
			else
			{
				target_size = views[0]->image->size;
				fbs.resize(views.size());
				for (auto i = 0; i < fbs.size(); i++)
					fbs[i].reset(new FramebufferPrivate(device, rp, { &views[i], 1 }));
			}
		}

		uint CanvasBridge::set_resource(int slot, ImageView* v, Sampler* sp, const wchar_t* filename, ImageAtlas* image_atlas, FontAtlas* font_atlas)
		{ 
			return ((CanvasPrivate*)this)->set_resource(slot, (ImageViewPrivate*)v, (SamplerPrivate*)sp, filename ? filename : L"", (ImageAtlasPrivate*)image_atlas, (FontAtlasPrivate*)font_atlas);
		}

		uint CanvasPrivate::set_resource(int slot, ImageViewPrivate* v, SamplerPrivate* sp, const std::filesystem::path& filename, ImageAtlasPrivate* image_atlas, FontAtlasPrivate* font_atlas)
		{
			if (resources.empty())
				return -1;
			auto iv_white = img_white->default_view.get();
			if (slot == -1)
			{
				assert(v);
				for (auto i = 0; i < resources.size(); i++)
				{
					if (resources[i]->view == iv_white)
					{
						slot = i;
						break;
					}
				}
				assert(slot != -1);
			}
			auto r = new CanvasResourcePrivate;
			if (v)
			{
				auto img = v->image;
				img->set_pixels(img->size - 1U, Vec2u(1), &Vec4c(255));
				r->white_uv = (Vec2f(img->size - 1U) + 0.5f) / Vec2f(img->size);
			}
			ds->set_image(0, slot, v, sp ? sp : device->sampler_linear.get());
			r->filename = filename;
			r->view = v ? v : iv_white;
			r->image_atlas = image_atlas;
			r->font_atlas = font_atlas;
			resources[slot].reset(r);
			return slot;
		}

		void CanvasBridge::add_atlas(ImageAtlas* a)
		{ 
			((CanvasPrivate*)this)->add_atlas((ImageAtlasPrivate*)a); 
		}

		void CanvasPrivate::add_atlas(ImageAtlasPrivate* a)
		{
			set_resource(-1, a->image->default_view.get(), a->border ? device->sampler_linear.get() : device->sampler_nearest.get(), "", a);
		}

		void CanvasBridge::add_font(FontAtlas* f)
		{ 
			((CanvasPrivate*)this)->add_font((FontAtlasPrivate*)f);
		}

		void CanvasPrivate::add_font(FontAtlasPrivate* f)
		{
			set_resource(-1, f->view.get(), device->sampler_nearest.get(), "", nullptr, f);
		}

		void CanvasPrivate::add_draw_cmd(int id)
		{
			auto equal = [&]() {
				if (cmds.empty())
					return false;
				auto& back = cmds.back();
				if (back.type == CmdDrawElement && (id == -1 || back.v.draw_data.id == id))
					return true;
				return false;
			};
			if (equal())
				return;
			Cmd cmd;
			cmd.type = CmdDrawElement;
			cmd.v.draw_data.id = id == -1 ? 0 : id;
			cmd.v.draw_data.vtx_cnt = 0;
			cmd.v.draw_data.idx_cnt = 0;
			cmds.push_back(cmd);
			auto& d = cmds.back().v.draw_data;
			p_vtx_cnt = &d.vtx_cnt;
			p_idx_cnt = &d.idx_cnt;
		}

		void CanvasPrivate::add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col)
		{
			auto& v = *vtx_end;
			v.pos = pos; 
			v.uv = uv; v.col = col; 
			vtx_end++;

			(*p_vtx_cnt)++;
		}

		void CanvasPrivate::add_idx(uint idx)
		{
			*idx_end = idx;
			idx_end++;

			(*p_idx_cnt)++;
		}

		static const auto feather = 1.f;

		void CanvasBridge::stroke(uint points_count, const Vec2f* points, const Vec4c& col, float thickness, bool aa) 
		{ 
			((CanvasPrivate*)this)->stroke({ points, points_count }, col, thickness, aa);
		}

		void CanvasPrivate::stroke(std::span<const Vec2f> points, const Vec4c& col, float thickness, bool aa)
		{
			if (points.size() < 2)
				return;

			thickness *= 0.5f;

			add_draw_cmd();
			auto vtx_cnt0 = *p_vtx_cnt;
			auto uv = resources[cmds.back().v.draw_data.id]->white_uv;

			auto closed = points[0] == points[points.size() - 1];

			std::vector<Vec2f> normals(points.size());
			for (auto i = 0; i < points.size() - 1; i++)
			{
				auto d = normalize(points[i + 1] - points[i]);
				auto normal = Vec2f(d.y(), -d.x());

				if (i > 0)
					normals[i] = normalize((normal + normals[i]) * 0.5f);
				else
					normals[i] = normal;

				if (closed && i + 1 == points.size() - 1)
					normals.front() = normals.back() = normalize((normal + normals[0]) * 0.5f);
				else
					normals[i + 1] = normal;
			}

			if (aa)
			{
				if (thickness > feather)
				{
					auto col_t = col;
					col_t.a() = 0;

					for (auto i = 0; i < points.size() - 1; i++)
					{
						if (i == 0)
						{
							auto p0 = points[i];
							auto p1 = points[i + 1];

							auto n0 = normals[i];
							auto n1 = normals[i + 1];

							auto vtx_cnt = *p_vtx_cnt;

							add_vtx(p0 + n0 * (thickness + feather), uv, col_t);
							add_vtx(p0 + n0 * (thickness - feather), uv, col);
							add_vtx(p0 - n0 * (thickness - feather), uv, col);
							add_vtx(p0 - n0 * (thickness + feather), uv, col_t);
							add_vtx(p1 + n1 * (thickness + feather), uv, col_t);
							add_vtx(p1 + n1 * (thickness - feather), uv, col);
							add_vtx(p1 - n1 * (thickness - feather), uv, col);
							add_vtx(p1 - n1 * (thickness + feather), uv, col_t);
							add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 6); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 5); add_idx(vtx_cnt + 6);
							add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 5); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 5);
							add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 7); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 6); add_idx(vtx_cnt + 7);
						}
						else if (closed && i == points.size() - 2)
						{
							auto vtx_cnt = *p_vtx_cnt;

							add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt0 + 2);
							add_idx(vtx_cnt - 4); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 3); add_idx(vtx_cnt - 4); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
							add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 3); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt0 + 3);
						}
						else
						{
							auto p1 = points[i + 1];

							auto n1 = normals[i + 1];

							auto vtx_cnt = *p_vtx_cnt;

							add_vtx(p1 + n1 * (thickness + feather), uv, col_t);
							add_vtx(p1 + n1 * (thickness - feather), uv, col);
							add_vtx(p1 - n1 * (thickness - feather), uv, col);
							add_vtx(p1 - n1 * (thickness + feather), uv, col_t);
							add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 2); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 2);
							add_idx(vtx_cnt - 4); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 3); add_idx(vtx_cnt - 4); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
							add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 3); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
						}
					}
				}
				else
				{
					auto col_c = col;
					col_c.a() = (thickness / feather) * 255;
					auto col_t = col;
					col_t.a() = 0;

					for (auto i = 0; i < points.size() - 1; i++)
					{
						if (i == 0)
						{
							auto p0 = points[i];
							auto p1 = points[i + 1];

							auto n0 = normals[i];
							auto n1 = normals[i + 1];

							auto vtx_cnt = *p_vtx_cnt;

							add_vtx(p0 + n0 * feather, uv, col_t);
							add_vtx(p0, uv, col_c);
							add_vtx(p0 - n0 * feather, uv, col_t);
							add_vtx(p1 + n1 * feather, uv, col_t);
							add_vtx(p1, uv, col_c);
							add_vtx(p1 - n1 * feather, uv, col_t);
							add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 4);
							add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 5); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 5);
						}
						else if (closed && i == points.size() - 2)
						{
							auto vtx_cnt = *p_vtx_cnt;

							add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
							add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt0 + 2);
						}
						else
						{
							auto p1 = points[i + 1];

							auto n1 = normals[i + 1];

							auto vtx_cnt = *p_vtx_cnt;

							add_vtx(p1 + n1 * feather, uv, col_t);
							add_vtx(p1, uv, col_c);
							add_vtx(p1 - n1 * feather, uv, col_t);
							add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
							add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 2); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 2);
						}
					}
				}
			}
			else
			{
				for (auto i = 0; i < points.size() - 1; i++)
				{
					if (i == 0)
					{
						auto p0 = points[i];
						auto p1 = points[i + 1];

						auto n0 = normals[i];
						auto n1 = normals[i + 1];

						auto vtx_cnt = *p_vtx_cnt;

						add_vtx(p0 + n0 * thickness, uv, col);
						add_vtx(p0 - n0 * thickness, uv, col);
						add_vtx(p1 + n1 * thickness, uv, col);
						add_vtx(p1 - n1 * thickness, uv, col);
						add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
					}
					else if (closed && i == points.size() - 2)
					{
						auto vtx_cnt = *p_vtx_cnt;

						add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
					}
					else
					{
						auto p1 = points[i + 1];

						auto n1 = normals[i + 1];

						auto vtx_cnt = *p_vtx_cnt;

						add_vtx(p1 + n1 * thickness, uv, col);
						add_vtx(p1 - n1 * thickness, uv, col);
						add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
					}
				}
			}
		}

		void CanvasBridge::fill(uint points_count, const Vec2f* points, const Vec4c& col, bool aa)
		{ 
			((CanvasPrivate*)this)->fill({ points, points_count }, col, aa);
		}

		void CanvasPrivate::fill(std::span<const Vec2f> points, const Vec4c& col, bool aa)
		{
			if (points.size() < 3)
				return;

			add_draw_cmd();
			auto vtx_cnt0 = *p_vtx_cnt;
			auto uv = resources[cmds.back().v.draw_data.id]->white_uv;

			if (aa)
			{
				std::vector<Vec2f> normals(points.size() + 1);
				for (auto i = 0; i < points.size(); i++)
				{
					auto d = -normalize((i + 1 == points.size() ? points[0] : points[i + 1]) - points[i]);
					auto normal = Vec2f(d.y(), -d.x());

					if (i > 0)
						normals[i] = normalize((normal + normals[i]) * 0.5f);
					else
						normals[i] = normal;

					if (i + 1 == points.size())
						normals.front() = normals.back() = normalize((normal + normals[0]) * 0.5f);
					else
						normals[i + 1] = normal;
				}

				auto col_t = col;
				col_t.a() = 0;

				for (auto i = 0; i < points.size(); i++)
				{
					if (i == 0)
					{
						auto p0 = points[i];
						auto p1 = i + 1 == points.size() ? points[0] : points[i + 1];

						auto n0 = normals[i];
						auto n1 = normals[i + 1];

						auto vtx_cnt = *p_vtx_cnt;

						add_vtx(p0, uv, col);
						add_vtx(p0 + n0 * feather, uv, col_t);
						add_vtx(p1, uv, col);
						add_vtx(p1 + n1 * feather, uv, col_t);
						add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
					}
					else if (i == points.size() - 1)
					{
						auto vtx_cnt = *p_vtx_cnt;

						add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
					}
					else
					{
						auto p1 = i + 1 == points.size() ? points[0] : points[i + 1];

						auto n1 = normals[i + 1];

						auto vtx_cnt = *p_vtx_cnt;

						add_vtx(p1, uv, col);
						add_vtx(p1 + n1 * feather, uv, col_t);
						add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
					}
				}
			}

			for (auto i = 0; i < points.size() - 2; i++)
			{
				auto vtx_cnt = *p_vtx_cnt;

				add_vtx(points[0], uv, col);
				add_vtx(points[i + 1], uv, col);
				add_vtx(points[i + 2], uv, col);
				add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1);
			}
		}

		void CanvasBridge::add_text(uint res_id, const wchar_t* text, uint size, const Vec2f& pos, const Vec4c& col)
		{ 
			((CanvasPrivate*)this)->add_text(res_id, text, size, pos, col);
		}

		void CanvasPrivate::add_text(uint res_id, const wchar_t* text, uint font_size, const Vec2f& pos, const Vec4c& col)
		{
			auto atlas = resources[res_id]->font_atlas;

			add_draw_cmd(res_id);

			auto _pos = pos;

			while (*text)
			{
				auto ch = *text;
				if (!ch)
					break;
				if (ch == '\n')
				{
					_pos.y() += font_size;
					_pos.x() = pos.x();
				}
				else if (ch != '\r')
				{
					if (ch == '\t')
						ch = ' ';

					auto g = atlas->_get_glyph(ch, font_size);

					auto p = _pos + Vec2f(g->off);
					auto size = Vec2f(g->size);
					if (rect_overlapping(Vec4f(Vec2f(p.x(), p.y() - size.y()), Vec2f(p.x() + size.x(), p.y())), curr_scissor))
					{
						auto uv = g->uv;
						auto uv0 = Vec2f(uv.x(), uv.y());
						auto uv1 = Vec2f(uv.z(), uv.w());

						auto vtx_cnt = *p_vtx_cnt;

						add_vtx(p, uv0, col);
						add_vtx(p + Vec2f(0.f, -size.y()), Vec2f(uv0.x(), uv1.y()), col);
						add_vtx(p + Vec2f(size.x(), -size.y()), uv1, col);
						add_vtx(p + Vec2f(size.x(), 0.f), Vec2f(uv1.x(), uv0.y()), col);
						add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2);
					}

					_pos.x() += g->advance;
				}

				text++;
			}
		}

		void CanvasBridge::add_image(uint res_id, uint tile_id, const Vec2f& pos, const Vec2f& size, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col)
		{ 
			((CanvasPrivate*)this)->add_image(res_id, tile_id, pos, size, uv0, uv1, tint_col);
		}

		void CanvasPrivate::add_image(uint res_id, uint tile_id, const Vec2f& pos, const Vec2f& size, Vec2f uv0, Vec2f uv1, const Vec4c& tint_col)
		{
			res_id = min(res_id, resources_count - 1);
			auto atlas = resources[res_id]->image_atlas;
			if (atlas)
			{
				auto tile = atlas->tiles[tile_id].get();
				auto tuv = tile->uv;
				auto tuv0 = Vec2f(tuv.x(), tuv.y());
				auto tuv1 = Vec2f(tuv.z(), tuv.w());
				uv0 = mix(tuv0, tuv1, uv0);
				uv1 = mix(tuv0, tuv1, uv1);
			}

			add_draw_cmd(res_id);

			auto vtx_cnt = *p_vtx_cnt;

			add_vtx(pos, uv0, tint_col);
			add_vtx(pos + Vec2f(0.f, size.y()), Vec2f(uv0.x(), uv1.y()), tint_col);
			add_vtx(pos + Vec2f(size.x(), size.y()), uv1, tint_col);
			add_vtx(pos + Vec2f(size.x(), 0.f), Vec2f(uv1.x(), uv0.y()), tint_col);
			add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2);
		}

		void CanvasPrivate::set_scissor(const Vec4f& _scissor)
		{
			auto scissor = Vec4f(
				max(_scissor.x(), 0.f),
				max(_scissor.y(), 0.f),
				min(_scissor.z(), (float)target_size.x()),
				min(_scissor.w(), (float)target_size.y()));
			if (scissor == curr_scissor)
				return;
			curr_scissor = scissor;
			Cmd cmd;
			cmd.type = CmdSetScissor;
			cmd.v.scissor = scissor;
			cmds.push_back(cmd);
		}

		void CanvasPrivate::prepare()
		{
			vtx_end = (Vertex*)buf_vtx->get_mapped();
			idx_end = (uint*)buf_idx->get_mapped();

			curr_scissor = Vec4f(Vec2f(0.f), Vec2f(target_size));

			cmds.clear();
		}

		void CanvasBridge::record(CommandBuffer* cb, uint image_index)
		{ 
			((CanvasPrivate*)this)->record((CommandBufferPrivate*)cb, image_index);
		}

		void CanvasPrivate::record(CommandBufferPrivate* cb, uint image_index)
		{
			cb->begin();
			cb->begin_renderpass(fbs[image_index].get(), { &clear_color, 1 });
			if (idx_end != buf_idx->get_mapped())
			{
				cb->set_viewport(curr_scissor);
				cb->set_scissor(curr_scissor);
				cb->bind_vertex_buffer(buf_vtx.get(), 0);
				cb->bind_index_buffer(buf_idx.get(), IndiceTypeUint);

				auto scale = Vec2f(2.f / target_size.x(), 2.f / target_size.y());
				cb->bind_pipeline(pl);
				cb->push_constant(0, sizeof(Vec2f), &scale, pll);
				cb->bind_descriptor_set(ds.get(), 0, pll);

				auto vtx_off = 0;
				auto idx_off = 0;
				for (auto& cmd : cmds)
				{
					switch (cmd.type)
					{
					case CmdDrawElement:
						if (cmd.v.draw_data.idx_cnt > 0)
						{
							cb->draw_indexed(cmd.v.draw_data.idx_cnt, idx_off, vtx_off, 1, cmd.v.draw_data.id);
							vtx_off += cmd.v.draw_data.vtx_cnt;
							idx_off += cmd.v.draw_data.idx_cnt;
						}
						break;
					case CmdSetScissor:
						cb->set_scissor(cmd.v.scissor);
						break;
					}
				}
			}
			cb->end_renderpass();
			cb->end();

			cmds.clear();
		}

		Canvas* Canvas::create(Device* d)
		{
			return new CanvasPrivate((DevicePrivate*)d);
		}
	}
}
