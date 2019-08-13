#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>

namespace flame
{
	struct CanvasShaderPushconstantT$
	{
		Vec2f scale$;
		Vec2f sdf_range$;
	};

	namespace graphics
	{
		static SampleCount$ sample_count = SampleCount_8;

		struct Vertex
		{
			Vec2f pos;
			Vec2f uv;
			Vec4c col;
		};

		struct DrawCmd
		{
			DrawCmdType type;
			int id;
			int vtx_cnt;
			int idx_cnt;
			Vec4f scissor;

			DrawCmd(DrawCmdType _type, int _id) :
				type(_type),
				id(_id)
			{
				vtx_cnt = 0;
				idx_cnt = 0;
			}

			DrawCmd(const Vec4f& _scissor) :
				type(DrawCmdScissor),
				scissor(_scissor)
			{
			}
		};

		struct CanvasPrivate : Canvas
		{
			Device* d;
			BP* renderpath;

			RenderpassAndFramebuffer* rnf;
			Vec4c clear_color;

			Pipelinelayout* pll;
			Pipeline* pl_element;
			Pipeline* pl_text_lcd;
			Pipeline* pl_text_sdf;

			Descriptorset* ds;
			Imageview* white_iv;

			Buffer* vtx_buffer;
			Buffer* idx_buffer;
			Vertex* vtx_end;
			uint* idx_end;

			std::vector<DrawCmd> draw_cmds;

			CanvasPrivate(Device* d, TargetType$ type, const void* v) :
				d(d)
			{
				clear_color = Vec4c(0, 0, 0, 255);

				renderpath = BP::create_from_file(L"../renderpath/canvas/bp", true);
				renderpath->set_graphics_device(d);

				set_render_target(type, v);

				pll = (Pipelinelayout*)renderpath->find_output("pll.out")->data_p();
				ds = (Descriptorset*)renderpath->find_output("ds.out")->data_p();
				white_iv = (Imageview*)renderpath->find_output("ds_wrt.iv")->data_p();
				vtx_buffer = (Buffer*)renderpath->find_output("vtx_buf.out")->data_p();
				idx_buffer = (Buffer*)renderpath->find_output("idx_buf.out")->data_p();

				vtx_buffer->map();
				idx_buffer->map();
				vtx_end = (Vertex*)vtx_buffer->mapped;
				idx_end = (uint*)idx_buffer->mapped;
			}

			void set_render_target(TargetType$ type, const void* v)
			{
				renderpath->find_input("rt_dst.type")->set_data_i(type);
				renderpath->find_input("rt_dst.v")->set_data_p(v);
				renderpath->update();
				rnf = (RenderpassAndFramebuffer*)renderpath->find_output("rnf.out")->data_p();
				if (rnf)
					rnf->clearvalues()->set(0, clear_color);
				pl_element = (Pipeline*)renderpath->find_output("pl_element.out")->data_p();
				pl_text_lcd = (Pipeline*)renderpath->find_output("pl_text_lcd.out")->data_p();
				pl_text_sdf = (Pipeline*)renderpath->find_output("pl_text_sdf.out")->data_p();
			}

			void set_image(int index, Imageview* v)
			{
				ds->set_image(0, index, v ? v : white_iv, d->sp_bi_linear);
			}

			void start_cmd(DrawCmdType type, int id)
			{
				if (!draw_cmds.empty())
				{
					auto& last = draw_cmds.back();
					if (last.type == type && last.id == id)
						return;
				}
				draw_cmds.emplace_back(type, id);
			}

