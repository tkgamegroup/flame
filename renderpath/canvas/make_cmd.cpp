#include <flame/foundation/foundation.h>
#include <flame/foundation/bitmap.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/all.h>

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
		AttributeD<Vec2f> pos$i;
		AttributeD<Vec2f> size$i;
		AttributeD<Vec4c> color$i;
		AttributeD<float> alpha$i;

		AttributeD<Rect> out$o;

		__declspec(dllexport) Rect$()
		{
			alpha$i.v = 1.f;
		}

		__declspec(dllexport) void update$()
		{
			out$o.v.type = CmdDrawRect;
			if (pos$i.frame > out$o.frame)
				out$o.v.pos = pos$i.v;
			if (size$i.frame > out$o.frame)
				out$o.v.size = size$i.v;
			if (color$i.frame > out$o.frame || alpha$i.frame > out$o.frame)
				out$o.v.color = alpha_mul(color$i.v, alpha$i.v);
			out$o.frame = maxN(pos$i.frame, size$i.frame, color$i.frame, alpha$i.frame);
		}
	};

	struct TextSdf : CmdBase
	{
		FontAtlas* font_atals;
		float scale;
		Vec2f pos;
		Vec4c color;
		std::vector<Glyph*> glyphs;
	};

	struct TextSdf$
	{
		AttributeP<void> font_atals$i;
		AttributeD<float> scale$i;
		AttributeD<Vec2f> pos$i;
		AttributeD<Vec4c> color$i;
		AttributeD<float> alpha$i;
		AttributeD<std::wstring> text$i;

		AttributeD<TextSdf> out$o;

		__declspec(dllexport) TextSdf$()
		{
			scale$i.v = 1.f;
			alpha$i.v = 1.f;

			text$i.v = L"Replace Me!";
		}

		__declspec(dllexport) void update$()
		{
			out$o.v.type = CmdDrawTextSdf;
			if (font_atals$i.frame > out$o.frame)
				out$o.v.font_atals = (FontAtlas*)font_atals$i.v;
			if (scale$i.frame > out$o.frame)
				out$o.v.scale = scale$i.v;
			if (pos$i.frame > out$o.frame)
				out$o.v.pos = pos$i.v;
			if (color$i.frame > out$o.frame || alpha$i.frame > out$o.frame)
				out$o.v.color = alpha_mul(color$i.v, alpha$i.v);
			if (text$i.frame > out$o.frame)
			{
				out$o.v.glyphs.resize(text$i.v.size());
				auto atlas = (FontAtlas*)font_atals$i.v;
				for (auto i = 0; i < text$i.v.size(); i++)
					out$o.v.glyphs[i] = atlas->get_glyph(text$i.v[i], 0);
			}
			out$o.frame = maxN(font_atals$i.frame, scale$i.frame, pos$i.frame, color$i.frame, alpha$i.frame, text$i.frame);
		}
	};

	struct MakeCmd$;

	struct CanvasPrivate : Canvas
	{
		MakeCmd$* thiz;

		void set_clear_color(const Vec4c& col) override;
		Imageview* get_image(uint index) override;
		Atlas* get_atlas(uint index) override;
		uint set_image(int index, Imageview* v, Filter filter, Atlas* atlas) override;

		void stroke(const std::vector<Vec2f>& points, const Vec4c& col, float thickness) override;
		void fill(const std::vector<Vec2f>& points, const Vec4c& col) override;

		void add_text(FontAtlas* f, const std::vector<Glyph*> glyphs, uint line_space, float scale, const Vec2f& pos, const Vec4c& col) override;
		void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col, bool repeat) override;
		const Vec4f& scissor() override;
		void set_scissor(const Vec4f& scissor) override;
	};

	struct MakeCmd$
	{
		AttributeP<std::vector<void*>> cbs$i;
		AttributeP<void> vtx_buf$i;
		AttributeP<void> idx_buf$i;
		AttributeP<void> rnf$i;
		AttributeD<uint> image_idx$i;
		AttributeP<void> pll$i;
		AttributeP<void> pl_element$i;
		AttributeP<void> pl_text_lcd$i;
		AttributeP<void> pl_text_sdf$i;
		AttributeP<void> ds$i;
		AttributeP<void> white_iv$i;

		AttributeP<void> canvas$o;

		AttributeP<std::vector<void*>> font_atlases$i;
		AttributeP<std::vector<void*>> content$i;

		Vec4c clear_color;

		std::vector<std::pair<Imageview*, Atlas*>> imgs;

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

		__declspec(dllexport) ~MakeCmd$()
		{
			delete (CanvasPrivate*)canvas$o.v;
		}

		uint set_image(int index, Imageview* v, Filter filter, Atlas* atlas)
		{
			if (index == -1)
			{
				assert(v);
				for (auto i = 1; i < imgs.size(); i++)
				{
					if (imgs[i].first == (Imageview*)white_iv$i.v)
					{
						index = i;
						break;
					}
				}
				assert(index != -1);
			}
			if (!v)
			{
				v = (Imageview*)white_iv$i.v;
				atlas = nullptr;
			}
			((Descriptorset*)ds$i.v)->set_image(0, index, v, filter);
			imgs[index] = std::make_pair(v, atlas);
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

		void stroke(const std::vector<Vec2f>& points, const Vec4c& col, float thickness)
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

					vtx_end->pos = p0 + n0; vtx_end->uv = Vec2f(0.5f); vtx_end->col = col; vtx_end++;
					vtx_end->pos = p0 - n0; vtx_end->uv = Vec2f(0.5f); vtx_end->col = col;  vtx_end++;
					vtx_end->pos = p1 - n1; vtx_end->uv = Vec2f(0.5f); vtx_end->col = col;  vtx_end++;
					vtx_end->pos = p1 + n1; vtx_end->uv = Vec2f(0.5f); vtx_end->col = col; vtx_end++;

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

					vtx_end->pos = p1 - n1; vtx_end->uv = Vec2f(0.5f); vtx_end->col = col;  vtx_end++;
					vtx_end->pos = p1 + n1; vtx_end->uv = Vec2f(0.5f); vtx_end->col = col; vtx_end++;

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

		void add_text(FontAtlas* f, const std::vector<Glyph*> glyphs, uint line_space, float scale, const Vec2f& _pos, const Vec4c& col)
		{
			auto pos = Vec2f(Vec2i(_pos));

			begin_draw((CmdType)f->draw_type, f->canvas_slot_);
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;

			for (auto g : glyphs)
			{
				auto ch = g->unicode;
				if (ch == '\n')
				{
					pos.y() += line_space;
					pos.x() = (int)_pos.x();
				}
				else if (ch != '\r')
				{
					if (ch == '\t')
						ch = ' ';

					auto p = pos + Vec2f(g->off) * scale;
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

					pos.x() += g->advance * scale;
				}
			}
		}

		void add_image_detail(const Vec2f& pos, const Vec2f& size, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col, uint& vtx_cnt, uint& idx_cnt)
		{
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

		void add_image(const Vec2f& _pos, const Vec2f& size, uint id, const Vec2f& _uv0, const Vec2f& _uv1, const Vec4c& tint_col, bool repeat)
		{
			auto pos = Vec2f(Vec2i(_pos));
			auto uv0 = _uv0;
			auto uv1 = _uv1;
			Vec2f img_size;

			auto img_id = (id & 0xffff0000) >> 16;
			begin_draw(CmdDrawElement, img_id);
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;
			auto& img = imgs[img_id];
			auto atlas = img.second;
			if (atlas)
			{
				auto& region = atlas->regions()[id & 0xffff];
				uv0 = mix(region.uv0, region.uv1, uv0);
				uv1 = mix(region.uv0, region.uv1, uv1);
				img_size = region.size;
			}
			else
				img_size = img.first->image()->size;

			if (repeat)
			{
				auto repeat_count = Vec2u(size / img_size);
				for (auto y = 0; y < repeat_count.y(); y++)
				{
					for (auto x = 0; x < repeat_count.x(); x++)
						add_image_detail(pos + Vec2f(x * img_size.x(), y * img_size.y()), img_size, uv0, uv1, tint_col, vtx_cnt, idx_cnt);
				}
			}
			else
				add_image_detail(pos, size, uv0, uv1, tint_col, vtx_cnt, idx_cnt);
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

				clear_color = Vec4c(0, 0, 0, 255);

				imgs.resize(ds->layout()->get_binding(0)->count, std::make_pair((Imageview*)white_iv$i.v, nullptr));

				auto c = new CanvasPrivate;
				c->thiz = this;
				canvas$o.v = c;
				canvas$o.frame = looper().frame;

				auto font_atlases = get_attribute_vec(font_atlases$i);
				for (auto f : font_atlases)
					c->add_font((FontAtlas*)f);

				frame = 0;

				return;
			}

			if (rnf)
			{
				auto fb = (Framebuffer*)rnf->framebuffers()[img_idx];
				surface_size = Vec2f(fb->image_size);

				curr_scissor = Vec4f(Vec2f(0.f), surface_size);

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
					case CmdDrawTextSdf:
					{
						auto c = (TextSdf*)_c;
						add_text(c->font_atals, c->glyphs,  sdf_font_size * c->scale, c->scale, c->pos, c->color);
					}
						break;
					}
				}

				cb->begin();
				auto cv = rnf->clearvalues();
				cv->set(0, clear_color);
				cb->begin_renderpass(fb, cv);
				if (idx_end != idx_buf->mapped)
				{
					cb->set_viewport(curr_scissor);
					cb->set_scissor(curr_scissor);
					cb->bind_vertexbuffer(vtx_buf, 0);
					cb->bind_indexbuffer(idx_buf, IndiceTypeUint);

					struct
					{
						Vec2f scale;
						Vec2f sdf_range;
					}pc;
					pc.scale = Vec2f(2.f / surface_size.x(), 2.f / surface_size.y());
					pc.sdf_range = Vec2f(sdf_range) / font_atlas_size;
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

	void CanvasPrivate::set_clear_color(const Vec4c& col)
	{
		thiz->clear_color = col;
	}

	Imageview* CanvasPrivate::get_image(uint index)
	{
		return thiz->imgs[index].first;
	}

	Atlas* CanvasPrivate::get_atlas(uint index)
	{
		return thiz->imgs[index].second;
	}

	uint CanvasPrivate::set_image(int index, Imageview* v, Filter filter, Atlas* atlas)
	{
		return thiz->set_image(index, v, filter, atlas);
	}

	void CanvasPrivate::stroke(const std::vector<Vec2f>& points, const Vec4c& col, float thickness)
	{
		thiz->stroke(points, col, thickness);
	}

	void CanvasPrivate::fill(const std::vector<Vec2f>& points, const Vec4c& col)
	{
		thiz->fill(points, col);
	}

	void CanvasPrivate::add_text(FontAtlas* f, const std::vector<Glyph*> glyphs, uint line_space, float scale, const Vec2f& pos, const Vec4c& col)
	{
		thiz->add_text(f, glyphs, line_space, scale, pos, col);
	}

	void CanvasPrivate::add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col, bool repeat)
	{
		thiz->add_image(pos, size, id, uv0, uv1, tint_col, repeat);
	}

	const Vec4f& CanvasPrivate::scissor()
	{
		return thiz->curr_scissor;
	}

	void CanvasPrivate::set_scissor(const Vec4f& scissor)
	{
		thiz->set_scissor(scissor);
	}
}

