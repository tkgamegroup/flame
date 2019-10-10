#include <flame/foundation/foundation.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/all.h>

#include "../canvas/type.h"
#include "canvas.h"

using namespace flame;
using namespace graphics;

namespace flame
{
	struct Vertex
	{
		Vec2f pos;
		Vec2f uv;
		Vec4c col;
	};

	enum CmdType
	{
		CmdDrawElement,
		CmdDrawTextLcd,
		CmdDrawTextSdf,
		CmdSetScissor,

		CmdDrawRect
	};

	struct CmdBase
	{
		CmdType type;
	};

	struct Cmd : CmdBase
	{
		union
		{
			struct
			{
				uint id;
				uint vtx_cnt;
				uint idx_cnt;
			}draw_data;
			Vec4f scissor;
		}v;
	};

	struct Rect : CmdBase
	{
		Vec2f pos;
		Vec2f size;
		Vec4c color;
	};

	struct Rect$
	{
		AttributeV<Vec2f> pos$i;
		AttributeV<Vec2f> size$i;
		AttributeV<Vec4c> color$i;

		AttributeV<Rect> out$o;

		__declspec(dllexport) void update$()
		{
			out$o.v.type = CmdDrawRect;
			if (pos$i.frame > out$o.frame)
				out$o.v.pos = pos$i.v;
			if (size$i.frame > out$o.frame)
				out$o.v.size = size$i.v;
			if (color$i.frame > out$o.frame)
				out$o.v.color = color$i.v;
			out$o.frame = maxN(pos$i.frame, size$i.frame, color$i.frame);
		}
	};

	struct MakeCmd$;

	struct CanvasPrivate : Canvas
	{
		MakeCmd$* thiz;

		virtual Imageview* get_image(uint index) override;
		virtual uint set_image(int index, Imageview* v, Filter filter) override;

		virtual void stroke(const std::vector<Vec2f>& points, const Vec4c& inner_col, const Vec4c& outter_col, float thickness) override;
		virtual void fill(const std::vector<Vec2f>& points, const Vec4c& col) override;

