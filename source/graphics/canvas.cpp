#include "../foundation/window.h"
#include "canvas_private.h"
#include "image_private.h"
#include "font_private.h"
#include "renderpass_private.h"
#include "window_private.h"

namespace flame
{
	namespace graphics
	{
		CanvasPrivate::CanvasPrivate(WindowPtr _window)
		{
			window = _window;
			auto gui_idx = window->renderers.find("Gui"_h);
			window->renderers.add([this](uint idx, CommandBufferPtr cb) {
				buf_vtx.upload(cb);
				buf_vtx.buf_top = buf_vtx.stag_top = 0;
				buf_idx.upload(cb);
				buf_idx.buf_top = buf_idx.stag_top = 0;

				auto vp = Rect(vec2(0), window->native->size);
				cb->set_viewport_and_scissor(vp);
				auto cv = vec4(0.4f, 0.4f, 0.58f, 1.f);
				cb->begin_renderpass(nullptr, window->swapchain->images[idx]->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), &cv);
				cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(buf_idx.buf.get(), IndiceTypeUint);
				cb->bind_pipeline(pl);
				auto scale = 2.f / vp.b;
				cb->push_constant_t(vec4(scale, -1.f, -1.f));
				for (auto& cmd : draw_cmds)
				{
					cb->bind_descriptor_set(0, cmd.ds);
					cb->draw_indexed(cmd.idx_cnt, 0, 0, 1, 0);
				}
				cb->end_renderpass();

				reset();
			}, "Canvas"_h, gui_idx != -1 ? gui_idx : -1);

			{
				std::vector<std::string> defines;
				defines.push_back("col_fmt=" + TypeInfo::serialize_t(Swapchain::format));
				rp = Renderpass::get(L"flame\\shaders\\color.rp", defines);
				defines.push_back("load_op=Load");
				defines.push_back("initia_layout=Attachment");
				rp_load = Renderpass::get(L"flame\\shaders\\color.rp", defines);
			}