			void stroke(const std::vector<Vec2f>& points, const Vec4c& inner_col, const Vec4c& outter_col, float thickness)
			{
				if (points.size() < 2)
					return;

				start_cmd(DrawCmdElement, 0);
				auto& vtx_cnt = draw_cmds.back().vtx_cnt;
				auto& idx_cnt = draw_cmds.back().idx_cnt;
				auto first_vtx_cnt = vtx_cnt;

				auto closed = points.front() == points.back();

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

						auto n0 = normals[i] * thickness * 0.5f;
						auto n1 = normals[i + 1] * thickness * 0.5f;

						vtx_end->pos = p0 + n0; vtx_end->uv = Vec2f(0.5f); vtx_end->col = outter_col; vtx_end++;
						vtx_end->pos = p0 - n0; vtx_end->uv = Vec2f(0.5f); vtx_end->col = inner_col;  vtx_end++;
						vtx_end->pos = p1 - n1; vtx_end->uv = Vec2f(0.5f); vtx_end->col = inner_col;  vtx_end++;
						vtx_end->pos = p1 + n1; vtx_end->uv = Vec2f(0.5f); vtx_end->col = outter_col; vtx_end++;

						*idx_end = vtx_cnt + 0; idx_end++;
						*idx_end = vtx_cnt + 2; idx_end++;
						*idx_end = vtx_cnt + 1; idx_end++;
						*idx_end = vtx_cnt + 0; idx_end++;
						*idx_end = vtx_cnt + 3; idx_end++;
						*idx_end = vtx_cnt + 2; idx_end++;

						vtx_cnt += 4;
						idx_cnt += 6;
					}
					else if (!(closed && i + 1 == points.size() - 1))
					{
						auto p1 = points[i + 1];

						auto n1 = normals[i + 1] * thickness * 0.5f;

						vtx_end->pos = p1 - n1; vtx_end->uv = Vec2f(0.5f); vtx_end->col = inner_col;  vtx_end++;
						vtx_end->pos = p1 + n1; vtx_end->uv = Vec2f(0.5f); vtx_end->col = outter_col; vtx_end++;

						*idx_end = vtx_cnt - 1; idx_end++;
						*idx_end = vtx_cnt + 0; idx_end++;
						*idx_end = vtx_cnt - 2; idx_end++;
						*idx_end = vtx_cnt - 1; idx_end++;
						*idx_end = vtx_cnt + 1; idx_end++;
						*idx_end = vtx_cnt + 0; idx_end++;

						vtx_cnt += 2;
						idx_cnt += 6;
					}
					else
					{
						*idx_end = vtx_cnt - 1;		  idx_end++;
						*idx_end = first_vtx_cnt + 1; idx_end++;
						*idx_end = vtx_cnt - 2;		  idx_end++;
						*idx_end = vtx_cnt - 1;		  idx_end++;
						*idx_end = first_vtx_cnt + 0; idx_end++;
						*idx_end = first_vtx_cnt + 1; idx_end++;

						idx_cnt += 6;
					}
				}
			}

			void fill(const std::vector<Vec2f>& points, const Vec4c& col)
			{
				if (points.size() < 3)
					return;

				start_cmd(DrawCmdElement, 0);
				auto& vtx_cnt = draw_cmds.back().vtx_cnt;
				auto& idx_cnt = draw_cmds.back().idx_cnt;

				for (auto i = 0; i < points.size() - 2; i++)
				{
					vtx_end->pos = points[0];	  vtx_end->uv = Vec2f(0.5f); vtx_end->col = col; vtx_end++;
					vtx_end->pos = points[i + 2]; vtx_end->uv = Vec2f(0.5f); vtx_end->col = col; vtx_end++;
					vtx_end->pos = points[i + 1]; vtx_end->uv = Vec2f(0.5f); vtx_end->col = col; vtx_end++;

					*idx_end = vtx_cnt + 0; idx_end++;
					*idx_end = vtx_cnt + 1; idx_end++;
					*idx_end = vtx_cnt + 2; idx_end++;

					vtx_cnt += 3;
					idx_cnt += 3;
				}
			}

			Vec2f add_text(FontAtlas* f, const Vec2f& pos, const Vec4c& col, const std::wstring& text, float scale)
			{
				if (f->draw_type != FontDrawSdf)
					scale = 1.f;
				auto pixel_height = f->pixel_height * scale;

				auto _pos = Vec2f(Vec2i(pos));

				DrawCmdType dct;
				switch (f->draw_type)
				{
				case FontDrawPixel:
					dct = DrawCmdElement;
					break;
				case FontDrawLcd:
					dct = DrawCmdTextLcd;
					break;
				case FontDrawSdf:
					dct = DrawCmdTextSdf;
					break;
				default:
					assert(0);
				}
				start_cmd(dct, f->index);
				auto& vtx_cnt = draw_cmds.back().vtx_cnt;
				auto& idx_cnt = draw_cmds.back().idx_cnt;

				Vec2f rect(0.f, pixel_height);
				auto line_width = 0.f;

				for (auto ch : text)
				{
					if (ch == '\n')
					{
						_pos.y() += pixel_height;
						_pos.x() = pos.x();

						rect.y() += pixel_height;
						line_width = 0.f;
					}
					else
					{
						auto g = f->get_glyph(ch);
						auto size = Vec2f(g->size) * scale;

						auto p = _pos + Vec2f(g->off) * scale;
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

						auto w = g->advance * scale;
						_pos.x() += w;
						line_width += w;
						if (line_width > rect.x())
							rect.x() = line_width;
					}
				}

				return rect;
			}