		virtual Vec2f add_text(FontAtlas* f, const Vec2f& pos, const Vec4c& col, const std::wstring& text, float scale) override;
		virtual void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col) override;
		virtual void set_scissor(const Vec4f& scissor) override;
	};

	struct MakeCmd$
	{
		AttributeP<std::vector<void*>> cbs$i;
		AttributeP<void> vtx_buf$i;
		AttributeP<void> idx_buf$i;
		AttributeP<void> rnf$i;
		AttributeV<uint> image_idx$i;
		AttributeP<void> pll$i;
		AttributeP<void> pl_element$i;
		AttributeP<void> pl_text_lcd$i;
		AttributeP<void> pl_text_sdf$i;
		AttributeP<void> ds$i;
		AttributeP<void> white_iv$i;

		AttributeP<void> canvas$o;

		AttributeP<std::vector<void*>> content$i;

		std::vector<Imageview*> ivs;

		Vertex* vtx_end;
		uint* idx_end;

		std::vector<Cmd> cmds;
		Vec2f surface_size;
		Vec4f curr_scissor;

		int frame;

		__declspec(dllexport) MakeCmd$() :
			frame(-1)
		{
		}

		uint set_image(int index, Imageview* v, Filter filter)
		{
			if (index == -1)
			{
				assert(v);
				for (auto i = 1; i < ivs.size(); i++)
				{
					if (ivs[i] == (Imageview*)white_iv$i.v)
					{
						index = i;
						break;
					}
				}
				assert(index != -1);
			}
			if (!v)
				v = (Imageview*)white_iv$i.v;
			((Descriptorset*)ds$i.v)->set_image(0, index, v, filter);
			ivs[index] = v;
			return index;
		}

		void set_scissor(const Vec4f& _scissor)
		{
			auto scissor = _scissor;
			scissor.x() = max(scissor.x(), 0.f);
			scissor.y() = max(scissor.y(), 0.f);
			scissor.z() = min(scissor.z(), surface_size.x());
			scissor.w() = min(scissor.w(), surface_size.y());
			if (scissor == curr_scissor)
				return;
			curr_scissor = scissor;
			Cmd cmd;
			cmd.type = CmdSetScissor;
			cmd.v.scissor = scissor;
			cmds.push_back(cmd);
		}

		void begin_draw(CmdType type, uint id)
		{
			if (!cmds.empty())
			{
				auto& last = cmds.back();
				if (last.type == type && last.v.draw_data.id == id)
					return;
			}
			Cmd cmd;
			cmd.type = type;
			cmd.v.draw_data.id = id;
			cmd.v.draw_data.vtx_cnt = 0;
			cmd.v.draw_data.idx_cnt = 0;
			cmds.push_back(cmd);
		}

		void stroke(const std::vector<Vec2f>& points, const Vec4c& inner_col, const Vec4c& outter_col, float thickness)
		{
			if (points.size() < 2)
				return;

			begin_draw(CmdDrawElement, 0);
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;
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

			begin_draw(CmdDrawElement, 0);
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;

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
			auto lh = f->pixel_height * scale;

			auto _pos = Vec2f(Vec2i(pos));

			begin_draw((CmdType)f->draw_type, f->index);
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;
			Vec2f rect(0.f, f->pixel_height);
			auto lw = 0.f;
			for (auto ch : text)
			{
				if (ch == '\n')
				{
					_pos.y() += lh;
					_pos.x() = pos.x();

					rect.y() += f->pixel_height;
					lw = 0.f;
				}
				else if (ch != '\r')
				{
					if (ch == '\t')
						ch = ' ';
					auto g = f->get_glyph(ch);

					auto p = _pos + Vec2f(g->off) * scale;
					auto size = Vec2f(g->size) * scale;
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

					_pos.x() += g->advance * scale;

					lw += g->advance;
					if (lw > rect.x())
						rect.x() = lw;
				}
			}

			return rect;
		}

		void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col)
		{
			auto _pos = Vec2f(Vec2i(pos));

			begin_draw(CmdDrawElement, id);
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;

			vtx_end->pos = _pos;								vtx_end->uv = uv0;						vtx_end->col = tint_col; vtx_end++;
			vtx_end->pos = _pos + Vec2f(0.f, size.y());			vtx_end->uv = Vec2f(uv0.x(), uv1.y());	vtx_end->col = tint_col; vtx_end++;
			vtx_end->pos = _pos + Vec2f(size.x(), size.y());	vtx_end->uv = uv1;						vtx_end->col = tint_col; vtx_end++;
			vtx_end->pos = _pos + Vec2f(size.x(), 0.f);			vtx_end->uv = Vec2f(uv1.x(), uv0.y());	vtx_end->col = tint_col; vtx_end++;

			*idx_end = vtx_cnt + 0; idx_end++;
			*idx_end = vtx_cnt + 2; idx_end++;
			*idx_end = vtx_cnt + 1; idx_end++;
			*idx_end = vtx_cnt + 0; idx_end++;
			*idx_end = vtx_cnt + 3; idx_end++;
			*idx_end = vtx_cnt + 2; idx_end++;

			vtx_cnt += 4;
			idx_cnt += 6;
		}

		__declspec(dllexport) void update$()
		{
			auto rnf = (RenderpassAndFramebuffer*)rnf$i.v;
			auto img_idx = image_idx$i.v;
			auto cb = (Commandbuffer*)(*cbs$i.v)[img_idx];
			auto vtx_buf = (Buffer*)vtx_buf$i.v;
			auto idx_buf = (Buffer*)idx_buf$i.v;
			auto pll = (Pipelinelayout*)pll$i.v;
			auto pl_element = (Pipeline*)pl_element$i.v;
			auto pl_text_lcd = (Pipeline*)pl_text_lcd$i.v;
			auto pl_text_sdf = (Pipeline*)pl_text_sdf$i.v;
			auto ds = (Descriptorset*)ds$i.v;

			if (frame == -1)
			{
				vtx_end = (Vertex*)vtx_buf->mapped;
				idx_end = (uint*)idx_buf->mapped;

				auto c = new CanvasPrivate;
				c->thiz = this;
				canvas$o.v = c;
				canvas$o.frame = looper().frame;

				frame = 0;
			}

			if (rnf)
			{
				auto fb = (Framebuffer*)rnf->framebuffers()[img_idx];
				surface_size = Vec2f(fb->image_size);

				auto content = get_attribute_vec(content$i);
				for (auto& _c : content)
				{
					switch (((CmdBase*)_c)->type)
					{
					case CmdDrawRect:
					{
						auto c = (Rect*)_c;
						std::vector<Vec2f> points;
						path_rect(points, c->pos, c->size);
						fill(points, c->color);
					}
						break;
					}
				}

				cb->begin();
				cb->begin_renderpass(fb, rnf->clearvalues());
				if (idx_end != idx_buf->mapped)
				{
					curr_scissor = Vec4f(Vec2f(0.f), surface_size);
					cb->set_viewport(curr_scissor);
					cb->set_scissor(curr_scissor);
					cb->bind_vertexbuffer(vtx_buf, 0);
					cb->bind_indexbuffer(idx_buf, IndiceTypeUint);

					PushconstantT$ pc;
					pc.scale$ = Vec2f(2.f / surface_size.x(), 2.f / surface_size.y());
					pc.sdf_range$ = Vec2f(4.f / 512.f); /* sdf_image->size */
					cb->push_constant(0, sizeof(PushconstantT$), &pc, pll);
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
								cb->bind_pipeline(pl_element);
								cb->draw_indexed(cmd.v.draw_data.idx_cnt, idx_off, vtx_off, 1, cmd.v.draw_data.id);
								vtx_off += cmd.v.draw_data.vtx_cnt;
								idx_off += cmd.v.draw_data.idx_cnt;
							}
							break;
						case CmdDrawTextLcd:
							if (cmd.v.draw_data.idx_cnt > 0)
							{
								cb->bind_pipeline(pl_text_lcd);
								cb->draw_indexed(cmd.v.draw_data.idx_cnt, idx_off, vtx_off, 1, cmd.v.draw_data.id);
								vtx_off += cmd.v.draw_data.vtx_cnt;
								idx_off += cmd.v.draw_data.idx_cnt;
							}
							break;
						case CmdDrawTextSdf:
							if (cmd.v.draw_data.idx_cnt > 0)
							{
								cb->bind_pipeline(pl_text_sdf);
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

				vtx_end = (Vertex*)vtx_buf->mapped;
				idx_end = (uint*)idx_buf->mapped;
				cmds.clear();
			}
			else
			{
				cb->begin();
				cb->end();
			}
		}
	};

	Imageview* CanvasPrivate::get_image(uint index)
	{
		return thiz->ivs[index];
	}

	uint CanvasPrivate::set_image(int index, Imageview* v, Filter filter)
	{
		return thiz->set_image(index, v, filter);
	}

	void CanvasPrivate::stroke(const std::vector<Vec2f>& points, const Vec4c& inner_col, const Vec4c& outter_col, float thickness)
	{
		thiz->stroke(points, inner_col, outter_col, thickness);
	}

	void CanvasPrivate::fill(const std::vector<Vec2f>& points, const Vec4c& col)
	{
		thiz->fill(points, col);
	}

	Vec2f CanvasPrivate::add_text(FontAtlas* f, const Vec2f& pos, const Vec4c& col, const std::wstring& text, float scale)
	{
		return thiz->add_text(f, pos, col, text, scale);
	}

	void CanvasPrivate::add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col)
	{
		thiz->add_image(pos, size, id, uv0, uv1, tint_col);
	}

	void CanvasPrivate::set_scissor(const Vec4f& scissor)
	{
		thiz->set_scissor(scissor);
	}
}

extern "C" __declspec(dllexport) bool package_check_update(BP::Package * p)
{
	return true;
}
