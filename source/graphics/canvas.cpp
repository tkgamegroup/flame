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
				pl = PipelinePrivate::_create(d, std::filesystem::path(engine_path) / L"shaders", shaders, pll, rp, 0, &vi);
			}

			_buf_vtx.reset(new BufferPrivate(d, 3495200, BufferUsageVertex, MemPropHost | MemPropHostCoherent));
			_buf_vtx->map();
			_buf_idx.reset(new BufferPrivate(d, 1048576, BufferUsageIndex, MemPropHost | MemPropHostCoherent));
			_buf_idx->map();

			_img_white.reset(new ImagePrivate(d, Format_R8G8B8A8_UNORM, Vec2u(4), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
			_img_white->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, Vec4c(255));
			_resources.resize(resource_count);
			auto iv_white = _img_white->_dv.get();
			for (auto i = 0; i < resource_count; i++)
			{
				auto r = new CanvasResourcePrivate;
				r->_view = iv_white;
				_resources[i].reset(r);
			}

			_ds.reset(new DescriptorsetPrivate(d->_descriptorpool.get(), dsl));
			auto sp = d->_sampler_linear.get();
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
				_target_size = views[0]->_image->_size;
				_fbs.resize(views.size());
				for (auto i = 0; i < _fbs.size(); i++)
					_fbs[i].reset(new FramebufferPrivate(_d, rp, { &views[i], 1 }));
			}
		}

		uint CanvasPrivate::_set_resource(int slot, ImageviewPrivate* v, SamplerPrivate* sp, ImageAtlasPrivate* image_atlas, FontAtlasPrivate* font_atlas)
		{
			if (_resources.empty())
				return -1;
			auto iv_white = _img_white->_dv.get();
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
			if (v)
			{
				auto img = v->_image;
				img->set_pixels(img->_size - 1U, Vec2u(1), &Vec4c(255));
				r->_white_uv = (Vec2f(img->_size - 1U) + 0.5f) / Vec2f(img->_size);
			}
			_ds->set_image(0, slot, v, sp ? sp : _d->_sampler_linear.get());
			r->_view = v ? v : iv_white;
			r->_image_atlas = image_atlas;
			r->_font_atlas = font_atlas;
			_resources[slot].reset(r);
			return slot;
		}

		void CanvasPrivate::_add_atlas(ImageAtlasPrivate* a)
		{
			_set_resource(-1, a->_image->_dv.get(), a->_border ? _d->_sampler_linear.get() : _d->_sampler_nearest.get(), a);
		}

		void CanvasPrivate::_add_font(FontAtlasPrivate* f)
		{
			_set_resource(-1, f->_view.get(), _d->_sampler_nearest.get(), nullptr, f);
		}

		void CanvasPrivate::_add_draw_cmd(int id)
		{
			auto equal = [&]() {
				if (_cmds.empty())
					return false;
				auto& back = _cmds.back();
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
			_cmds.push_back(cmd);
			auto& d = _cmds.back().v.draw_data;
			_p_vtx_cnt = &d.vtx_cnt;
			_p_idx_cnt = &d.idx_cnt;
		}

		void CanvasPrivate::_add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col)
		{
			auto& v = *_vtx_end;
			v.pos = pos; 
			v.uv = uv; v.col = col; 
			_vtx_end++;

			(*_p_vtx_cnt)++;
		}

		void CanvasPrivate::_add_idx(uint idx)
		{
			*_idx_end = idx;
			_idx_end++;

			(*_p_idx_cnt)++;
		}

		static const auto feather = 0.5f;

		void CanvasPrivate::_stroke(std::span<const Vec2f> points, const Vec4c& col, float thickness, bool aa)
		{
			if (points.size() < 2)
				return;

			thickness *= 0.5f;

			_add_draw_cmd();
			auto vtx_cnt0 = *_p_vtx_cnt;
			auto uv = _resources[_cmds.back().v.draw_data.id]->_white_uv;

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

							auto vtx_cnt = *_p_vtx_cnt;

							_add_vtx(p0 + n0 * (thickness + feather), uv, col_t);
							_add_vtx(p0 + n0 * (thickness - feather), uv, col);
							_add_vtx(p0 - n0 * (thickness - feather), uv, col);
							_add_vtx(p0 - n0 * (thickness + feather), uv, col_t);
							_add_vtx(p1 + n1 * (thickness + feather), uv, col_t);
							_add_vtx(p1 + n1 * (thickness - feather), uv, col);
							_add_vtx(p1 - n1 * (thickness - feather), uv, col);
							_add_vtx(p1 - n1 * (thickness + feather), uv, col_t);
							_add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 6); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 5); _add_idx(vtx_cnt + 6);
							_add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 5); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 4); _add_idx(vtx_cnt + 5);
							_add_idx(vtx_cnt + 2); _add_idx(vtx_cnt + 7); _add_idx(vtx_cnt + 3); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt + 6); _add_idx(vtx_cnt + 7);
						}
						else if (closed && i == points.size() - 2)
						{
							auto vtx_cnt = *_p_vtx_cnt;

							_add_idx(vtx_cnt - 3); _add_idx(vtx_cnt0 + 2); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt - 3); _add_idx(vtx_cnt0 + 1); _add_idx(vtx_cnt0 + 2);
							_add_idx(vtx_cnt - 4); _add_idx(vtx_cnt0 + 1); _add_idx(vtx_cnt - 3); _add_idx(vtx_cnt - 4); _add_idx(vtx_cnt0 + 0); _add_idx(vtx_cnt0 + 1);
							_add_idx(vtx_cnt - 2); _add_idx(vtx_cnt0 + 3); _add_idx(vtx_cnt - 1); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt0 + 2); _add_idx(vtx_cnt0 + 3);
						}
						else
						{
							auto p1 = points[i + 1];

							auto n1 = normals[i + 1];

							auto vtx_cnt = *_p_vtx_cnt;

							_add_vtx(p1 + n1 * (thickness + feather), uv, col_t);
							_add_vtx(p1 + n1 * (thickness - feather), uv, col);
							_add_vtx(p1 - n1 * (thickness - feather), uv, col);
							_add_vtx(p1 - n1 * (thickness + feather), uv, col_t);
							_add_idx(vtx_cnt - 3); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt - 3); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 2);
							_add_idx(vtx_cnt - 4); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt - 3); _add_idx(vtx_cnt - 4); _add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 1);
							_add_idx(vtx_cnt - 2); _add_idx(vtx_cnt + 3); _add_idx(vtx_cnt - 1); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt + 3);
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

							auto vtx_cnt = *_p_vtx_cnt;

							_add_vtx(p0 + n0 * thickness, uv, col_t);
							_add_vtx(p0, uv, col_c);
							_add_vtx(p0 - n0 * thickness, uv, col_t);
							_add_vtx(p1 + n1 * thickness, uv, col_t);
							_add_vtx(p1, uv, col_c);
							_add_vtx(p1 - n1 * thickness, uv, col_t);
							_add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 4); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 3); _add_idx(vtx_cnt + 4);
							_add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 5); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 4); _add_idx(vtx_cnt + 5);
						}
						else if (closed && i == points.size() - 2)
						{
							auto vtx_cnt = *_p_vtx_cnt;

							_add_idx(vtx_cnt - 3); _add_idx(vtx_cnt0 + 1); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt - 3); _add_idx(vtx_cnt0 + 0); _add_idx(vtx_cnt0 + 1);
							_add_idx(vtx_cnt - 2); _add_idx(vtx_cnt0 + 2); _add_idx(vtx_cnt - 1); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt0 + 1); _add_idx(vtx_cnt0 + 2);
						}
						else
						{
							auto p1 = points[i + 1];

							auto n1 = normals[i + 1];

							auto vtx_cnt = *_p_vtx_cnt;

							_add_vtx(p1 + n1 * thickness, uv, col_t);
							_add_vtx(p1, uv, col_c);
							_add_vtx(p1 - n1 * thickness, uv, col_t);
							_add_idx(vtx_cnt - 3); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt - 3); _add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 1);
							_add_idx(vtx_cnt - 2); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt - 1); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 2);
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

						auto vtx_cnt = *_p_vtx_cnt;

						_add_vtx(p0 + n0 * thickness, uv, col);
						_add_vtx(p0 - n0 * thickness, uv, col);
						_add_vtx(p1 + n1 * thickness, uv, col);
						_add_vtx(p1 - n1 * thickness, uv, col);
						_add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 3); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt + 3);
					}
					else if (closed && i == points.size() - 2)
					{
						auto vtx_cnt = *_p_vtx_cnt;

						_add_idx(vtx_cnt - 2); _add_idx(vtx_cnt0 + 1); _add_idx(vtx_cnt - 1); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt0 + 0); _add_idx(vtx_cnt0 + 1);
					}
					else
					{
						auto p1 = points[i + 1];

						auto n1 = normals[i + 1];

						auto vtx_cnt = *_p_vtx_cnt;

						_add_vtx(p1 + n1 * thickness, uv, col);
						_add_vtx(p1 - n1 * thickness, uv, col);
						_add_idx(vtx_cnt - 2); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt - 1); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 1);
					}
				}
			}
		}

		void CanvasPrivate::_fill(std::span<const Vec2f> points, const Vec4c& col, bool aa)
		{
			if (points.size() < 3)
				return;

			_add_draw_cmd();
			auto vtx_cnt0 = *_p_vtx_cnt;
			auto uv = _resources[_cmds.back().v.draw_data.id]->_white_uv;

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

						auto vtx_cnt = *_p_vtx_cnt;

						_add_vtx(p0, uv, col);
						_add_vtx(p0 + n0, uv, col_t);
						_add_vtx(p1, uv, col);
						_add_vtx(p1 + n1, uv, col_t);
						_add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 3); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt + 3);
					}
					else if (i == points.size() - 1)
					{
						auto vtx_cnt = *_p_vtx_cnt;

						_add_idx(vtx_cnt - 2); _add_idx(vtx_cnt0 + 1); _add_idx(vtx_cnt - 1); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt0 + 0); _add_idx(vtx_cnt0 + 1);
					}
					else
					{
						auto p1 = i + 1 == points.size() ? points[0] : points[i + 1];

						auto n1 = normals[i + 1];

						auto vtx_cnt = *_p_vtx_cnt;

						_add_vtx(p1, uv, col);
						_add_vtx(p1 + n1, uv, col_t);
						_add_idx(vtx_cnt - 2); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt - 1); _add_idx(vtx_cnt - 2); _add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 1);
					}
				}
			}

			for (auto i = 0; i < points.size() - 2; i++)
			{
				auto vtx_cnt = *_p_vtx_cnt;

				_add_vtx(points[0], uv, col);
				_add_vtx(points[i + 1], uv, col);
				_add_vtx(points[i + 2], uv, col);
				_add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt + 1);
			}
		}

		void CanvasPrivate::_add_text(uint res_id, const wchar_t* text, uint font_size, const Vec2f& pos, const Vec4c& col)
		{
			auto atlas = _resources[res_id]->_font_atlas;

			_add_draw_cmd(res_id);

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

					auto p = _pos + Vec2f(g->_off);
					auto size = Vec2f(g->_size);
					if (rect_overlapping(Vec4f(Vec2f(p.x(), p.y() - size.y()), Vec2f(p.x() + size.x(), p.y())), _curr_scissor))
					{
						auto uv = g->_uv;
						auto uv0 = Vec2f(uv.x(), uv.y());
						auto uv1 = Vec2f(uv.z(), uv.w());

						auto vtx_cnt = *_p_vtx_cnt;

						_add_vtx(p, uv0, col);
						_add_vtx(p + Vec2f(0.f, -size.y()), Vec2f(uv0.x(), uv1.y()), col);
						_add_vtx(p + Vec2f(size.x(), -size.y()), uv1, col);
						_add_vtx(p + Vec2f(size.x(), 0.f), Vec2f(uv1.x(), uv0.y()), col);
						_add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 3); _add_idx(vtx_cnt + 2);
					}

					_pos.x() += g->_advance;
				}

				text++;
			}
		}

		void CanvasPrivate::_add_image(uint res_id, uint tile_id, const Vec2f& pos, const Vec2f& size, Vec2f uv0, Vec2f uv1, const Vec4c& tint_col)
		{
			auto atlas = _resources[res_id]->_image_atlas;
			if (atlas)
			{
				auto tile = atlas->_tiles[tile_id].get();
				auto tuv = tile->_uv;
				auto tuv0 = Vec2f(tuv.x(), tuv.y());
				auto tuv1 = Vec2f(tuv.z(), tuv.w());
				uv0 = mix(tuv0, tuv1, uv0);
				uv1 = mix(tuv0, tuv1, uv1);
			}

			_add_draw_cmd(res_id);

			auto vtx_cnt = *_p_vtx_cnt;

			_add_vtx(pos, uv0, tint_col);
			_add_vtx(pos + Vec2f(0.f, size.y()), Vec2f(uv0.x(), uv1.y()), tint_col);
			_add_vtx(pos + Vec2f(size.x(), size.y()), uv1, tint_col);
			_add_vtx(pos + Vec2f(size.x(), 0.f), Vec2f(uv1.x(), uv0.y()), tint_col);
			_add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 2); _add_idx(vtx_cnt + 1); _add_idx(vtx_cnt + 0); _add_idx(vtx_cnt + 3); _add_idx(vtx_cnt + 2);
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
