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
				cb->bind_descriptor_set(0, main_ds.get());
				auto scale = 2.f / vp.b;
				cb->push_constant_t(vec4(scale, -1.f, -1.f));
				for (auto& cmd : draw_cmds)
					cb->draw_indexed(cmd.idx_cnt, 0, 0, 1, 0);
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
			font_atlas = FontAtlas::get({ L"flame\\fonts\\OpenSans-Regular.ttf" });
			font_atlas->get_glyph(0, 14); // get empty slot at first place to allow embed a white pixel in it
			for (auto ch = 0x0020; ch <= 0x00FF; ch++)
				font_atlas->get_glyph(ch, 14);
			main_img = font_atlas->image.get();
			main_img->set_pixel(0, 0, 0, 0, vec4(1.f));
			main_img->upload_pixels(0, 0, 1, 1, 0, 0);
			main_ds.reset(DescriptorSet::create(nullptr, pl->layout->dsls[0]));
			main_ds->set_image_i(0, 0, main_img->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR }), Sampler::get(FilterNearest, FilterNearest, false, AddressClampToEdge));
			main_ds->update();

			reset();
		}

		CanvasPrivate::~CanvasPrivate()
		{
			window->renderers.remove("Canvas"_h);
			
			GraphicsPipeline::release(pl);
			FontAtlas::release(font_atlas);
		}

		void CanvasPrivate::reset()
		{
			draw_cmds.resize(1);
			auto& cmd = draw_cmds[0];
			cmd.idx_cnt = 0;
			cmd.tex = main_img;
		}

		void CanvasPrivate::path_rect(const vec2& a, const vec2& b)
		{
			path.push_back(a);
			path.push_back(vec2(a.x, b.y));
			path.push_back(b);
			path.push_back(vec2(b.x, a.y));
		}

		void CanvasPrivate::stroke(float thickness, const cvec4& col)
		{
			//thickness *= 0.5f;

			//auto& cmd = draw_cmds.back();

			//auto get_normal = [](const vec2& p1, const vec2& p2) {
			//	auto d = normalize(p2 - p1);
			//	return vec2(d.y, -d.x);
			//};

			//vec2 first_normal;
			//vec2 last_normal;

			//first_normal = last_normal = get_normal(pts[0], pts[1]);
			//{
			//	auto& v = buf_vtx.add_t<DrawVert>();
			//	v.pos = pts[0] + first_normal * thickness + 0.5f;
			//	v.uv = vec2(0.f);
			//	v.col = col;
			//}
			//{
			//	auto& v = buf_vtx.add_t<DrawVert>();
			//	v.pos = pts[0] - first_normal * thickness + 0.5f;
			//	v.uv = vec2(0.f);
			//	v.col = col;
			//}

			//{
			//	for (auto i = 1; i < pt_cnt - 1; i++)
			//	{
			//		auto _n = get_normal(pts[i], pts[i + 1]);
			//		auto n = normalize(last_normal + _n);
			//		last_normal = _n;

			//		auto vtx_off = buf_vtx.stag_top;
			//		{
			//			auto& v = buf_vtx.add_t<DrawVert>();
			//			v.pos = pts[i] + n * thickness + 0.5f;
			//			v.uv = vec2(0.f);
			//			v.col = col;
			//		}
			//		{
			//			auto& v = buf_vtx.add_t<DrawVert>();
			//			v.pos = pts[i] - n * thickness + 0.5f;
			//			v.uv = vec2(0.f);
			//			v.col = col;
			//		}

			//		buf_idx.add(vtx_off - 2);
			//		buf_idx.add(vtx_off - 1);
			//		buf_idx.add(vtx_off + 1);
			//		buf_idx.add(vtx_off - 2);
			//		buf_idx.add(vtx_off + 1);
			//		buf_idx.add(vtx_off + 0);
			//		cmd.idx_cnt += 6;
			//	}
			//}

			//{
			//	auto _n = get_normal(pts[pt_cnt - 2], pts[0]);
			//	auto n = normalize(_n + first_normal);

			//	auto vtx_off = buf_vtx.stag_top;

			//	buf_idx.add(vtx_off - 2);
			//	buf_idx.add(vtx_off - 1);
			//	buf_idx.add(1);
			//	buf_idx.add(vtx_off - 2);
			//	buf_idx.add(1);
			//	buf_idx.add(0);
			//	cmd.idx_cnt += 6;
			//}

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
		}

		void CanvasPrivate::add_rect_filled(const vec2& a, const vec2& b, const cvec4& col)
		{
			path_rect(a, b);
			fill(col);
		}

		void CanvasPrivate::push_font(FontAtlasPtr font)
		{

		}

		void CanvasPrivate::pop_font()
		{
			if (fonts.size() <= 1)
			{
				printf("graphics canvas: cannot pop the default font\n");
				return;
			}
		}

		void CanvasPrivate::add_text(const vec2& pos, std::wstring_view str, const cvec4& col)
		{
			auto& cmd = draw_cmds.back();

			auto p = pos;
			for (auto ch : str)
			{
				auto& g = font_atlas->get_glyph(ch, 14);
				auto o = p + vec2(g.off);
				auto s = vec2(g.size);

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
