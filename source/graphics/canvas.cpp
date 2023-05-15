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
		CanvasPrivate::CanvasPrivate(WindowPtr _window, bool use_window_targets, std::span<ImageViewPtr> targets)
		{
			window = _window;

			auto gui_idx = window->renderers.find("gui"_h);
			window->renderers.add([this](int idx, CommandBufferPtr cb) {
				if (idx < 0 || iv_tars.empty())
				{
					reset();
					return;
				}

				buf_vtx.upload(cb);
				buf_vtx.buf_top = buf_vtx.stag_top = 0;
				buf_idx.upload(cb);
				buf_idx.buf_top = buf_idx.stag_top = 0;

				cb->begin_debug_label("Canvas");
				idx = fb_tars.size() > 1 ? idx : 0;
				auto vp = Rect(vec2(0), window->native->size);
				cb->set_viewport_and_scissor(vp);
				if (clear_framebuffer)
				{
					auto cv = vec4(0.4f, 0.4f, 0.58f, 1.f);
					cb->begin_renderpass(rp, fb_tars[idx], &cv);
				}
				else
				{
					cb->image_barrier(iv_tars[idx]->image, {}, ImageLayoutAttachment);
					cb->begin_renderpass(rp_load, fb_tars[idx]);
				}
				cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(buf_idx.buf.get(), IndiceTypeUint);
				cb->bind_pipeline(pl);
				prm.pc.mark_dirty_c("scale"_h).as<vec2>() = 2.f / vp.b;
				prm.push_constant(cb);
				auto last_pl = pl;
				auto idx_off = 0;
				for (auto& cmd : draw_cmds)
				{
					if (cmd.idx_cnt > 0)
					{
						switch (cmd.type)
						{
						case DrawCmd::DrawBmp:
							if (last_pl != pl)
							{
								cb->bind_pipeline(pl);
								last_pl = pl;
							}
							break;
						case DrawCmd::DrawSdf:
							if (last_pl != pl_sdf)
							{
								cb->bind_pipeline(pl_sdf);
								last_pl = pl_sdf;
							}
							prm.pc.mark_dirty_c("data"_h).as<vec4>() = vec4(1.f / cmd.data.sdf.scale / 4.f, cmd.data.sdf.thickness, cmd.data.sdf.border, 0.f);
							prm.push_constant(cb);
							break;
						}
						cb->bind_descriptor_set(0, cmd.ds);
						cb->draw_indexed(cmd.idx_cnt, idx_off, 0, 1, 0);
						idx_off += cmd.idx_cnt;
					}
				}
				cb->end_renderpass();
				cb->end_debug_label();

				reset();
			}, "Canvas"_h, gui_idx != -1 ? gui_idx : -1);

			{
				std::vector<std::string> defines;
				defines.push_back("col_fmt=" + TypeInfo::serialize_t(use_window_targets ? Swapchain::format : Format_R8G8B8A8_UNORM));
				defines.push_back("final_layout=ShaderReadOnly");
				rp = Renderpass::get(L"flame\\shaders\\color.rp", defines);
				defines.push_back("load_op=Load");
				defines.push_back("initia_layout=Attachment");
				rp_load = Renderpass::get(L"flame\\shaders\\color.rp", defines);
			}

			if (use_window_targets)
			{
				window->native->resize_listeners.add([this](const uvec2& sz) {
					graphics::Queue::get()->wait_idle();
					iv_tars.clear();
					std::vector<graphics::ImageViewPtr> ivs;
					for (auto& i : window->swapchain->images)
						ivs.push_back(i->get_view());
					set_targets(ivs);
				});
				std::vector<graphics::ImageViewPtr> ivs;
				for (auto& i : window->swapchain->images)
					ivs.push_back(i->get_view());
				set_targets(ivs);
			}
			else
				set_targets(targets);

			pl = GraphicsPipeline::get(L"flame\\shaders\\canvas.pipeline", { "rp=" + str(rp) });
			pl_sdf = GraphicsPipeline::get(L"flame\\shaders\\canvas.pipeline", { "rp=" + str(rp), "frag:MSDF" });
			prm.init(pl->layout, graphics::PipelineGraphics);
			buf_vtx.create(sizeof(DrawVert), 360000);
			buf_idx.create(240000);
			const auto font_size = 14;
			main_font_atlas = FontAtlas::get({ L"flame\\fonts\\OpenSans-Regular.ttf" });
			main_font_atlas->get_glyph(0, font_size); // get empty slot at first place to allow embed a white pixel in it
			main_font_atlas->init_latin_glyphs(font_size);
			main_img = main_font_atlas->image.get();
			main_img->set_pixel(0, 0, 0, 0, vec4(1.f));
			main_img->upload_pixels(0, 0, 1, 1, 0, 0);
			main_img->change_layout(ImageLayoutShaderReadOnly);
			main_ds.reset(DescriptorSet::create(nullptr, prm.get_dsl(""_h)));
			main_ds->set_image_i(0, 0, main_img->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR }), Sampler::get(FilterNearest, FilterNearest, false, AddressClampToEdge));
			main_ds->update();

			reset();
		}

		CanvasPrivate::~CanvasPrivate()
		{
			window->renderers.remove("Canvas"_h);
			
			GraphicsPipeline::release(pl);
			FontAtlas::release(main_font_atlas);
		}

		void CanvasPrivate::set_targets(std::span<ImageViewPtr> targets)
		{
			iv_tars.assign(targets.begin(), targets.end());
			for (auto fb : fb_tars)
				delete fb;
			for (auto iv : iv_tars)
				fb_tars.push_back(Framebuffer::create(rp, iv));
		}

		void CanvasPrivate::reset()
		{
			draw_cmds.clear();
			draw_cmds.emplace_back().ds = main_ds.get();
		}

		CanvasPrivate::DrawCmd& CanvasPrivate::get_bmp_cmd(DescriptorSetPtr ds)
		{
			auto& cmd = draw_cmds.back();
			if (cmd.ds == ds && cmd.type == DrawCmd::DrawBmp)
				return cmd;
			auto& new_cmd = draw_cmds.emplace_back();
			new_cmd.ds = ds;
			return new_cmd;
		}

		CanvasPrivate::DrawCmd& CanvasPrivate::get_sdf_cmd(DescriptorSetPtr ds, float sdf_scale, float thickness, float border)
		{
			auto& cmd = draw_cmds.back();
			if (cmd.ds == ds && cmd.type == DrawCmd::DrawSdf &&
				cmd.data.sdf.scale == sdf_scale &&
				cmd.data.sdf.thickness == thickness &&
				cmd.data.sdf.border == border)
				return cmd;
			auto& new_cmd = draw_cmds.emplace_back();
			new_cmd.type = DrawCmd::DrawSdf;
			new_cmd.ds = ds;
			new_cmd.data.sdf.scale = sdf_scale;
			new_cmd.data.sdf.thickness = thickness;
			new_cmd.data.sdf.border = border;
			return new_cmd;
		}

		void CanvasPrivate::path_rect(const vec2& a, const vec2& b)
		{
			path.push_back(a);
			path.push_back(vec2(a.x, b.y));
			path.push_back(b);
			path.push_back(vec2(b.x, a.y));
		}

		void CanvasPrivate::stroke_path(DrawCmd& cmd, float thickness, const cvec4& col, bool closed)
		{
			auto get_normal = [](const vec2& p1, const vec2& p2) {
				auto d = normalize(p2 - p1);
				return vec2(d.y, -d.x);
			};

			auto first_normal = get_normal(path[0], path[1]);
			vec2 last_normal = first_normal;
			thickness *= 0.5f;

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
				buf_vtx.item_t<DrawVert>(vtx_off + 0).pos = path[n_pts - 1] + n1 * thickness + 0.5f;
				buf_vtx.item_t<DrawVert>(vtx_off + 1).pos = path[n_pts - 1] - n1 * thickness + 0.5f;

				auto n2 = normalize(n + first_normal);
				buf_vtx.item_t<DrawVert>(vtx0_off + 0).pos = path[0] + n2 * thickness + 0.5f;
				buf_vtx.item_t<DrawVert>(vtx0_off + 1).pos = path[0] - n2 * thickness + 0.5f;

				buf_idx.add(vtx_off + 0);
				buf_idx.add(vtx_off + 1);
				buf_idx.add(vtx0_off + 1);
				buf_idx.add(vtx_off + 0);
				buf_idx.add(vtx0_off + 1);
				buf_idx.add(vtx0_off + 0);
				cmd.idx_cnt += 6;
			}
		}

		void CanvasPrivate::fill_path(DrawCmd& cmd, const cvec4& col)
		{
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
		}

		void CanvasPrivate::stroke(float thickness, const cvec4& col, bool closed)
		{
			auto& cmd = get_bmp_cmd(main_ds.get());
			stroke_path(cmd, thickness, col, closed);
			path.clear();
		}

		void CanvasPrivate::fill(const cvec4& col)
		{
			auto& cmd = get_bmp_cmd(main_ds.get());
			fill_path(cmd, col);
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

		void CanvasPrivate::add_text(FontAtlasPtr font_atlas, uint font_size, const vec2& pos, std::wstring_view str, const cvec4& col, float thickness, float border)
		{
			font_atlas = font_atlas ? font_atlas : main_font_atlas;
			thickness = clamp(thickness, -1.f, +1.f);
			border = clamp(border, 0.f, 0.25f);
			auto scale = font_atlas->get_scale(font_size);
			auto ds = font_atlas == main_font_atlas ? main_ds.get() : font_atlas->view->get_shader_read_src(nullptr);
			auto& cmd = font_atlas->type == FontAtlasBitmap ? get_bmp_cmd(ds) : get_sdf_cmd(ds, scale, thickness, border);

			auto p = pos;
			for (auto ch : str)
			{
				if (ch == L'\n')
				{
					p.y += font_size;
					p.x = pos.x;
					continue;
				}

				auto& g = font_atlas->get_glyph(ch, font_size);
				auto o = p + vec2(g.off) * scale;
				auto s = vec2(g.size) * scale;
				s.y *= -1.f;

				path_rect(o, o + s);
				fill_path(cmd, col);
				path.clear();
				buf_vtx.item_t<DrawVert>(-4).uv = g.uv.xy;
				buf_vtx.item_t<DrawVert>(-3).uv = g.uv.xw;
				buf_vtx.item_t<DrawVert>(-2).uv = g.uv.zw;
				buf_vtx.item_t<DrawVert>(-1).uv = g.uv.zy;

				p.x += g.advance * scale;
			}
		}

		void CanvasPrivate::add_image(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs)
		{
			auto& cmd = get_bmp_cmd(view->get_shader_read_src(nullptr));

			path_rect(a, b);
			fill_path(cmd, cvec4(255));
			path.clear();
			buf_vtx.item_t<DrawVert>(-4).uv = uvs.xy;
			buf_vtx.item_t<DrawVert>(-3).uv = uvs.xw;
			buf_vtx.item_t<DrawVert>(-2).uv = uvs.zw;
			buf_vtx.item_t<DrawVert>(-1).uv = uvs.zy;
		}

		struct CanvasCreate : Canvas::Create
		{
			CanvasPtr operator()(WindowPtr window) override
			{
				return new CanvasPrivate(window);
			}

			CanvasPtr operator()(WindowPtr window, std::span<ImageViewPtr> targets) override
			{
				return new CanvasPrivate(window, false, targets);
			}
		}Canvas_create;
		Canvas::Create& Canvas::create = Canvas_create;
	}
}
