#include <flame/serialize.h>
#include "device_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "font_private.h"
#include "renderpass_private.h"
#include "shader_private.h"
#include "commandbuffer_private.h"
#include "swapchain_private.h"
#include "canvas_private.h"

namespace flame
{
	namespace graphics
	{
		const auto resource_count = 64U;
		static RenderpassPrivate* rp = nullptr;
		static DescriptorlayoutPrivate* dsl = nullptr;
		static PipelinelayoutPrivate* pll = nullptr;
		static ShaderPrivate* vert = nullptr;
		static ShaderPrivate* frag = nullptr;
		static PipelinePrivate* pl = nullptr;

		CanvasPrivate::CanvasPrivate(DevicePrivate* d) :
			_d(d),
			_clear_color(0.f, 0.f, 0.f, 1.f)
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
				db.count = resource_count;
				db.name = "images";
				dsl = new DescriptorlayoutPrivate(d, { &db, 1});
			}
			if (!pll)
				pll = new PipelinelayoutPrivate(d, { &dsl, 1 }, 16);
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
				vib.attributes_count = array_size(vias);
				vib.attributes = vias;
				VertexInputInfo vi;
				vi.buffer_count = 1;
				vi.buffers = &vib;
				vert = new ShaderPrivate(L"element.vert");
				frag = new ShaderPrivate(L"element.frag");
				ShaderPrivate* shaders[] = {
					vert,
					frag
				};
				pl = new PipelinePrivate(d, std::filesystem::path(get_engine_path()) / L"shaders", shaders, pll, rp, 0, &vi);
			}

			_buf_vtx.reset(new BufferPrivate(d, 3495200, BufferUsageVertex, MemPropHost | MemPropHostCoherent));
			_buf_vtx->map();
			_buf_idx.reset(new BufferPrivate(d, 1048576, BufferUsageIndex, MemPropHost | MemPropHostCoherent));
			_buf_idx->map();

			_img_white.reset(new ImagePrivate(d, Format_R8G8B8A8_UNORM, Vec2u(4), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
			_img_white->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, Vec4c(255));
			_resources.resize(resource_count);
			auto iv_white = _img_white->dv.get();
			for (auto i = 0; i < resource_count; i++)
			{
				auto r = new CanvasResourcePrivate;
				r->_view = iv_white;
				r->_atlas = nullptr;
				r->_white_uv = 0.5f;
				_resources[i].reset(r);
			}

			_ds.reset(new DescriptorsetPrivate(d->default_descriptorpool.get(), dsl));
			auto sp = d->default_sampler_linear.get();
			for (auto i = 0; i < resource_count; i++)
				_ds->set_image(0, i, iv_white, sp);
		}

		void CanvasPrivate::_set_target(std::span<ImageviewPrivate*> views)
		{
			_fbs.clear();

			if (views.empty())
				_target_size = 0.f;
			else
			{
				_target_size = views[0]->image->size;
				_fbs.resize(views.size());
				for (auto i = 0; i < _fbs.size(); i++)
					_fbs[i].reset(new FramebufferPrivate(_d, rp, { &views[i], 1 }));
			}
		}

		uint CanvasPrivate::_set_resource(int slot, ImageviewPrivate* v, SamplerPrivate* sp, ImageAtlasPrivate* atlas)
		{
			if (_resources.empty())
				return -1;
			auto iv_white = _img_white->dv.get();
			if (slot == -1)
			{
				assert(v);
				for (auto i = 0; i < _resources.size(); i++)
				{
					if (_resources[i]->_view == iv_white)
					{
						slot = i;
						break;
					}
				}
				assert(slot != -1);
			}
			auto r = new CanvasResourcePrivate;
			if (!v)
			{
				v = iv_white;
				r->_white_uv = 0.5f;
				atlas = nullptr;
			}
			else
			{
				auto img = v->image;
				img->set_pixels(img->size - 1U, Vec2u(1), &Vec4c(255));
				r->_white_uv = (Vec2f(img->size - 1U) + 0.5f) / Vec2f(img->size);
			}
			_ds->set_image(0, slot, v, sp ? sp : _d->default_sampler_linear.get());
			r->_view = v;
			r->_atlas = atlas;
			_resources[slot].reset(r);
			return slot;
		}

		void CanvasPrivate::_add_atlas(ImageAtlasPrivate* a)
		{
			a->slot = _set_resource(-1, a->image->dv.get(), a->border ? _d->default_sampler_linear.get() : _d->default_sampler_nearest.get(), a);
		}

		void CanvasPrivate::_add_font(FontAtlasPrivate* f)
		{
			f->slot = _set_resource(-1, f->view.get(), _d->default_sampler_nearest.get());
		}

		void CanvasPrivate::_stroke(std::span<const Vec2f> points, const Vec4c& col, float thickness)
		{
			if (points.size() < 2)
				return;

			if (_cmds.empty() || _cmds.back().type == CmdSetScissor)
			{
				Cmd cmd;
				cmd.type = CmdDrawElement;
				cmd.v.draw_data.id = 0;
				cmd.v.draw_data.vtx_cnt = 0;
				cmd.v.draw_data.idx_cnt = 0;
				_cmds.push_back(cmd);
			}
			auto& vtx_cnt = _cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = _cmds.back().v.draw_data.idx_cnt;
			auto first_vtx_cnt = vtx_cnt;
			auto uv = _resources[_cmds.back().v.draw_data.id]->_white_uv;

			auto closed = points[0] == points[points.size() - 1];

			std::vector<Vec2f> normals(points.size());
			for (auto i = 0; i < points.size() - 1; i++)
			{
				auto d = normalize(points[i + 1] - points[i]);
				auto normal = Vec2f(d.y(), -d.x());

				if (i > 0)
					normals[i] = (normal + normals[i]) * 0.5f;
				else
					normals[i] = normal;

				if (closed && i + 1 == points.size() - 1)
					normals.front() = normals.back() = (normal + normals[0]) * 0.5f;
				else
					normals[i + 1] = normal;
			}

			for (auto i = 0; i < points.size() - 1; i++)
			{
				if (i == 0)
				{
					auto p0 = points[i];
					auto p1 = points[i + 1];

					auto n0 = normals[i] * thickness;
					auto n1 = normals[i + 1] * thickness;

					_vtx_end->pos = p0; _vtx_end->uv = uv; _vtx_end->col = col; _vtx_end++;
					_vtx_end->pos = p0 - n0; _vtx_end->uv = uv; _vtx_end->col = col; _vtx_end++;
					_vtx_end->pos = p1 - n1; _vtx_end->uv = uv; _vtx_end->col = col; _vtx_end++;
					_vtx_end->pos = p1; _vtx_end->uv = uv; _vtx_end->col = col; _vtx_end++;

					*_idx_end = vtx_cnt + 0; _idx_end++;
					*_idx_end = vtx_cnt + 2; _idx_end++;
					*_idx_end = vtx_cnt + 1; _idx_end++;
					*_idx_end = vtx_cnt + 0; _idx_end++;
					*_idx_end = vtx_cnt + 3; _idx_end++;
					*_idx_end = vtx_cnt + 2; _idx_end++;

					vtx_cnt += 4;
					idx_cnt += 6;
				}
				else if (closed && i == points.size() - 2)
				{
					*_idx_end = vtx_cnt - 1;		  _idx_end++;
					*_idx_end = first_vtx_cnt + 1; _idx_end++;
					*_idx_end = vtx_cnt - 2;		  _idx_end++;
					*_idx_end = vtx_cnt - 1;		  _idx_end++;
					*_idx_end = first_vtx_cnt + 0; _idx_end++;
					*_idx_end = first_vtx_cnt + 1; _idx_end++;

					idx_cnt += 6;
				}
				else
				{
					auto p1 = points[i + 1];

					auto n1 = normals[i + 1] * thickness;

					_vtx_end->pos = p1 - n1; _vtx_end->uv = uv; _vtx_end->col = col;  _vtx_end++;
					_vtx_end->pos = p1; _vtx_end->uv = uv; _vtx_end->col = col; _vtx_end++;

					*_idx_end = vtx_cnt - 1; _idx_end++;
					*_idx_end = vtx_cnt + 0; _idx_end++;
					*_idx_end = vtx_cnt - 2; _idx_end++;
					*_idx_end = vtx_cnt - 1; _idx_end++;
					*_idx_end = vtx_cnt + 1; _idx_end++;
					*_idx_end = vtx_cnt + 0; _idx_end++;

					vtx_cnt += 2;
					idx_cnt += 6;
				}
			}
		}

		void CanvasPrivate::_fill(std::span<const Vec2f> points, const Vec4c& col)
		{
			if (points.size() < 3)
				return;

			if (_cmds.empty() || _cmds.back().type == CmdSetScissor)
			{
				Cmd cmd;
				cmd.type = CmdDrawElement;
				cmd.v.draw_data.id = 0;
				cmd.v.draw_data.vtx_cnt = 0;
				cmd.v.draw_data.idx_cnt = 0;
				_cmds.push_back(cmd);
			}
			auto& vtx_cnt = _cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = _cmds.back().v.draw_data.idx_cnt;
			auto uv = _resources[_cmds.back().v.draw_data.id]->_white_uv;

			for (auto i = 0; i < points.size() - 2; i++)
			{
				_vtx_end->pos = points[0];	  _vtx_end->uv = uv; _vtx_end->col = col; _vtx_end++;
				_vtx_end->pos = points[i + 2]; _vtx_end->uv = uv; _vtx_end->col = col; _vtx_end++;
				_vtx_end->pos = points[i + 1]; _vtx_end->uv = uv; _vtx_end->col = col; _vtx_end++;

				*_idx_end = vtx_cnt + 0; _idx_end++;
				*_idx_end = vtx_cnt + 1; _idx_end++;
				*_idx_end = vtx_cnt + 2; _idx_end++;

				vtx_cnt += 3;
				idx_cnt += 3;
			}
		}

		void CanvasPrivate::_add_text(FontAtlasPrivate* f, std::wstring_view text, uint font_size, const Vec2f& _pos, const Vec4c& col)
		{
			auto pos = Vec2f(Vec2i(_pos));

			if (_cmds.empty() || _cmds.back().type != CmdDrawElement || _cmds.back().v.draw_data.id != f->slot)
			{
				Cmd cmd;
				cmd.type = CmdDrawElement;
				cmd.v.draw_data.id = f->slot;
				cmd.v.draw_data.vtx_cnt = 0;
				cmd.v.draw_data.idx_cnt = 0;
				_cmds.push_back(cmd);
			}
			auto& vtx_cnt = _cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = _cmds.back().v.draw_data.idx_cnt;

			auto pstr = text.begin();
			while (pstr != text.end())
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

					auto g = f->_get_glyph(ch, font_size);

					auto p = pos + Vec2f(g->off);
					auto size = Vec2f(g->size);
					if (rect_overlapping(Vec4f(Vec2f(p.x(), p.y() - size.y()), Vec2f(p.x() + size.x(), p.y())), _curr_scissor))
					{
						auto uv = g->uv;
						auto uv0 = Vec2f(uv.x(), uv.y());
						auto uv1 = Vec2f(uv.z(), uv.w());

						_vtx_end->pos = p;						       _vtx_end->uv = uv0;						_vtx_end->col = col; _vtx_end++;
						_vtx_end->pos = p + Vec2f(0.f, -size.y());	   _vtx_end->uv = Vec2f(uv0.x(), uv1.y());	_vtx_end->col = col; _vtx_end++;
						_vtx_end->pos = p + Vec2f(size.x(), -size.y()); _vtx_end->uv = uv1;						_vtx_end->col = col; _vtx_end++;
						_vtx_end->pos = p + Vec2f(size.x(), 0.f);	   _vtx_end->uv = Vec2f(uv1.x(), uv0.y());   _vtx_end->col = col; _vtx_end++;

						*_idx_end = vtx_cnt + 0; _idx_end++;
						*_idx_end = vtx_cnt + 2; _idx_end++;
						*_idx_end = vtx_cnt + 1; _idx_end++;
						*_idx_end = vtx_cnt + 0; _idx_end++;
						*_idx_end = vtx_cnt + 3; _idx_end++;
						*_idx_end = vtx_cnt + 2; _idx_end++;

						vtx_cnt += 4;
						idx_cnt += 6;
					}

					pos.x() += g->advance;
				}

				pstr++;
			}
		}

		void CanvasPrivate::_add_image(const Vec2f& _pos, const Vec2f& size, uint id, const Vec2f& _uv0, const Vec2f& _uv1, const Vec4c& tint_col)
		{
			auto pos = Vec2f(Vec2i(_pos));
			auto uv0 = _uv0;
			auto uv1 = _uv1;
			Vec2f img_size;

			auto img_id = (id & 0xffff0000) >> 16;
			if (_cmds.empty() || _cmds.back().type != CmdDrawElement || _cmds.back().v.draw_data.id != img_id)
			{
				Cmd cmd;
				cmd.type = CmdDrawElement;
				cmd.v.draw_data.id = img_id;
				cmd.v.draw_data.vtx_cnt = 0;
				cmd.v.draw_data.idx_cnt = 0;
				_cmds.push_back(cmd);
			}
			auto& vtx_cnt = _cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = _cmds.back().v.draw_data.idx_cnt;
			auto res = _resources[img_id].get();
			auto atlas = res->_atlas;
			if (atlas)
			{
				auto tile = atlas->tiles[id & 0xffff].get();
				auto tuv = tile->uv;
				auto tuv0 = Vec2f(tuv.x(), tuv.y());
				auto tuv1 = Vec2f(tuv.z(), tuv.w());
				uv0 = mix(tuv0, tuv1, uv0);
				uv1 = mix(tuv0, tuv1, uv1);
				img_size = tile->size;
			}
			else
				img_size = res->_view->image->size;

			_vtx_end->pos = pos;									_vtx_end->uv = uv0;						_vtx_end->col = tint_col; _vtx_end++;
			_vtx_end->pos = pos + Vec2f(0.f, size.y());			_vtx_end->uv = Vec2f(uv0.x(), uv1.y());	_vtx_end->col = tint_col; _vtx_end++;
			_vtx_end->pos = pos + Vec2f(size.x(), size.y());		_vtx_end->uv = uv1;						_vtx_end->col = tint_col; _vtx_end++;
			_vtx_end->pos = pos + Vec2f(size.x(), 0.f);			_vtx_end->uv = Vec2f(uv1.x(), uv0.y());	_vtx_end->col = tint_col; _vtx_end++;

			*_idx_end = vtx_cnt + 0; _idx_end++;
			*_idx_end = vtx_cnt + 2; _idx_end++;
			*_idx_end = vtx_cnt + 1; _idx_end++;
			*_idx_end = vtx_cnt + 0; _idx_end++;
			*_idx_end = vtx_cnt + 3; _idx_end++;
			*_idx_end = vtx_cnt + 2; _idx_end++;

			vtx_cnt += 4;
			idx_cnt += 6;
		}

		void CanvasPrivate::_set_scissor(const Vec4f& _scissor)
		{
			auto scissor = Vec4f(
				max(_scissor.x(), 0.f),
				max(_scissor.y(), 0.f),
				min(_scissor.z(), (float)_target_size.x()),
				min(_scissor.w(), (float)_target_size.y()));
			if (scissor == _curr_scissor)
				return;
			_curr_scissor = scissor;
			Cmd cmd;
			cmd.type = CmdSetScissor;
			cmd.v.scissor = scissor;
			_cmds.push_back(cmd);
		}

		void CanvasPrivate::_prepare()
		{
			_vtx_end = (Vertex*)_buf_vtx->get_mapped();
			_idx_end = (uint*)_buf_idx->get_mapped();

			_curr_scissor = Vec4f(Vec2f(0.f), Vec2f(_target_size));

			_cmds.clear();
		}

		void CanvasPrivate::_record(CommandbufferPrivate* cb, uint image_index)
		{
			cb->_begin();
			cb->_begin_renderpass(_fbs[image_index].get(), { &_clear_color, 1 });
			if (_idx_end != _buf_idx->get_mapped())
			{
				cb->_set_viewport(_curr_scissor);
				cb->_set_scissor(_curr_scissor);
				cb->_bind_vertexbuffer(_buf_vtx.get(), 0);
				cb->_bind_indexbuffer(_buf_idx.get(), IndiceTypeUint);

				auto scale = Vec2f(2.f / _target_size.x(), 2.f / _target_size.y());
				cb->_bind_pipeline(pl);
				cb->_push_constant(0, sizeof(Vec2f), &scale, pll);
				cb->_bind_descriptorset(_ds.get(), 0, pll);

				auto vtx_off = 0;
				auto idx_off = 0;
				for (auto& cmd : _cmds)
				{
					switch (cmd.type)
					{
					case CmdDrawElement:
						if (cmd.v.draw_data.idx_cnt > 0)
						{
							cb->_draw_indexed(cmd.v.draw_data.idx_cnt, idx_off, vtx_off, 1, cmd.v.draw_data.id);
							vtx_off += cmd.v.draw_data.vtx_cnt;
							idx_off += cmd.v.draw_data.idx_cnt;
						}
						break;
					case CmdSetScissor:
						cb->_set_scissor(cmd.v.scissor);
						break;
					}
				}
			}
			cb->_end_renderpass();
			cb->_end();

			_cmds.clear();
		}

		Canvas* Canvas::create(Device* d)
		{
			return new CanvasPrivate((DevicePrivate*)d);
		}
	}
}
