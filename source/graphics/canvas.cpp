// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/foundation/window.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		static Vec2 circle_subdiv[36];

		const auto MaxImageviewCount = 64;

		static Device* device;
		static Image* white_image;
		static Imageview* white_imageview;
		static Pipeline* pl_element;
		static Pipeline* pl_text_lcd;
		static Pipeline* pl_text_sdf;

		struct Vertex
		{
			Vec2 pos;
			Vec2 uv;
			Bvec4 col;
		};

		struct DrawCmd
		{
			DrawCmdType type;
			int id;
			int vtx_cnt;
			int idx_cnt;
			Rect scissor;

			DrawCmd(DrawCmdType _type, int _id) :
				type(_type),
				id(_id)
			{
				vtx_cnt = 0;
				idx_cnt = 0;
			}

			DrawCmd(const Rect &_scissor) :
				type(DrawCmdScissor),
				scissor(_scissor)
			{
			}
		};

		struct CanvasPrivate : Canvas
		{
			Swapchain* sc;
			Descriptorset* ds;
			ClearValues* cv;

			std::vector<Vec2> points;
			std::vector<DrawCmd> draw_cmds;

			Buffer *vtx_buffer;
			Buffer *idx_buffer;
			Vertex *vtx_end;
			int *idx_end;
			Commandbuffer *cb;

			std::vector<std::tuple<Font*, bool, int>> fonts;

			inline CanvasPrivate(Swapchain *_sc)
			{
				sc = _sc;
				ds = Descriptorset::create(device->dp, pl_element->layout()->dsl(0));

				for (auto i = 0; i < MaxImageviewCount; i++)
				{
					//image_views[i] = white_imageview;
					ds->set_imageview(0, i, white_imageview, device->sp_bi_linear);
				}

				cv = ClearValues::create(sc->get_renderpass_clear());
				cv->set(0, Bvec4(0));

				vtx_buffer = Buffer::create(device, sizeof(Vertex) * 43690,
					BufferUsageVertex, MemPropHost | MemPropHostCoherent);
				idx_buffer = Buffer::create(device, sizeof(int) * 65536,
					BufferUsageIndex, MemPropHost | MemPropHostCoherent);
				vtx_buffer->map();
				idx_buffer->map();
				vtx_end = (Vertex*)vtx_buffer->mapped;
				idx_end = (int*)idx_buffer->mapped;

				cb = Commandbuffer::create(device->gcp);
			}

			inline ~CanvasPrivate()
			{
				Buffer::destroy(vtx_buffer);
				Buffer::destroy(idx_buffer);
				Commandbuffer::destroy(cb);
			}

			inline int add_font(Font* font)
			{
				auto image_idx = fonts.size() + 1;
				fonts.emplace_back(font, font->sdf(), (int)image_idx);

				ds->set_imageview(0, image_idx, Imageview::get(font->get_atlas()), device->sp_bi_linear);

				return image_idx - 1;
			}

			inline void start_cmd(DrawCmdType type, int id)
			{
				if (!draw_cmds.empty())
				{
					auto &last = draw_cmds.back();
					if (last.type == type && last.id == id)
						return;
				}
				draw_cmds.emplace_back(type, id);
			}

			inline void path_line_to(const Vec2 &p)
			{
				points.push_back(p);
			}

			inline void path_rect(const Vec2 &pos, const Vec2 &size, float round_radius, int round_flags)
			{
				if (round_radius == 0.f || round_flags == 0)
				{
					points.push_back(pos);
					points.push_back(pos + Vec2(size.x, 0.f));
					points.push_back(pos + size);
					points.push_back(pos + Vec2(0.f, size.y));
				}
				else
				{
					if (round_flags & Rect::SideNW)
						path_arc_to(pos + Vec2(round_radius), round_radius, 18, 27);
					else
						path_line_to(pos);
					if (round_flags & Rect::SideNE)
						path_arc_to(pos + Vec2(size.x - round_radius, round_radius), round_radius, 27, 35);
					else
						path_line_to(pos + Vec2(size.x, 0.f));
					if (round_flags & Rect::SideSE)
						path_arc_to(pos + size - Vec2(round_radius), round_radius, 0, 9);
					else
						path_line_to(pos + size);
					if (round_flags & Rect::SideSW)
						path_arc_to(pos + Vec2(round_radius, size.y - round_radius), round_radius, 9, 17);
					else
						path_line_to(pos + Vec2(0.f, size.y));
				}
			}

			inline void path_arc_to(const Vec2 &center, float radius, int a_min, int a_max)
			{
				for (auto a = a_min; a <= a_max; a++)
					points.push_back(center + circle_subdiv[a % FLAME_ARRAYSIZE(circle_subdiv)] * radius);
			}

			inline void path_bezier(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4, int level = 0)
			{
				auto dx = p4.x - p1.x;
				auto dy = p4.y - p1.y;
				auto d2 = ((p2.x - p4.x) * dy - (p2.y - p4.y) * dx);
				auto d3 = ((p3.x - p4.x) * dy - (p3.y - p4.y) * dx);
				d2 = (d2 >= 0) ? d2 : -d2;
				d3 = (d3 >= 0) ? d3 : -d3;
				if ((d2 + d3) * (d2 + d3) < 1.25f * (dx * dx + dy * dy))
				{
					if (points.empty())
						points.push_back(p1);
					points.push_back(p4);
				}
				else if (level < 10)
				{
					auto p12 = (p1 + p2) * 0.5f;
					auto p23 = (p2 + p3) * 0.5f;
					auto p34 = (p3 + p4) * 0.5f;
					auto p123 = (p12 + p23) * 0.5f;
					auto p234 = (p23 + p34) * 0.5f;
					auto p1234 = (p123 + p234) * 0.5f;

					path_bezier(p1, p12, p123, p1234, level + 1);
					path_bezier(p1234, p234, p34, p4, level + 1);
				}
			}

			inline void clear_path()
			{
				points.clear();
			}

			inline void stroke(const Bvec4 &col, float thickness, bool closed)
			{
				stroke_col2(col, col, thickness, closed);
			}

			inline void stroke_col2(const Bvec4 &inner_col, const Bvec4 &outter_col, float thickness, bool closed)
			{
				if (points.size() < 2)
					return;

				if (closed)
					points.push_back(points[0]);

				start_cmd(DrawCmdElement, 0);
				auto &vtx_cnt = draw_cmds.back().vtx_cnt;
				auto &idx_cnt = draw_cmds.back().idx_cnt;
				auto first_vtx_cnt = vtx_cnt;

				std::vector<Vec2> normals(points.size());
				for (auto i = 0; i < points.size() - 1; i++)
				{
					auto d = points[i + 1] - points[i];
					d.normalize();
					auto normal = Vec2(d.y, -d.x);

					if (i > 0)
						normals[i] = (normal + normals[i]) * 0.5f;
					else
						normals[i] = normal;
					normals[i].normalize();

					if (closed && i + 1 == points.size() - 1)
					{
						normals[0] = (normal + normals[0]) * 0.5f;
						normals[0].normalize();
						normals[i + 1] = normals[0];
					}
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

						vtx_end->pos = p0 + n0; vtx_end->uv = Vec2(0.5f); vtx_end->col = outter_col; vtx_end++;
						vtx_end->pos = p0 - n0; vtx_end->uv = Vec2(0.5f); vtx_end->col = inner_col;  vtx_end++;
						vtx_end->pos = p1 - n1; vtx_end->uv = Vec2(0.5f); vtx_end->col = inner_col;  vtx_end++;
						vtx_end->pos = p1 + n1; vtx_end->uv = Vec2(0.5f); vtx_end->col = outter_col; vtx_end++;

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

						vtx_end->pos = p1 - n1; vtx_end->uv = Vec2(0.5f); vtx_end->col = inner_col;  vtx_end++;
						vtx_end->pos = p1 + n1; vtx_end->uv = Vec2(0.5f); vtx_end->col = outter_col; vtx_end++;

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

			inline void fill(const Bvec4 &col)
			{
				if (points.size() < 3)
					return;

				start_cmd(DrawCmdElement, 0);
				auto &vtx_cnt = draw_cmds.back().vtx_cnt;
				auto &idx_cnt = draw_cmds.back().idx_cnt;

				for (auto i = 0; i < points.size() - 2; i++)
				{
					vtx_end->pos = points[0];	  vtx_end->uv = Vec2(0.5f); vtx_end->col = col; vtx_end++;
					vtx_end->pos = points[i + 2]; vtx_end->uv = Vec2(0.5f); vtx_end->col = col; vtx_end++;
					vtx_end->pos = points[i + 1]; vtx_end->uv = Vec2(0.5f); vtx_end->col = col; vtx_end++;

					*idx_end = vtx_cnt + 0; idx_end++;
					*idx_end = vtx_cnt + 1; idx_end++;
					*idx_end = vtx_cnt + 2; idx_end++;

					vtx_cnt += 3;
					idx_cnt += 3;
				}
			}

			inline void add_text(int font_index, const Vec2 &pos, const Bvec4 &col, const wchar_t *text, float scale)
			{
				if (text[0] == 0 || font_index >= fonts.size())
					return;

				const auto& f = fonts[font_index];
				auto font = std::get<0>(f);
				auto pixel_height = font->pixel_height();
				auto sdf = std::get<1>(f);
				if (!sdf)
					scale = 1.f;

				auto _pos = Vec2(Ivec2(pos));

				start_cmd(sdf ? DrawCmdTextSdf : DrawCmdTextLcd, std::get<2>(f));
				auto &vtx_cnt = draw_cmds.back().vtx_cnt;
				auto &idx_cnt = draw_cmds.back().idx_cnt;

				auto s = text;
				while (*s != 0)
				{
					if (*s == '\n')
					{
						_pos.y += pixel_height;
						_pos.x = pos.x;
					}
					else
					{
						auto g = font->get_glyph(*s);
						auto size = Vec2(g->size) * scale;

						auto p = _pos + Vec2(g->off) * scale;
						vtx_end->pos = p;						  vtx_end->uv = g->uv0;						vtx_end->col = col; vtx_end++;
						vtx_end->pos = p + Vec2(0.f, -size.y);	  vtx_end->uv = Vec2(g->uv0.x, g->uv1.y);   vtx_end->col = col; vtx_end++;
						vtx_end->pos = p + Vec2(size.x, -size.y); vtx_end->uv = g->uv1;						vtx_end->col = col; vtx_end++;
						vtx_end->pos = p + Vec2(size.x, 0.f);	  vtx_end->uv = Vec2(g->uv1.x, g->uv0.y);   vtx_end->col = col; vtx_end++;

						*idx_end = vtx_cnt + 0; idx_end++;
						*idx_end = vtx_cnt + 2; idx_end++;
						*idx_end = vtx_cnt + 1; idx_end++;
						*idx_end = vtx_cnt + 0; idx_end++;
						*idx_end = vtx_cnt + 3; idx_end++;
						*idx_end = vtx_cnt + 2; idx_end++;

						vtx_cnt += 4;
						idx_cnt += 6;

						_pos.x += g->advance * scale;
					}
					s++;
				}
			}

			inline void add_line(const Vec2 &p0, const Vec2 &p1, const Bvec4 &col, float thickness)
			{
				if (distance(p0, p1) < 0.5f)
					return;

				path_line_to(p0);
				path_line_to(p1);
				stroke(col, thickness, false);
				clear_path();
			}

			inline void add_triangle_filled(const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Bvec4 &col)
			{
				path_line_to(p0);
				path_line_to(p1);
				path_line_to(p2);
				fill(col);
				clear_path();
			}

			inline void add_rect(const Vec2 &pos, const Vec2 &size, const Bvec4 &col, float thickness, float round_radius = 0.f, int round_flags = Rect::SideNW | Rect::SideNE | Rect::SideSW | Rect::SideSE)
			{
				add_rect_col2(pos, size, col, col, thickness, round_radius, round_flags);
			}

			inline void add_rect_col2(const Vec2 &pos, const Vec2 &size, const Bvec4 &inner_col, const Bvec4 &outter_col, float thickness, float round_radius = 0.f, int round_flags = Rect::SideNW | Rect::SideNE | Rect::SideSW | Rect::SideSE)
			{
				path_rect(pos, size, round_radius, round_flags);
				stroke_col2(inner_col, outter_col, thickness, true);
				clear_path();
			}

			inline void add_rect_rotate(const Vec2 &pos, const Vec2 &size, const Bvec4 &col, float thickness, const Vec2 &rotate_center, float angle)
			{
				path_rect(pos, size, 0.f, 0);
				for (auto &p : points)
					p = rotate(p, rotate_center, angle * ANG_RAD);
				stroke(col, thickness, true);
				clear_path();
			}

			inline void add_rect_filled(const Vec2 &pos, const Vec2 &size, const Bvec4 &col, float round_radius = 0.f, int round_flags = 0)
			{
				path_rect(pos, size, round_radius, round_flags);
				fill(col);
				clear_path();
			}

			inline void add_circle(const Vec2 &center, float radius, const Bvec4 &col, float thickness)
			{
				path_arc_to(center, radius, 0, FLAME_ARRAYSIZE(circle_subdiv) - 1);
				stroke(col, thickness, true);
				clear_path();
			}

			inline void add_circle_filled(const Vec2 &center, float radius, const Bvec4 &col)
			{
				path_arc_to(center, radius, 0, FLAME_ARRAYSIZE(circle_subdiv) - 1);
				fill(col);
				clear_path();
			}

			inline void add_bezier(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4, const Bvec4 &col, float thickness)
			{
				path_bezier(p1, p2, p3, p4);
				stroke(col, thickness, false);
				clear_path();
			}

			inline void add_image(const Vec2 &pos, const Vec2 &size, int id, const Vec2 &uv0 = Vec2(0.f), const Vec2 &uv1 = Vec2(1.f), const Bvec4 &tint_col = Bvec4(255))
			{
				auto _pos = Vec2(Ivec2(pos));

				start_cmd(DrawCmdElement, id);
				auto &vtx_cnt = draw_cmds.back().vtx_cnt;
				auto &idx_cnt = draw_cmds.back().idx_cnt;

				vtx_end->pos = _pos;						vtx_end->uv = uv0;				  vtx_end->col = tint_col; vtx_end++;
				vtx_end->pos = _pos + Vec2(0.f, size.y);	vtx_end->uv = Vec2(uv0.x, uv1.y); vtx_end->col = tint_col; vtx_end++;
				vtx_end->pos = _pos + Vec2(size.x, size.y); vtx_end->uv = uv1;				  vtx_end->col = tint_col; vtx_end++;
				vtx_end->pos = _pos + Vec2(size.x, 0.f);	vtx_end->uv = Vec2(uv1.x, uv0.y); vtx_end->col = tint_col; vtx_end++;

				*idx_end = vtx_cnt + 0; idx_end++;
				*idx_end = vtx_cnt + 2; idx_end++;
				*idx_end = vtx_cnt + 1; idx_end++;
				*idx_end = vtx_cnt + 0; idx_end++;
				*idx_end = vtx_cnt + 3; idx_end++;
				*idx_end = vtx_cnt + 2; idx_end++;

				vtx_cnt += 4;
				idx_cnt += 6;
			}

			inline void add_image_stretch(const Vec2 &pos, const Vec2 &size, int id, const Vec4 &border, const Bvec4 &tint_col = Bvec4(255))
			{
				//auto image_size = share_data.image_views[id]->image()->size;

				//auto b_uv = Vec4(Vec2(border[0], border[1]) / image_size.x,
				//	Vec2(border[2], border[3]) / image_size.y);

				//// corners
				//add_image(pos, Vec2(border[0], border[2]), id, Vec2(0.f), Vec2(b_uv[0], b_uv[2])); // LT
				//add_image(pos + Vec2(0.f, size.y - border[3]), Vec2(border[0], border[3]), id, Vec2(0.f, 1.f - b_uv[3]), Vec2(b_uv[0], 1.f)); // LB
				//add_image(pos + Vec2(size.x - border[1], 0.f), Vec2(border[1], border[2]), id, Vec2(1.f - b_uv[1], 0.f), Vec2(1.f, b_uv[2])); // RT
				//add_image(pos + Vec2(size.x - border[1], size.y - border[3]), Vec2(border[1], border[3]), id, Vec2(1.f - b_uv[1], 1.f - b_uv[3]), Vec2(1.f)); // RB

				//// borders
				//add_image(pos + Vec2(0.f, border[2]), Vec2(border[0], size.y - border[2] - border[3]), id, Vec2(0.f, b_uv[2]), Vec2(b_uv[0], 1.f - b_uv[3])); // L
				//add_image(pos + Vec2(size.x - border[1], border[2]), Vec2(border[1], size.y - border[2] - border[3]), id, Vec2(1.f - b_uv[1], b_uv[2]), Vec2(1.f, 1.f - b_uv[3])); // R
				//add_image(pos + Vec2(border[0], 0.f), Vec2(size.x - border[0] - border[1], border[2]), id, Vec2(b_uv[0], 0.f), Vec2(1.f - b_uv[1], b_uv[2])); // T
				//add_image(pos + Vec2(border[0], size.y - border[3]), Vec2(size.x - border[0] - border[1], border[3]), id, Vec2(b_uv[0], 1.f - b_uv[3]), Vec2(1.f - b_uv[1], 1.f)); // B

				//add_image(pos + Vec2(border[0], border[2]), Vec2(size.x - border[0] - border[1], size.y - border[2] - border[3]), id, Vec2(b_uv[0], b_uv[2]), Vec2(1.f - b_uv[1], 1.f - b_uv[3]));
			}

			inline void set_scissor(const Rect &scissor)
			{
				draw_cmds.emplace_back(scissor);
			}

			inline void record_cb()
			{
				cb->begin();
				//cb->begin_renderpass(clear_col.w > 0 ? share_data.renderpass : share_data.renderpass_noclear, s->framebuffers[swacpchain_image_index], clear_col.w > 0 ? clear_values : nullptr);
				cb->begin_renderpass(sc->get_renderpass_clear(), sc->get_framebuffer(sc->get_avalible_image_index()), cv);
				if (idx_end != idx_buffer->mapped)
				{
					auto surface_size = Vec2(sc->window()->size);

					cb->set_viewport(Rect(Vec2(0.f), surface_size));
					cb->set_scissor(Rect(Vec2(0.f), surface_size));
					cb->bind_vertexbuffer(vtx_buffer, 0);
					cb->bind_indexbuffer(idx_buffer, IndiceTypeUint);

					auto pc = Vec4(2.f / surface_size.x, 2.f / surface_size.y, 
						4.f / /*share_data.font_sdf_image->size*/1.f, 4.f / /*share_data.font_sdf_image->size*/1.f);

					cb->push_constant(0, sizeof(Vec4), &pc, pl_element->layout());

					auto vtx_off = 0;
					auto idx_off = 0;
					for (auto &dc : draw_cmds)
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
				idx_end = (int*)idx_buffer->mapped;
				draw_cmds.clear();
			}
		};

		int Canvas::add_font(Font* font)
		{
			return ((CanvasPrivate*)this)->add_font(font);
		}

		void Canvas::start_cmd(DrawCmdType type, int id)
		{
			((CanvasPrivate*)this)->start_cmd(type, id);
		}

		void Canvas::path_line_to(const Vec2 &p)
		{
			((CanvasPrivate*)this)->path_line_to(p);
		}

		void Canvas::path_rect(const Vec2 &pos, const Vec2 &size, float round_radius, int round_flags)
		{
			((CanvasPrivate*)this)->path_rect(pos, size, round_radius, round_flags);
		}

		void Canvas::path_arc_to(const Vec2 &center, float radius, int a_min, int a_max)
		{
			((CanvasPrivate*)this)->path_arc_to(center, radius, a_min, a_max);
		}

		void Canvas::path_bezier(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4, int level)
		{
			((CanvasPrivate*)this)->path_bezier(p1, p2, p3, p4, level);
		}

		void Canvas::clear_path()
		{
			((CanvasPrivate*)this)->clear_path();
		}

		void Canvas::stroke(const Bvec4 &col, float thickness, bool closed)
		{
			((CanvasPrivate*)this)->stroke(col, thickness, closed);
		}

		void Canvas::stroke_col2(const Bvec4 &inner_col, const Bvec4 &outter_col, float thickness, bool closed)
		{
			((CanvasPrivate*)this)->stroke_col2(inner_col, outter_col, thickness, closed);
		}

		void Canvas::fill(const Bvec4 &col)
		{
			((CanvasPrivate*)this)->fill(col);
		}

		void Canvas::add_text(int font_index, const Vec2 &pos, const Bvec4 &col, const wchar_t *text, float scale)
		{
			((CanvasPrivate*)this)->add_text(font_index, pos, col, text, scale);
		}

		void Canvas::add_line(const Vec2 &p0, const Vec2 &p1, const Bvec4 &col, float thickness)
		{
			((CanvasPrivate*)this)->add_line(p0, p1, col, thickness);
		}

		void Canvas::add_triangle_filled(const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Bvec4 &col)
		{
			((CanvasPrivate*)this)->add_triangle_filled(p0, p1, p2, col);
		}

		void Canvas::add_rect(const Vec2 &pos, const Vec2 &size, const Bvec4 &col, float thickness, float round_radius, int round_flags)
		{
			((CanvasPrivate*)this)->add_rect(pos, size, col, thickness, round_radius, round_flags);
		}

		void Canvas::add_rect_col2(const Vec2 &pos, const Vec2 &size, const Bvec4 &inner_col, const Bvec4 &outter_col, float thickness, float round_radius, int round_flags)
		{
			((CanvasPrivate*)this)->add_rect_col2(pos, size, inner_col, outter_col, thickness, round_radius, round_flags);
		}

		void Canvas::add_rect_rotate(const Vec2 &pos, const Vec2 &size, const Bvec4 &col, float thickness, const Vec2 &rotate_center, float angle)
		{
			((CanvasPrivate*)this)->add_rect_rotate(pos, size, col, thickness, rotate_center, angle);
		}

		void Canvas::add_rect_filled(const Vec2 &pos, const Vec2 &size, const Bvec4 &col, float round_radius, int round_flags)
		{
			((CanvasPrivate*)this)->add_rect_filled(pos, size, col, round_radius, round_flags);
		}

		void Canvas::add_circle(const Vec2 &center, float radius, const Bvec4 &col, float thickness)
		{
			((CanvasPrivate*)this)->add_circle(center, radius, col, thickness);
		}

		void Canvas::add_circle_filled(const Vec2 &center, float radius, const Bvec4 &col)
		{
			((CanvasPrivate*)this)->add_circle_filled(center, radius, col);
		}

		void Canvas::add_bezier(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4, const Bvec4 &col, float thickness)
		{
			((CanvasPrivate*)this)->add_bezier(p1, p2, p3, p4, col, thickness);
		}

		void Canvas::add_image(const Vec2 &pos, const Vec2 &size, int id, const Vec2 &uv0, const Vec2 &uv1, const Bvec4 &tint_col)
		{
			((CanvasPrivate*)this)->add_image(pos, size, id, uv0, uv1, tint_col);
		}

		void Canvas::add_image_stretch(const Vec2 &pos, const Vec2 &size, int id, const Vec4 &border, const Bvec4 &tint_col)
		{
			((CanvasPrivate*)this)->add_image_stretch(pos, size, id, border, tint_col);
		}

		void Canvas::set_scissor(const Rect &scissor)
		{
			((CanvasPrivate*)this)->set_scissor(scissor);
		}

		Commandbuffer *Canvas::get_cb() const
		{
			return ((CanvasPrivate*)this)->cb;
		}

		void Canvas::record_cb()
		{
			((CanvasPrivate*)this)->record_cb();
		}

		void Canvas::initialize(Device* d, Swapchain* sc)
		{
			device = d;

			white_image = Image::create(device, Format_R8G8B8A8_UNORM, Ivec2(4), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst, MemPropDevice);
			white_image->init(Bvec4(255));
			white_imageview = Imageview::get(white_image);

			auto vib = VertexInputBufferInfo({
					Format_R32G32_SFLOAT,
					Format_R32G32_SFLOAT,
					Format_R8G8B8A8_UNORM });

			auto sample_count = sc->get_sample_count();
			auto renderpass = sc->get_renderpass_clear();

			GraphicsPipelineInfo pl_element_info;
			pl_element_info.shaders.resize(2);
			pl_element_info.shaders[0].filename = L"2d/element.vert";
			pl_element_info.shaders[1].filename = L"2d/element.frag";
			pl_element_info.vi_buffers.push_back(vib);
			pl_element_info.cull_mode = CullModeNone;
			pl_element_info.sample_count = sample_count;
			pl_element_info.blend_states[0] = BlendInfo(
				BlendFactorSrcAlpha, BlendFactorOneMinusSrcAlpha,
				BlendFactorZero, BlendFactorOneMinusSrcAlpha);
			pl_element_info.renderpass = renderpass;
			pl_element = Pipeline::create(device, pl_element_info);

			graphics::GraphicsPipelineInfo pl_info_text_lcd;
			pl_info_text_lcd.shaders.resize(2);
			pl_info_text_lcd.shaders[0].filename = L"2d/element.vert";
			pl_info_text_lcd.shaders[1].filename = L"2d/text_lcd.frag";
			pl_info_text_lcd.vi_buffers.push_back(vib);
			pl_info_text_lcd.cull_mode = graphics::CullModeNone;
			pl_info_text_lcd.sample_count = sample_count;
			pl_info_text_lcd.blend_states[0] = graphics::BlendInfo(
				graphics::BlendFactorSrc1Color, graphics::BlendFactorOneMinusSrc1Color,
				graphics::BlendFactorZero, graphics::BlendFactorZero);
			pl_info_text_lcd.renderpass = renderpass;
			pl_text_lcd = graphics::Pipeline::create(d, pl_info_text_lcd);

			graphics::GraphicsPipelineInfo pl_info_text_sdf;
			pl_info_text_sdf.shaders.resize(2);
			pl_info_text_sdf.shaders[0].filename = L"2d/element.vert";
			pl_info_text_sdf.shaders[1].filename = L"2d/text_sdf.frag";
			pl_info_text_sdf.vi_buffers.push_back(vib);
			pl_info_text_sdf.cull_mode = graphics::CullModeNone;
			pl_info_text_sdf.sample_count = sample_count;
			pl_info_text_sdf.blend_states[0] = graphics::BlendInfo(
				graphics::BlendFactorSrcAlpha, graphics::BlendFactorOneMinusSrcAlpha,
				graphics::BlendFactorZero, graphics::BlendFactorOneMinusSrcAlpha);
			pl_info_text_sdf.renderpass = renderpass;
			pl_text_sdf = graphics::Pipeline::create(d, pl_info_text_sdf);

			for (auto i = 0; i < FLAME_ARRAYSIZE(circle_subdiv); i++)
			{
				auto rad = ANG_RAD * ((360.f / FLAME_ARRAYSIZE(circle_subdiv)) * i);
				circle_subdiv[i].y = sin(rad);
				circle_subdiv[i].x = cos(rad);
			}
		}

		void Canvas::deinitialize()
		{

		}

		Canvas *Canvas::create(Swapchain *sc)
		{
			return new CanvasPrivate(sc);
		}

		void Canvas::destroy(Canvas *c)
		{
			delete (CanvasPrivate*)c;
		}
	}
}
