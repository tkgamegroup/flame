#include <flame/serialize.h>
#include "device_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/swapchain.h>
#include "canvas_private.h"

namespace flame
{
	namespace graphics
	{
		const auto resource_count = 64U;
		static Renderpass* rp = nullptr;
		static Descriptorlayout* dsl = nullptr;
		static Pipelinelayout* pll = nullptr;
		static Pipeline* pl = nullptr;

		CanvasPrivate::CanvasPrivate(DevicePrivate* d) :
			d(d),
			clear_color(0.f, 0.f, 0.f, 1.f)
		{
			if (!rp)
			{
				auto fmt = Swapchain::get_format();
				AttachmentInfo att;
				att.format = fmt;
				att.clear = true;
				SubpassInfo sp;
				uint col_refs[] = {
					0
				};
				sp.color_attachment_count = 1;
				sp.color_attachments = col_refs;
				rp = Renderpass::create(d, 1, &att, 1, &sp, 0, nullptr);
			}
			if (!dsl)
			{
				DescriptorBinding db;
				db.type = DescriptorSampledImage;
				db.count = resource_count;
				db.name = "images";
				dsl = Descriptorlayout::create(d, 1, &db);
			}
			if (!pll)
				pll = Pipelinelayout::create(d, 1, &dsl, 16);
			if (!pl)
			{
				VertexInputAttribute vias[3];
				auto& via1 = vias[0];
				via1.format = Format_R32G32_SFLOAT;
				via1.name = "pos";
				auto& via2 = vias[1];
				via2.format = Format_R32G32_SFLOAT;
				via2.name = "uv";
				auto& via3 = vias[2];
				via3.format = Format_R8G8B8A8_UNORM;
				via3.name = "color";
				VertexInputBuffer vib;
				vib.attribute_count = array_size(vias);
				vib.attributes = vias;
				VertexInputInfo vi;
				vi.buffer_count = 1;
				vi.buffers = &vib;
				const wchar_t* shaders[] = {
					L"element.vert",
					L"element.frag"
				};
				pl = Pipeline::create(d, (std::filesystem::path(get_engine_path()) / L"shaders").c_str(), array_size(shaders), shaders, pll, rp, 0, &vi);
			}

			buf_vtx.reset(new BufferPrivate(d, 3495200, BufferUsageVertex, MemPropHost | MemPropHostCoherent));
			buf_vtx->map();
			buf_idx.reset(new BufferPrivate(d, 1048576, BufferUsageIndex, MemPropHost | MemPropHostCoherent));
			buf_idx->map();

			img_white.reset(new ImagePrivate(d, Format_R8G8B8A8_UNORM, Vec2u(4), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
			img_white->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, Vec4c(255));
			resources.resize(resource_count, { img_white->dv.get(), nullptr, Vec2f(0.5f) });

			ds = Descriptorset::create(Descriptorpool::get_default(), dsl);
			{
				auto iv_white = img_white->default_view();
				auto sp = Sampler::get_default(FilterLinear);
				for (auto i = 0; i < resource_count; i++)
					ds->set_image(0, i, iv_white, sp);
			}
		}

		void CanvasPrivate::set_target(const std::vector<Imageview*>& views)
		{
			for (auto f : fbs)
				Framebuffer::destroy(f);

			if (views.empty())
			{
				target_size = 0.f;
				fbs.clear();
			}
			else
			{
				target_size = views[0]->image()->size;
				fbs.resize(views.size());
				for (auto i = 0; i < fbs.size(); i++)
					fbs[i] = Framebuffer::create(d, rp, 1, &views[i]);
			}
		}

		uint CanvasPrivate::set_resource(int slot, Imageview* v, Sampler* sp, ImageAtlas* atlas)
		{
			if (resources.empty())
				return -1;
			auto iv_white = img_white->default_view();
			if (slot == -1)
			{
				assert(v);
				for (auto i = 0; i < resources.size(); i++)
				{
					if (resources[i].view == iv_white)
					{
						slot = i;
						break;
					}
				}
				assert(slot != -1);
			}
			Vec2f white_uv;
			if (!v)
			{
				v = iv_white;
				white_uv = 0.5f;
				atlas = nullptr;
			}
			else
			{
				auto img = v->image();
				img->set_pixels(img->size - 1U, Vec2u(1), &Vec4c(255));
				white_uv = (Vec2f(img->size - 1U) + 0.5f) / Vec2f(img->size);
			}
			ds->set_image(0, slot, v, sp ? sp : Sampler::get_default(FilterLinear));
			resources[slot] = { v, atlas, white_uv };
			return slot;
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

		void CanvasPrivate::stroke(uint point_count, const Vec2f* points, const Vec4c& col, float thickness)
		{
			if (point_count < 2)
				return;

			if (cmds.empty() || cmds.back().type == CmdSetScissor)
			{
				Cmd cmd;
				cmd.type = CmdDrawElement;
				cmd.v.draw_data.id = 0;
				cmd.v.draw_data.vtx_cnt = 0;
				cmd.v.draw_data.idx_cnt = 0;
				cmds.push_back(cmd);
			}
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;
			auto first_vtx_cnt = vtx_cnt;
			auto uv = resources[cmds.back().v.draw_data.id].white_uv;

			auto closed = points[0] == points[point_count - 1];

			std::vector<Vec2f> normals(point_count);
			for (auto i = 0; i < point_count - 1; i++)
			{
				auto d = normalize(points[i + 1] - points[i]);
				auto normal = Vec2f(d.y(), -d.x());

				if (i > 0)
					normals[i] = (normal + normals[i]) * 0.5f;
				else
					normals[i] = normal;

				if (closed && i + 1 == point_count - 1)
					normals.front() = normals.back() = (normal + normals[0]) * 0.5f;
				else
					normals[i + 1] = normal;
			}

			for (auto i = 0; i < point_count - 1; i++)
			{
				if (i == 0)
				{
					auto p0 = points[i];
					auto p1 = points[i + 1];

					auto n0 = normals[i] * thickness;
					auto n1 = normals[i + 1] * thickness;

					vtx_end->pos = p0; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;
					vtx_end->pos = p0 - n0; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;
					vtx_end->pos = p1 - n1; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;
					vtx_end->pos = p1; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;

					*idx_end = vtx_cnt + 0; idx_end++;
					*idx_end = vtx_cnt + 2; idx_end++;
					*idx_end = vtx_cnt + 1; idx_end++;
					*idx_end = vtx_cnt + 0; idx_end++;
					*idx_end = vtx_cnt + 3; idx_end++;
					*idx_end = vtx_cnt + 2; idx_end++;

					vtx_cnt += 4;
					idx_cnt += 6;
				}
				else if (closed && i == point_count - 2)
				{
					*idx_end = vtx_cnt - 1;		  idx_end++;
					*idx_end = first_vtx_cnt + 1; idx_end++;
					*idx_end = vtx_cnt - 2;		  idx_end++;
					*idx_end = vtx_cnt - 1;		  idx_end++;
					*idx_end = first_vtx_cnt + 0; idx_end++;
					*idx_end = first_vtx_cnt + 1; idx_end++;

					idx_cnt += 6;
				}
				else
				{
					auto p1 = points[i + 1];

					auto n1 = normals[i + 1] * thickness;

					vtx_end->pos = p1 - n1; vtx_end->uv = uv; vtx_end->col = col;  vtx_end++;
					vtx_end->pos = p1; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;

					*idx_end = vtx_cnt - 1; idx_end++;
					*idx_end = vtx_cnt + 0; idx_end++;
					*idx_end = vtx_cnt - 2; idx_end++;
					*idx_end = vtx_cnt - 1; idx_end++;
					*idx_end = vtx_cnt + 1; idx_end++;
					*idx_end = vtx_cnt + 0; idx_end++;

					vtx_cnt += 2;
					idx_cnt += 6;
				}
			}
		}

		void CanvasPrivate::fill(uint point_count, const Vec2f* points, const Vec4c& col)
		{
			if (point_count < 3)
				return;

			if (cmds.empty() || cmds.back().type == CmdSetScissor)
			{
				Cmd cmd;
				cmd.type = CmdDrawElement;
				cmd.v.draw_data.id = 0;
				cmd.v.draw_data.vtx_cnt = 0;
				cmd.v.draw_data.idx_cnt = 0;
				cmds.push_back(cmd);
			}
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;
			auto uv = resources[cmds.back().v.draw_data.id].white_uv;

			for (auto i = 0; i < point_count - 2; i++)
			{
				vtx_end->pos = points[0];	  vtx_end->uv = uv; vtx_end->col = col; vtx_end++;
				vtx_end->pos = points[i + 2]; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;
				vtx_end->pos = points[i + 1]; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;

				*idx_end = vtx_cnt + 0; idx_end++;
				*idx_end = vtx_cnt + 1; idx_end++;
				*idx_end = vtx_cnt + 2; idx_end++;

				vtx_cnt += 3;
				idx_cnt += 3;
			}
		}

		void CanvasPrivate::add_text(FontAtlas* f, const wchar_t* text_begin, const wchar_t* text_end, uint font_size, const Vec2f& _pos, const Vec4c& col)
		{
			auto pos = Vec2f(Vec2i(_pos));

			if (cmds.empty() || cmds.back().type != CmdDrawElement || cmds.back().v.draw_data.id != f->canvas_slot_)
			{
				Cmd cmd;
				cmd.type = CmdDrawElement;
				cmd.v.draw_data.id = f->canvas_slot_;
				cmd.v.draw_data.vtx_cnt = 0;
				cmd.v.draw_data.idx_cnt = 0;
				cmds.push_back(cmd);
			}
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;

			auto pstr = text_begin;
			while (pstr != text_end)
			{
				auto ch = *pstr;
				if (!ch)
					break;
				if (ch == '\n')
				{
					pos.y() += font_size;
					pos.x() = (int)_pos.x();
				}
				else if (ch != '\r')
				{
					if (ch == '\t')
						ch = ' ';

					auto g = f->get_glyph(ch, font_size);

					auto p = pos + Vec2f(g->off);
					auto size = Vec2f(g->size);
					if (rect_overlapping(Vec4f(Vec2f(p.x(), p.y() - size.y()), Vec2f(p.x() + size.x(), p.y())), curr_scissor))
					{
						vtx_end->pos = p;						       vtx_end->uv = g->uv0;						  vtx_end->col = col; vtx_end++;
						vtx_end->pos = p + Vec2f(0.f, -size.y());	   vtx_end->uv = Vec2f(g->uv0.x(), g->uv1.y());   vtx_end->col = col; vtx_end++;
						vtx_end->pos = p + Vec2f(size.x(), -size.y()); vtx_end->uv = g->uv1;						  vtx_end->col = col; vtx_end++;
						vtx_end->pos = p + Vec2f(size.x(), 0.f);	   vtx_end->uv = Vec2f(g->uv1.x(), g->uv0.y());   vtx_end->col = col; vtx_end++;

						*idx_end = vtx_cnt + 0; idx_end++;
						*idx_end = vtx_cnt + 2; idx_end++;
						*idx_end = vtx_cnt + 1; idx_end++;
						*idx_end = vtx_cnt + 0; idx_end++;
						*idx_end = vtx_cnt + 3; idx_end++;
						*idx_end = vtx_cnt + 2; idx_end++;

						vtx_cnt += 4;
						idx_cnt += 6;
					}

					pos.x() += g->advance;
				}

				pstr++;
			}
		}

		void CanvasPrivate::add_image(const Vec2f& _pos, const Vec2f& size, uint id, const Vec2f& _uv0, const Vec2f& _uv1, const Vec4c& tint_col)
		{
			auto pos = Vec2f(Vec2i(_pos));
			auto uv0 = _uv0;
			auto uv1 = _uv1;
			Vec2f img_size;

			auto img_id = (id & 0xffff0000) >> 16;
			if (cmds.empty() || cmds.back().type != CmdDrawElement || cmds.back().v.draw_data.id != img_id)
			{
				Cmd cmd;
				cmd.type = CmdDrawElement;
				cmd.v.draw_data.id = img_id;
				cmd.v.draw_data.vtx_cnt = 0;
				cmd.v.draw_data.idx_cnt = 0;
				cmds.push_back(cmd);
			}
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;
			auto& img = resources[img_id];
			auto atlas = img.atlas;
			if (atlas)
			{
				auto& tile = atlas->tile(id & 0xffff);
				uv0 = mix(tile.uv0, tile.uv1, uv0);
				uv1 = mix(tile.uv0, tile.uv1, uv1);
				img_size = tile.size;
			}
			else
				img_size = img.view->image()->size;

			vtx_end->pos = pos;									vtx_end->uv = uv0;						vtx_end->col = tint_col; vtx_end++;
			vtx_end->pos = pos + Vec2f(0.f, size.y());			vtx_end->uv = Vec2f(uv0.x(), uv1.y());	vtx_end->col = tint_col; vtx_end++;
			vtx_end->pos = pos + Vec2f(size.x(), size.y());		vtx_end->uv = uv1;						vtx_end->col = tint_col; vtx_end++;
			vtx_end->pos = pos + Vec2f(size.x(), 0.f);			vtx_end->uv = Vec2f(uv1.x(), uv0.y());	vtx_end->col = tint_col; vtx_end++;

			*idx_end = vtx_cnt + 0; idx_end++;
			*idx_end = vtx_cnt + 2; idx_end++;
			*idx_end = vtx_cnt + 1; idx_end++;
			*idx_end = vtx_cnt + 0; idx_end++;
			*idx_end = vtx_cnt + 3; idx_end++;
			*idx_end = vtx_cnt + 2; idx_end++;

			vtx_cnt += 4;
			idx_cnt += 6;
		}

		void CanvasPrivate::prepare()
		{
			vtx_end = (Vertex*)buf_vtx->get_mapped();
			idx_end = (uint*)buf_idx->get_mapped();

			curr_scissor = Vec4f(Vec2f(0.f), Vec2f(target_size));

			cmds.clear();
		}

		void CanvasPrivate::record(Commandbuffer* cb, uint image_index)
		{
			cb->begin();
			cb->begin_renderpass(fbs[image_index], 1, &clear_color);
			if (idx_end != buf_idx->get_mapped())
			{
				cb->set_viewport(curr_scissor);
				cb->set_scissor(curr_scissor);
				cb->bind_vertexbuffer(buf_vtx, 0);
				cb->bind_indexbuffer(buf_idx, IndiceTypeUint);

				auto scale = Vec2f(2.f / target_size.x(), 2.f / target_size.y());
				cb->bind_pipeline(pl);
				cb->push_constant(0, sizeof(Vec2f), &scale, pll);
				cb->bind_descriptorset(ds, 0, pll);

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

		void Canvas::set_target(uint view_count, Imageview* const* views)
		{
			std::vector<Imageview*> vs(view_count);
			for (auto i = 0; i < view_count; i++)
				vs[i] = views[i];
			((CanvasPrivate*)this)->set_target(vs);
		}

		Canvas* Canvas::create(Device* d)
		{
			return new CanvasPrivate(d);
		}
	}
}