			void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255))
			{
				auto _pos = Vec2f(Vec2i(pos));

				start_cmd(DrawCmdElement, id);
				auto& vtx_cnt = draw_cmds.back().vtx_cnt;
				auto& idx_cnt = draw_cmds.back().idx_cnt;

				vtx_end->pos = _pos;						vtx_end->uv = uv0;				  vtx_end->col = tint_col; vtx_end++;
				vtx_end->pos = _pos + Vec2f(0.f, size.y());	vtx_end->uv = Vec2f(uv0.x(), uv1.y()); vtx_end->col = tint_col; vtx_end++;
				vtx_end->pos = _pos + Vec2f(size.x(), size.y()); vtx_end->uv = uv1;				  vtx_end->col = tint_col; vtx_end++;
				vtx_end->pos = _pos + Vec2f(size.x(), 0.f);	vtx_end->uv = Vec2f(uv1.x(), uv0.y()); vtx_end->col = tint_col; vtx_end++;

				*idx_end = vtx_cnt + 0; idx_end++;
				*idx_end = vtx_cnt + 2; idx_end++;
				*idx_end = vtx_cnt + 1; idx_end++;
				*idx_end = vtx_cnt + 0; idx_end++;
				*idx_end = vtx_cnt + 3; idx_end++;
				*idx_end = vtx_cnt + 2; idx_end++;

				vtx_cnt += 4;
				idx_cnt += 6;
			}

			void add_image_stretch(const Vec2f& pos, const Vec2f& size, uint id, const Vec4f& border, const Vec4c& tint_col = Vec4c(255))
			{
				//auto image_size = share_data.image_views[id]->image()->size;

				//auto b_uv = Vec4f(Vec2f(border[0], border[1]) / image_size.x(),
				//	Vec2f(border[2], border[3]) / image_size.y());

				//// corners
				//add_image(pos, Vec2f(border[0], border[2]), id, Vec2f(0.f), Vec2f(b_uv[0], b_uv[2])); // LT
				//add_image(pos + Vec2f(0.f, size.y() - border[3]), Vec2f(border[0], border[3]), id, Vec2f(0.f, 1.f - b_uv[3]), Vec2f(b_uv[0], 1.f)); // LB
				//add_image(pos + Vec2f(size.x() - border[1], 0.f), Vec2f(border[1], border[2]), id, Vec2f(1.f - b_uv[1], 0.f), Vec2f(1.f, b_uv[2])); // RT
				//add_image(pos + Vec2f(size.x() - border[1], size.y() - border[3]), Vec2f(border[1], border[3]), id, Vec2f(1.f - b_uv[1], 1.f - b_uv[3]), Vec2f(1.f)); // RB

				//// borders
				//add_image(pos + Vec2f(0.f, border[2]), Vec2f(border[0], size.y() - border[2] - border[3]), id, Vec2f(0.f, b_uv[2]), Vec2f(b_uv[0], 1.f - b_uv[3])); // L
				//add_image(pos + Vec2f(size.x() - border[1], border[2]), Vec2f(border[1], size.y() - border[2] - border[3]), id, Vec2f(1.f - b_uv[1], b_uv[2]), Vec2f(1.f, 1.f - b_uv[3])); // R
				//add_image(pos + Vec2f(border[0], 0.f), Vec2f(size.x() - border[0] - border[1], border[2]), id, Vec2f(b_uv[0], 0.f), Vec2f(1.f - b_uv[1], b_uv[2])); // T
				//add_image(pos + Vec2f(border[0], size.y() - border[3]), Vec2f(size.x() - border[0] - border[1], border[3]), id, Vec2f(b_uv[0], 1.f - b_uv[3]), Vec2f(1.f - b_uv[1], 1.f)); // B

				//add_image(pos + Vec2f(border[0], border[2]), Vec2f(size.x() - border[0] - border[1], size.y() - border[2] - border[3]), id, Vec2f(b_uv[0], b_uv[2]), Vec2f(1.f - b_uv[1], 1.f - b_uv[3]));
			}

			void set_scissor(const Vec4f& scissor)
			{
				draw_cmds.emplace_back(scissor);
			}

			void record(Commandbuffer* cb, uint image_idx)
			{
				auto fb = (Framebuffer*)rnf->framebuffers()[image_idx];

				cb->begin();
				cb->begin_renderpass(rnf->renderpass(), fb, rnf->clearvalues());
				if (idx_end != idx_buffer->mapped)
				{
					auto surface_size = Vec2f(fb->image_size);

					cb->set_viewport(Vec4f(Vec2f(0.f), surface_size));
					cb->set_scissor(Vec4f(Vec2f(0.f), surface_size));
					cb->bind_vertexbuffer(vtx_buffer, 0);
					cb->bind_indexbuffer(idx_buffer, IndiceTypeUint);

					CanvasShaderPushconstantT$ pc;
					pc.scale$ = Vec2f(2.f / surface_size.x(), 2.f / surface_size.y());
					pc.sdf_range$ = Vec2f(4.f / 512.f); /* sdf_image->size */

					cb->push_constant(pll, 0, sizeof(CanvasShaderPushconstantT$), &pc);

					auto vtx_off = 0;
					auto idx_off = 0;
					for (auto& dc : draw_cmds)
					{
						switch (dc.type)
						{
						case DrawCmdElement:
							cb->bind_pipeline(pl_element);
							cb->bind_descriptorset(ds, 0);
							cb->draw_indexed(dc.idx_cnt, idx_off, vtx_off, 1, dc.id);
							vtx_off += dc.vtx_cnt;
							idx_off += dc.idx_cnt;
							break;
						case DrawCmdTextLcd:
							cb->bind_pipeline(pl_text_lcd);
							cb->bind_descriptorset(ds, 0);
							cb->draw_indexed(dc.idx_cnt, idx_off, vtx_off, 1, dc.id);
							vtx_off += dc.vtx_cnt;
							idx_off += dc.idx_cnt;
							break;
						case DrawCmdTextSdf:
							cb->bind_pipeline(pl_text_sdf);
							cb->bind_descriptorset(ds, 0);
							cb->draw_indexed(dc.idx_cnt, idx_off, vtx_off, 1, dc.id);
							vtx_off += dc.vtx_cnt;
							idx_off += dc.idx_cnt;
							break;
						case DrawCmdScissor:
							cb->set_scissor(dc.scissor);
							break;
						}
					}
				}
				cb->end_renderpass();
				cb->end();

				vtx_end = (Vertex*)vtx_buffer->mapped;
				idx_end = (uint*)idx_buffer->mapped;
				draw_cmds.clear();
			}
		};

		void Canvas::set_render_target(TargetType$ type, const void* v)
		{
			((CanvasPrivate*)this)->set_render_target(type, v);
		}

		void Canvas::set_clear_color(const Vec4c& col)
		{
			auto thiz = (CanvasPrivate*)this;
			thiz->clear_color = col;
			thiz->rnf->clearvalues()->set(0, col);
		}

		void Canvas::set_image(uint index, Imageview* v)
		{
			((CanvasPrivate*)this)->set_image(index, v);
		}

		void Canvas::stroke(const std::vector<Vec2f>& points, const Vec4c& inner_col, const Vec4c& outter_col, float thickness)
		{
			((CanvasPrivate*)this)->stroke(points, inner_col, outter_col, thickness);
		}

		void Canvas::fill(const std::vector<Vec2f>& points, const Vec4c& col)
		{
			((CanvasPrivate*)this)->fill(points, col);
		}

		Vec2f Canvas::add_text(FontAtlas* f, const Vec2f& pos, const Vec4c& col, const std::wstring& text, float scale)
		{
			return ((CanvasPrivate*)this)->add_text(f, pos, col, text, scale);
		}

		void Canvas::add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col)
		{
			((CanvasPrivate*)this)->add_image(pos, size, id, uv0, uv1, tint_col);
		}

		void Canvas::add_image_stretch(const Vec2f& pos, const Vec2f& size, uint id, const Vec4f& border, const Vec4c& tint_col)
		{
			((CanvasPrivate*)this)->add_image_stretch(pos, size, id, border, tint_col);
		}

		void Canvas::set_scissor(const Vec4f& scissor)
		{
			((CanvasPrivate*)this)->set_scissor(scissor);
		}

		void Canvas::record(Commandbuffer* cb, uint image_idx)
		{
			((CanvasPrivate*)this)->record(cb, image_idx);
		}

		Canvas* Canvas::create(Device* d, TargetType$ type, const void* v)
		{
			return new CanvasPrivate(d, type, v);
		}

		void Canvas::destroy(Canvas* c)
		{
			delete (CanvasPrivate*)c;
		}
	}
}
