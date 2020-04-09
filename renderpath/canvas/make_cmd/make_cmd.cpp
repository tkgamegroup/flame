#include <flame/foundation/blueprint.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/commandbuffer.h>

#include "../canvas.h"

using namespace flame;
using namespace graphics;

enum CmdType
{
	CmdDrawElement,
	CmdSetScissor
};

struct Cmd
{
	CmdType type;
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

struct CanvasPrivate : Canvas
{
	void* mc;

	void set_clear_color(const Vec4c& col) override;
	Imageview* get_image(uint index) override;
	Atlas* get_atlas(uint index) override;
	uint set_image(int index, Imageview* v, Filter filter, Atlas* atlas) override;

	void stroke(uint point_count, const Vec2f* points, const Vec4c& col, float thickness) override;
	void fill(uint point_count, const Vec2f* points, const Vec4c& col) override;

	void add_text(FontAtlas* f, const wchar_t* text_begin, const wchar_t* text_end, uint font_size, const Vec2f& pos, const Vec4c& col) override;
	void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col) override;
	const Vec4f& scissor() override;
	void set_scissor(const Vec4f& scissor) override;

	void set_draw(void(*_draw)(void* c), const Mail& _capture) override;
};

struct Img
{
	Imageview* view;
	Vec2f white_uv;
	Atlas* atlas;
};

struct Vertex
{
	Vec2f pos;
	Vec2f uv;
	Vec4c col;
};

struct FLAME_R(MakeCmd)
{
	BP::Node* n;

	FLAME_B0;
	FLAME_RV(Array<Commandbuffer*>*, cbs, i);
	FLAME_RV(Buffer*, vtx_buf, i);
	FLAME_RV(Buffer*, idx_buf, i);
	FLAME_RV(RenderpassAndFramebuffer*, rnf, i);
	FLAME_RV(uint, image_idx, i);
	FLAME_RV(Pipelinelayout*, pll, i);
	FLAME_RV(Pipeline*, pl, i);
	FLAME_RV(Descriptorset*, ds, i);
	FLAME_RV(Imageview*, white_iv, i);

	FLAME_B1;
	FLAME_RV(Canvas*, canvas, o);

	Vec4c clear_color;

	std::vector<Img> imgs;

	Vertex* vtx_end;
	uint* idx_end;

	Vec2f surface_size;
	Vec4f curr_scissor;

	void(*draw)(void*);
	Mail capture;

	std::vector<Cmd> cmds;

	int frame;

	__declspec(dllexport) RF(MakeCmd)() :
		draw(nullptr),
		frame(-1)
	{
	}

	__declspec(dllexport) FLAME_RF(~MakeCmd)()
	{
		delete (CanvasPrivate*)canvas;
		f_free(capture.p);
	}

	uint set_image(int index, Imageview * v, Filter filter, Atlas * atlas)
	{
		if (imgs.empty())
			return -1;
		if (index == -1)
		{
			assert(v);
			for (auto i = 0; i < imgs.size(); i++)
			{
				if (imgs[i].view == white_iv)
				{
					index = i;
					break;
				}
			}
			assert(index != -1);
		}
		Vec2f white_uv;
		if (!v)
		{
			v = white_iv;
			white_uv = 0.5f;
			atlas = nullptr;
		}
		else
		{
			auto img = v->image();
			img->set_pixels(img->size - 1U, Vec2u(1), &Vec4c(255));
			white_uv = (Vec2f(img->size - 1U) + 0.5f) / Vec2f(img->size);
		}
		ds->set_image(0, index, v, filter);
		imgs[index] = { v, white_uv, atlas };
		return index;
	}

	void set_scissor(const Vec4f & _scissor)
	{
		auto scissor = Vec4f(
			max(_scissor.x(), 0.f),
			max(_scissor.y(), 0.f),
			min(_scissor.z(), surface_size.x()),
			min(_scissor.w(), surface_size.y()));
		if (scissor == curr_scissor)
			return;
		curr_scissor = scissor;
		Cmd cmd;
		cmd.type = CmdSetScissor;
		cmd.v.scissor = scissor;
		cmds.push_back(cmd);
	}

	void stroke(uint point_count, const Vec2f* points, const Vec4c& col, float thickness)
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
		auto uv = imgs[cmds.back().v.draw_data.id].white_uv;

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

				auto n0 = normals[i] * thickness * 0.5f;
				auto n1 = normals[i + 1] * thickness * 0.5f;

				vtx_end->pos = p0 + n0; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;
				vtx_end->pos = p0 - n0; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;
				vtx_end->pos = p1 - n1; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;
				vtx_end->pos = p1 + n1; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;

				*idx_end = vtx_cnt + 0; idx_end++;
				*idx_end = vtx_cnt + 2; idx_end++;
				*idx_end = vtx_cnt + 1; idx_end++;
				*idx_end = vtx_cnt + 0; idx_end++;
				*idx_end = vtx_cnt + 3; idx_end++;
				*idx_end = vtx_cnt + 2; idx_end++;