			pl = GraphicsPipeline::get(L"flame\\shaders\\canvas.pipeline", { "rp=" + str(rp) });
			buf_vtx.create(sizeof(DrawVert), 360000);
			buf_idx.create(240000);
			main_font = FontAtlas::get({ L"flame\\fonts\\OpenSans-Regular.ttf" });
			main_font->get_glyph(0); // get empty slot at first place to allow embed a white pixel in it
			main_font->init_latin_glyphs();
			main_img = main_font->image.get();
			main_img->set_pixel(0, 0, 0, 0, vec4(1.f));
			main_img->upload_pixels(0, 0, 1, 1, 0, 0);
			main_ds.reset(DescriptorSet::create(nullptr, pl->layout->dsls[0]));
			main_ds->set_image_i(0, 0, main_img->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR }), Sampler::get(FilterNearest, FilterNearest, false, AddressClampToEdge));
			main_ds->update();

			reset();
			push_font(main_font);
		}

		CanvasPrivate::~CanvasPrivate()
		{
			window->renderers.remove("Canvas"_h);
			
			GraphicsPipeline::release(pl);
			FontAtlas::release(main_font);
		}

		void CanvasPrivate::reset()
		{
			draw_cmds.resize(1);
			auto& cmd = draw_cmds[0];
			cmd.idx_cnt = 0;
			cmd.ds = main_ds.get();
		}

		CanvasPrivate::DrawCmd& CanvasPrivate::get_cmd(DescriptorSetPtr ds)
		{
			auto& ret = draw_cmds.back();
			if (ret.ds == ds)
				return ret;
			ret = draw_cmds.emplace_back();
			ret.idx_cnt = 0;
			ret.ds = ds;
			return ret;
		}

		void CanvasPrivate::path_rect(const vec2& a, const vec2& b)
		{
			path.push_back(a);
			path.push_back(vec2(a.x, b.y));
			path.push_back(b);
			path.push_back(vec2(b.x, a.y));
		}

		void CanvasPrivate::stroke(float thickness, const cvec4& col, bool closed)
		{
			thickness *= 0.5f;

			auto& cmd = draw_cmds.back();

			auto get_normal = [](const vec2& p1, const vec2& p2) {
				auto d = normalize(p2 - p1);
				return vec2(d.y, -d.x);
			};

			auto first_normal = get_normal(path[0], path[1]);
			vec2 last_normal = first_normal;

			int n_pts = path.size();
			auto vtx0_off = buf_vtx.stag_top;
			{
				auto& v = buf_vtx.add_t<DrawVert>();

				v.pos = path[0] + first_normal * thickness + 0.5f;
				v.uv = vec2(0.f);
				v.col = col;
			}
			{
				auto& v = buf_vtx.add_t<DrawVert>();
				v.pos = path[0] - first_normal * thickness + 0.5f;
				v.uv = vec2(0.f);
				v.col = col;
			}

			int vtx_off;
			for (auto i = 1; i < n_pts - 1; i++)
			{
				auto n = last_normal;
				last_normal = get_normal(path[i], path[i + 1]);
				n = normalize(n + last_normal);

				vtx_off = buf_vtx.stag_top;
				{
					auto& v = buf_vtx.add_t<DrawVert>();
					v.pos = path[i] + n * thickness + 0.5f;
					v.uv = vec2(0.f);
					v.col = col;
				}
				{
					auto& v = buf_vtx.add_t<DrawVert>();
					v.pos = path[i] - n * thickness + 0.5f;
					v.uv = vec2(0.f);
					v.col = col;
				}

				buf_idx.add(vtx_off - 2);
				buf_idx.add(vtx_off - 1);
				buf_idx.add(vtx_off + 1);
				buf_idx.add(vtx_off - 2);
				buf_idx.add(vtx_off + 1);
				buf_idx.add(vtx_off + 0);
				cmd.idx_cnt += 6;
			}

			vtx_off = buf_vtx.stag_top;
			{
				auto& v = buf_vtx.add_t<DrawVert>();

				v.pos = path[n_pts - 1] + last_normal * thickness + 0.5f;
				v.uv = vec2(0.f);
				v.col = col;
			}
			{
				auto& v = buf_vtx.add_t<DrawVert>();
				v.pos = path[n_pts - 1] - last_normal * thickness + 0.5f;
				v.uv = vec2(0.f);
				v.col = col;
			}
			buf_idx.add(vtx_off - 2);
			buf_idx.add(vtx_off - 1);
			buf_idx.add(vtx_off + 1);
			buf_idx.add(vtx_off - 2);
			buf_idx.add(vtx_off + 1);
			buf_idx.add(vtx_off + 0);
			cmd.idx_cnt += 6;

			if (closed)
			{
				auto n = get_normal(path[n_pts - 1], path[0]);

				auto n1 = normalize(n + last_normal);
				buf_vtx.get_t<DrawVert>(vtx_off + 0).pos = path[n_pts - 1] + n1 * thickness + 0.5f;
				buf_vtx.get_t<DrawVert>(vtx_off + 1).pos = path[n_pts - 1] - n1 * thickness + 0.5f;

				auto n2 = normalize(n + first_normal);
				buf_vtx.get_t<DrawVert>(vtx0_off + 0).pos = path[0] + n2 * thickness + 0.5f;
				buf_vtx.get_t<DrawVert>(vtx0_off + 1).pos = path[0] - n2 * thickness + 0.5f;

				buf_idx.add(vtx_off + 0);
				buf_idx.add(vtx_off + 1);
				buf_idx.add(vtx0_off + 1);
				buf_idx.add(vtx_off + 0);
				buf_idx.add(vtx0_off + 1);
				buf_idx.add(vtx0_off + 0);
				cmd.idx_cnt += 6;
			}

			path.clear();
		}

		void CanvasPrivate::fill(const cvec4& col)
		{
			auto& cmd = draw_cmds.back();

			int n_pts = path.size();
			auto vtx0_off = buf_vtx.stag_top;
			{
				auto& v = buf_vtx.add_t<DrawVert>();
				v.pos = path[0];
				v.uv = vec2(0.f);
				v.col = col;
			}
			{
				auto& v = buf_vtx.add_t<DrawVert>();
				v.pos = path[1];
				v.uv = vec2(0.f);
				v.col = col;
			}
			for (auto i = 0; i < n_pts - 2; i++)
			{
				auto vtx_off = buf_vtx.stag_top;
				{
					auto& v = buf_vtx.add_t<DrawVert>();
					v.pos = path[i + 2];
					v.uv = vec2(0.f);
					v.col = col;
				}

				buf_idx.add(vtx0_off); buf_idx.add(vtx_off - 1); buf_idx.add(vtx_off);
				cmd.idx_cnt += 3;
			}

			path.clear();
		}
		
		void CanvasPrivate::add_rect(const vec2& a, const vec2& b, float thickness, const cvec4& col)
		{
			path_rect(a, b);
			stroke(thickness, col, true);
		}

		void CanvasPrivate::add_rect_filled(const vec2& a, const vec2& b, const cvec4& col)
		{
			path_rect(a, b);
			fill(col);
		}

		void CanvasPrivate::push_font(FontAtlasPtr font)
		{
			fonts.push(font);
		}

		void CanvasPrivate::pop_font()
		{
			if (fonts.size() <= 1)
			{
				printf("graphics canvas: cannot pop the default font\n");
				return;
			}

			fonts.pop();
		}

		void CanvasPrivate::add_text(const vec2& pos, std::wstring_view str, const cvec4& col)
		{
			auto& cmd = draw_cmds.back();

			auto p = pos;
			for (auto ch : str)
			{
				auto& g = fonts.top()->get_glyph(ch);
				auto o = p + vec2(g.off);
				auto s = vec2(g.size);
				s.y *= -1.f;

				path_rect(o, o + s);
				fill(col);
				buf_vtx.get_t<DrawVert>(-4).uv = g.uv.xy;
				buf_vtx.get_t<DrawVert>(-3).uv = g.uv.xw;
				buf_vtx.get_t<DrawVert>(-2).uv = g.uv.zw;
				buf_vtx.get_t<DrawVert>(-1).uv = g.uv.zy;

				p.x += g.advance;
			}
		}

		struct CanvasCreate : Canvas::Create
		{
			CanvasPtr operator()(WindowPtr window) override
			{
				return new CanvasPrivate(window);
			}
		}Canvas_create;
		Canvas::Create& Canvas::create = Canvas_create;
	}
}
