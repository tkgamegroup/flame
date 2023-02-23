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
			}, "Canvas"_h);

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

		void CanvasPrivate::add_rect_filled(const vec2& a, const vec2& b, const cvec4& col)
		{
			auto& cmd = draw_cmds.back();

			auto vtx_off = buf_vtx.stag_top;
			{
				auto& v = buf_vtx.add_t<DrawVert>();
				v.pos = a;
				v.uv = vec2(0.f);
				v.col = col;
			}
			{
				auto& v = buf_vtx.add_t<DrawVert>();
				v.pos = vec2(a.x, b.y);
				v.uv = vec2(0.f);
				v.col = col;
			}
			{
				auto& v = buf_vtx.add_t<DrawVert>();
				v.pos = b;
				v.uv = vec2(0.f);
				v.col = col;
			}
			{
				auto& v = buf_vtx.add_t<DrawVert>();
				v.pos = vec2(b.x, a.y);
				v.uv = vec2(0.f);
				v.col = col;
			}

			buf_idx.add(vtx_off + 0); buf_idx.add(vtx_off + 1); buf_idx.add(vtx_off + 2);
			buf_idx.add(vtx_off + 0); buf_idx.add(vtx_off + 2); buf_idx.add(vtx_off + 3);

			cmd.idx_cnt += 6;
		}

		void CanvasPrivate::add_text(const vec2& pos, std::wstring_view str, const cvec4& col)
		{
			auto& cmd = draw_cmds.back();

			for (auto& g : font_atlas->get_draw_glyphs(14, str, pos))
			{
				auto vtx_off = buf_vtx.stag_top;
				{
					auto& v = buf_vtx.add_t<DrawVert>();
					v.pos = g.points[0];
					v.uv = g.uvs.xy;
					v.col = col;
				}
				{
					auto& v = buf_vtx.add_t<DrawVert>();
					v.pos = g.points[1];
					v.uv = g.uvs.xw;
					v.col = col;
				}
				{
					auto& v = buf_vtx.add_t<DrawVert>();
					v.pos = g.points[2];
					v.uv = g.uvs.zw;
					v.col = col;
				}
				{
					auto& v = buf_vtx.add_t<DrawVert>();
					v.pos = g.points[3];
					v.uv = g.uvs.zy;
					v.col = col;
				}

				buf_idx.add(vtx_off + 0); buf_idx.add(vtx_off + 1); buf_idx.add(vtx_off + 2);
				buf_idx.add(vtx_off + 0); buf_idx.add(vtx_off + 2); buf_idx.add(vtx_off + 3);

				cmd.idx_cnt += 6;
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