				vtx_cnt += 4;
				idx_cnt += 6;
			}
			else if (!(closed && i + 1 == point_count - 1))
			{
				auto p1 = points[i + 1];

				auto n1 = normals[i + 1] * thickness * 0.5f;

				vtx_end->pos = p1 - n1; vtx_end->uv = uv; vtx_end->col = col;  vtx_end++;
				vtx_end->pos = p1 + n1; vtx_end->uv = uv; vtx_end->col = col; vtx_end++;

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

	void fill(uint point_count, const Vec2f* points, const Vec4c& col)
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
		auto uv = imgs[cmds.back().v.draw_data.id].white_uv;

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

	void add_text(FontAtlas* f, const wchar_t* text_begin, const wchar_t* text_end, uint font_size, const Vec2f& _pos, const Vec4c& col)
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

	void add_image(const Vec2f& _pos, const Vec2f& size, uint id, const Vec2f& _uv0, const Vec2f& _uv1, const Vec4c& tint_col)
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
		auto& img = imgs[img_id];
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

	__declspec(dllexport) void FLAME_RF(update)(uint _frame)
	{
		vtx_end = vtx_buf ? (Vertex*)vtx_buf->mapped : nullptr;
		idx_end = idx_buf ? (uint*)idx_buf->mapped : nullptr;

		if (frame == -1)
		{
			clear_color = Vec4c(0, 0, 0, 255);

			auto ds_layout = ds->layout();
			imgs.resize(ds_layout->binding_count() ? ds_layout->get_binding(0).count : 0, { white_iv, Vec2f(0.5f), nullptr });

			auto c = new CanvasPrivate;
			c->scene = n->scene();
			c->mc = this;
			canvas = c;
			canvas_s()->set_frame(_frame);

			frame = 0;
		}
		else
		{
			if (cbs && cbs->s > 0)
			{
				auto cb = cbs->at(image_idx);
				if (rnf && pl)
				{
					surface_size = Vec2f(rnf->framebuffer(image_idx)->image_size);
					curr_scissor = Vec4f(Vec2f(0.f), surface_size);

					if (draw)
						draw(capture.p);

					cb->begin();
					auto cv = rnf->clearvalues();
					cv->set(0, clear_color);
					cb->begin_renderpass(rnf->framebuffer(image_idx), cv);
					if (idx_end != idx_buf->mapped)
					{
						cb->set_viewport(curr_scissor);
						cb->set_scissor(curr_scissor);
						cb->bind_vertexbuffer(vtx_buf, 0);
						cb->bind_indexbuffer(idx_buf, IndiceTypeUint);

						struct
						{
							Vec2f scale;
						}pc;
						pc.scale = Vec2f(2.f / surface_size.x(), 2.f / surface_size.y());
						cb->bind_pipeline(pl);
						cb->push_constant(0, sizeof(pc), &pc, pll);
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
				}
				else
				{
					cb->begin();
					cb->end();
				}
			}

			cmds.clear();
		}
	}
};

void CanvasPrivate::set_clear_color(const Vec4c& col)
{
	((MakeCmd*)mc)->clear_color = col;
}

Imageview* CanvasPrivate::get_image(uint index)
{
	return ((MakeCmd*)mc)->imgs[index].view;
}

Atlas* CanvasPrivate::get_atlas(uint index)
{
	return ((MakeCmd*)mc)->imgs[index].atlas;
}

uint CanvasPrivate::set_image(int index, Imageview* v, Filter filter, Atlas* atlas)
{
	return ((MakeCmd*)mc)->set_image(index, v, filter, atlas);
}

void CanvasPrivate::stroke(uint point_count, const Vec2f* points, const Vec4c& col, float thickness)
{
	((MakeCmd*)mc)->stroke(point_count, points, col, thickness);
}

void CanvasPrivate::fill(uint point_count, const Vec2f* points, const Vec4c& col)
{
	((MakeCmd*)mc)->fill(point_count, points, col);
}

void CanvasPrivate::add_text(FontAtlas* f, const wchar_t* text_begin, const wchar_t* text_end, uint font_size, const Vec2f& pos, const Vec4c& col)
{
	((MakeCmd*)mc)->add_text(f, text_begin, text_end, font_size, pos, col);
}

void CanvasPrivate::add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col)
{
	((MakeCmd*)mc)->add_image(pos, size, id, uv0, uv1, tint_col);
}

const Vec4f& CanvasPrivate::scissor()
{
	return ((MakeCmd*)mc)->curr_scissor;
}

void CanvasPrivate::set_scissor(const Vec4f& scissor)
{
	((MakeCmd*)mc)->set_scissor(scissor);
}

void CanvasPrivate::set_draw(void(*draw)(void* c), const Mail& capture)
{
	auto thiz = ((MakeCmd*)mc);
	thiz->draw = draw;
	thiz->capture = capture;
}

