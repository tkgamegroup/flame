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
#define PRECOMPUTE_CIRCLES_NUMBER 64U
		static std::vector<vec2> precompute_circles[PRECOMPUTE_CIRCLES_NUMBER];

		const std::vector<vec2>& get_precompute_circle(float r)
		{
			auto idx = min((uint)(r / 4.f), PRECOMPUTE_CIRCLES_NUMBER - 1);
			auto& ret = precompute_circles[idx];
			if (ret.empty())
			{
				ret.resize(idx + 8);
				auto s = 1.f / (float)ret.size() * 2.f * pi<float>();
				for (auto i = 0; i < ret.size(); i++)
				{
					auto a = i * s;
					ret[i] = vec2(cos(a), sin(a));
				}
			}
			return ret;
		}

		CanvasPrivate::CanvasPrivate(bool hdr)
		{
			create_renderpass(hdr ? Format_R16G16B16A16_SFLOAT : Format_R8G8B8A8_UNORM);

			std::string rp_define = "rp=" + str(rp);
			std::vector<std::string> stencil_write_defines = { "stencil_test=true", "stencil_op=" + TypeInfo::serialize_t(StencilOpReplace), "color_mask=" + TypeInfo::serialize_t(ColorComponentNone), "frag:ALPHA_TEST" };
			std::vector<std::string> stencil_compare_defines = { "stencil_test=true", "stencil_compare_op=" + TypeInfo::serialize_t(CompareOpEqual), };
			pl = GraphicsPipeline::get(L"flame\\shaders\\canvas.pipeline", { rp_define });
			{
				std::vector<std::string> defines;
				defines.push_back(rp_define);
				defines.insert(defines.end(), stencil_write_defines.begin(), stencil_write_defines.end());
				pl_stencil_write = GraphicsPipeline::get(L"flame\\shaders\\canvas.pipeline", defines);
			}
			{
				std::vector<std::string> defines;
				defines.push_back(rp_define);
				defines.insert(defines.end(), stencil_compare_defines.begin(), stencil_compare_defines.end());
				pl_stencil_compare = GraphicsPipeline::get(L"flame\\shaders\\canvas.pipeline", defines);
			}
			std::string sdf_define = "frag:MSDF";
			pl_sdf = GraphicsPipeline::get(L"flame\\shaders\\canvas.pipeline", { rp_define, sdf_define });
			{
				std::vector<std::string> defines;
				defines.push_back(rp_define);
				defines.push_back(sdf_define);
				defines.insert(defines.end(), stencil_write_defines.begin(), stencil_write_defines.end());
				pl_sdf_stencil_write = GraphicsPipeline::get(L"flame\\shaders\\canvas.pipeline", defines);
			}
			{
				std::vector<std::string> defines;
				defines.push_back(rp_define);
				defines.push_back(sdf_define);
				defines.insert(defines.end(), stencil_compare_defines.begin(), stencil_compare_defines.end());
				pl_sdf_stencil_compare = GraphicsPipeline::get(L"flame\\shaders\\canvas.pipeline", defines);
			}
			prm.init(pl->layout, graphics::PipelineGraphics);
			buf_vtx.create(sizeof(DrawVert), 360000);
			buf_idx.create(240000);
			default_font_atlas = FontAtlas::get({ L"flame\\fonts\\OpenSans-Regular.ttf" });
			main_img = default_font_atlas->image.get();
			main_img->set_staging_pixel(0, 0, 0, 0, vec4(1.f));
			main_img->upload_staging_pixels(0, 0, 1, 1, 0, 0);
			main_img->change_layout(ImageLayoutShaderReadOnly);
			main_ds.reset(DescriptorSet::create(nullptr, prm.get_dsl(""_h)));
			main_ds->set_image_i(0, 0, main_img->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR }), Sampler::get(FilterNearest, FilterNearest, false, AddressClampToEdge));
			main_ds->update();

			reset_drawing();
		}

		CanvasPrivate::~CanvasPrivate()
		{
			GraphicsPipeline::release(pl);
			FontAtlas::release(default_font_atlas);
		}

		void CanvasPrivate::create_renderpass(Format format)
		{
			std::vector<std::string> defines;
			defines.push_back("col_fmt=" + TypeInfo::serialize_t(format));
			rp = Renderpass::get(L"flame\\shaders\\canvas.rp", defines);
			defines.push_back("load_op=Load");
			defines.push_back("initia_layout=Attachment");
			rp_load = Renderpass::get(L"flame\\shaders\\canvas.rp", defines);
		}

		void CanvasPrivate::set_targets(std::span<ImageViewPtr> targets)
		{
			iv_tars.assign(targets.begin(), targets.end());
			for (auto fb : fb_tars)
				delete fb;

			size = targets.empty() ? vec2(0.f) : (vec2)targets[0]->image->extent.xy();

			reset_drawing();
			fb_tars.clear();

			if (size.x <= 0.f && size.y <= 0.f)
				return;

			if (!stencil_img || (vec2)stencil_img->extent.xy() != size)
			{
				stencil_img.reset(Image::create(Format_Stencil8, uvec3((uvec2)size, 1), ImageUsageAttachment));
				stencil_img->change_layout(ImageLayoutAttachment);
			}
			for (auto iv : iv_tars)
			{
				ImageViewPtr views[] = { iv, stencil_img->get_view() };
				fb_tars.push_back(Framebuffer::create(rp, views));
			}
		}

		void CanvasPrivate::bind_window(WindowPtr window)
		{
			assert(!bound_window);
			bound_window = window;
			
			create_renderpass(Swapchain::format);

			window->native->resize_listeners.add([this](const uvec2& sz) {
				graphics::Queue::get()->wait_idle();
				iv_tars.clear();
				std::vector<graphics::ImageViewPtr> ivs;
				for (auto& i : bound_window->swapchain->images)
					ivs.push_back(i->get_view());
				set_targets(ivs);
			});
			std::vector<graphics::ImageViewPtr> ivs;
			for (auto& i : window->swapchain->images)
				ivs.push_back(i->get_view());
			set_targets(ivs);
		}

		void CanvasPrivate::register_icon(wchar_t code, const graphics::ImageDesc& image)
		{
			if (code < ICON_BEGIN || code > ICON_END)
				return;
			auto idx = code - ICON_BEGIN;
			if (idx >= icons.size())
				icons.resize(idx + 1);
			icons[idx] = image;
		}

		void CanvasPrivate::reset_drawing()
		{
			while (!scissor_stack.empty())
				scissor_stack.pop();
			if (!iv_tars.empty())
				push_scissor(Rect(vec2(0.f), iv_tars.front()->image->extent.xy()));
			draw_cmds.clear();
			draw_cmds.emplace_back().ds = main_ds.get();
			buf_vtx.reset();
			buf_idx.reset();
		}

		CanvasPrivate::DrawCmd& CanvasPrivate::get_blit_cmd(DescriptorSetPtr ds)
		{
			auto& cmd = draw_cmds.back();
			if (cmd.ds == ds && cmd.type == DrawCmd::Blit)
				return cmd;
			auto& new_cmd = draw_cmds.emplace_back();
			new_cmd.type = DrawCmd::Blit;
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

		CanvasPrivate::DrawVert* CanvasPrivate::stroke_path(DrawCmd& cmd, float thickness, const cvec4& col, bool closed)
		{
			int n_pts = path.size();
			if (n_pts < 2)
				return nullptr;

			auto get_normal = [](const vec2& p1, const vec2& p2) {
				auto d = normalize(p2 - p1);
				return vec2(d.y, -d.x);
			};

			auto first_normal = get_normal(path[0], path[1]);
			vec2 last_normal = first_normal;
			thickness *= 0.5f;

			std::vector<DrawVert> vertices;
			std::vector<uint> indices;
			{
				auto& v = vertices.emplace_back();
				v.pos = path[0] + first_normal * thickness;
				v.uv = vec2(0.f);
				v.col = col;
			}
			{
				auto& v = vertices.emplace_back();
				v.pos = path[0] - first_normal * thickness;
				v.uv = vec2(0.f);
				v.col = col;
			}

			for (auto i = 1; i < n_pts - 1; i++)
			{
				auto n = last_normal;
				last_normal = get_normal(path[i], path[i + 1]);
				n = normalize(n + last_normal);
				auto t = thickness / dot(n, last_normal);

				int vtx_off = vertices.size();
				{
					auto& v = vertices.emplace_back();
					v.pos = path[i] + n * t;
					v.uv = vec2(0.f);
					v.col = col;
				}
				{
					auto& v = vertices.emplace_back();
					v.pos = path[i] - n * t;
					v.uv = vec2(0.f);
					v.col = col;
				}

				indices.push_back(vtx_off - 2);
				indices.push_back(vtx_off - 1);
				indices.push_back(vtx_off + 1);
				indices.push_back(vtx_off - 2);
				indices.push_back(vtx_off + 1);
				indices.push_back(vtx_off + 0);
			}

			int vtx_off = vertices.size();
			{
				auto& v = vertices.emplace_back();
				v.pos = path[n_pts - 1] + last_normal * thickness;
				v.uv = vec2(0.f);
				v.col = col;
			}
			{
				auto& v = vertices.emplace_back();
				v.pos = path[n_pts - 1] - last_normal * thickness;
				v.uv = vec2(0.f);
				v.col = col;
			}
			indices.push_back(vtx_off - 2);
			indices.push_back(vtx_off - 1);
			indices.push_back(vtx_off + 1);
			indices.push_back(vtx_off - 2);
			indices.push_back(vtx_off + 1);
			indices.push_back(vtx_off + 0);

			if (closed)
			{
				if (path[n_pts - 1] != path[0])
				{
					auto n = get_normal(path[n_pts - 1], path[0]);
					auto n1 = normalize(n + last_normal);
					auto t1 = thickness / dot(n1, last_normal);
					vertices[vtx_off + 0].pos = path[n_pts - 1] + n1 * t1;
					vertices[vtx_off + 1].pos = path[n_pts - 1] - n1 * t1;
					auto n2 = normalize(n + first_normal);
					auto t2 = thickness / dot(n2, first_normal);
					vertices[0].pos = path[0] + n2 * t2;
					vertices[1].pos = path[0] - n2 * t2;

					indices.push_back(vtx_off + 0);
					indices.push_back(vtx_off + 1);
					indices.push_back(1);
					indices.push_back(vtx_off + 0);
					indices.push_back(1);
					indices.push_back(0);
				}
				else
				{
					auto n = normalize(first_normal + last_normal);
					auto t1 = thickness / dot(n, last_normal);
					vertices[vtx_off + 0].pos = path[n_pts - 1] + n * t1;
					vertices[vtx_off + 1].pos = path[n_pts - 1] - n * t1;
					auto t2 = thickness / dot(n, first_normal);
					vertices[0].pos = path[0] + n * t2;
					vertices[1].pos = path[0] - n * t2;
				}
			}

			auto buf_vtx_off = buf_vtx.add(vertices.data(), vertices.size());
			for (auto i = 0; i < indices.size(); i++)
				indices[i] += buf_vtx_off;
			buf_idx.add(indices.data(), indices.size());
			cmd.idx_cnt += indices.size();
			return &buf_vtx.item_t<DrawVert>(buf_vtx_off);
		}

		CanvasPrivate::DrawVert* CanvasPrivate::fill_path(DrawCmd& cmd, const cvec4& col)
		{
			int n_pts = path.size();
			if (n_pts < 3)
				return nullptr;

			std::vector<DrawVert> vertices;
			std::vector<uint> indices;
			{
				auto& v = vertices.emplace_back();
				v.pos = path[0];
				v.uv = vec2(0.f);
				v.col = col;
			}
			{
				auto& v = vertices.emplace_back();
				v.pos = path[1];
				v.uv = vec2(0.f);
				v.col = col;
			}
			for (auto i = 0; i < n_pts - 2; i++)
			{
				auto vtx_off = vertices.size();
				{
					auto& v = vertices.emplace_back();
					v.pos = path[i + 2];
					v.uv = vec2(0.f);
					v.col = col;
				}

				indices.push_back(0); 
				indices.push_back(vtx_off - 1);
				indices.push_back(vtx_off);
			}

			auto buf_vtx_off = buf_vtx.add(vertices.data(), vertices.size());
			for (auto i = 0; i < indices.size(); i++)
				indices[i] += buf_vtx_off;
			buf_idx.add(indices.data(), indices.size());
			cmd.idx_cnt += indices.size();
			return &buf_vtx.item_t<DrawVert>(buf_vtx_off);
		}

		Canvas::DrawVert* CanvasPrivate::stroke(float thickness, const cvec4& col, bool closed)
		{
			auto& cmd = get_blit_cmd(main_ds.get());
			auto verts = stroke_path(cmd, thickness, col, closed);
			path.clear();
			return verts;
		}

		Canvas::DrawVert* CanvasPrivate::fill(const cvec4& col)
		{
			auto& cmd = get_blit_cmd(main_ds.get());
			auto verts = fill_path(cmd, col);
			path.clear();
			return verts;
		}

		uint CanvasPrivate::set_translate(const vec2& translate)
		{
			auto& new_cmd = draw_cmds.emplace_back();
			new_cmd.type = DrawCmd::SetTranslate;
			new_cmd.data.translate = translate;
			return draw_cmds.size() - 1;
		}

		void CanvasPrivate::push_scissor(const Rect& _rect)
		{
			Rect rect;
			if (scissor_stack.empty())
				rect = _rect;
			else
			{
				auto& curr = scissor_stack.top();
				rect.a = max(_rect.a, curr.a);
				rect.b = min(_rect.b, curr.b);
			}
			scissor_stack.push(rect);

			auto& new_cmd = draw_cmds.emplace_back();
			new_cmd.type = DrawCmd::SetScissor;
			new_cmd.data.rect = rect;
		}

		void CanvasPrivate::pop_scissor()
		{
			scissor_stack.pop();

			auto& new_cmd = draw_cmds.emplace_back();
			new_cmd.type = DrawCmd::SetScissor;
			new_cmd.data.rect = scissor_stack.top();
		}

		void CanvasPrivate::begin_stencil_write()
		{
			assert(stencil_state == StencilStateNone);
			stencil_state = StencilStateWrite;

			auto& new_cmd = draw_cmds.emplace_back();
			new_cmd.type = DrawCmd::SetStencilState;
			new_cmd.data.stencil_state = stencil_state;
		}

		void CanvasPrivate::end_stencil_write()
		{
			assert(stencil_state == StencilStateWrite);
			stencil_state = StencilStateNone;

			auto& new_cmd = draw_cmds.emplace_back();
			new_cmd.type = DrawCmd::SetStencilState;
			new_cmd.data.stencil_state = stencil_state;
		}

		void CanvasPrivate::begin_stencil_compare()
		{
			assert(stencil_state == StencilStateNone);
			stencil_state = StencilStateCompare;

			auto& new_cmd = draw_cmds.emplace_back();
			new_cmd.type = DrawCmd::SetStencilState;
			new_cmd.data.stencil_state = stencil_state;
		}

		void CanvasPrivate::end_stencil_compare()
		{
			assert(stencil_state == StencilStateCompare);
			stencil_state = StencilStateNone;

			auto& new_cmd = draw_cmds.emplace_back();
			new_cmd.type = DrawCmd::SetStencilState;
			new_cmd.data.stencil_state = stencil_state;
		}
		
		Canvas::DrawVert* CanvasPrivate::draw_rect(const vec2& a, const vec2& b, float thickness, const cvec4& col)
		{
			path_rect(a, b);
			return stroke(thickness, col, true);
		}

		Canvas::DrawVert* CanvasPrivate::draw_rect_filled(const vec2& a, const vec2& b, const cvec4& col)
		{
			path_rect(a, b);
			return fill(col);
		}

		Canvas::DrawVert* CanvasPrivate::draw_rect_rotated(const vec2& a, const vec2& b, float thickness, const cvec4& col, float angle)
		{
			path_rect(a, b);
			auto verts = stroke(thickness, col, true);
			auto c = (a + b) * 0.5f;
			auto r = rotate(mat3(1.f), radians(angle));
			for (auto i = 0; i < 4; i++)
			{
				auto& vtx = verts[i];
				vtx.pos = vec2(r * vec3(vtx.pos - c, 1.f)) + c;
			}
			return verts;
		}

		Canvas::DrawVert* CanvasPrivate::draw_rect_filled_rotated(const vec2& a, const vec2& b, const cvec4& col, float angle)
		{
			path_rect(a, b);
			auto verts = fill(col);
			auto c = (a + b) * 0.5f;
			auto r = rotate(mat3(1.f), radians(angle));
			for (auto i = 0; i < 4; i++)
			{
				auto& vtx = verts[i];
				vtx.pos = vec2(r * vec3(vtx.pos - c, 1.f)) + c;
			}
			return verts;
		}

		Canvas::DrawVert* CanvasPrivate::draw_circle(const vec2& p, float radius, float thickness, const cvec4& col, float begin, float end)
		{
			auto& circle = get_precompute_circle(radius);
			auto ib = int(circle.size() * clamp(begin, 0.f, 1.f));
			auto ie = int(circle.size() * clamp(end, 0.f, 1.f));
			path.resize(ie - ib);
			for (auto i = 0; i < path.size(); i++)
				path[i] = p + circle[ib + i] * radius;
			return stroke(thickness, col, (begin == 0.f && end == 1.f));
		}

		Canvas::DrawVert* CanvasPrivate::draw_circle_filled(const vec2& p, float radius, const cvec4& col, float begin, float end)
		{
			auto& circle = get_precompute_circle(radius);
			auto ib = int(circle.size() * clamp(begin, 0.f, 1.f));
			auto ie = int(circle.size() * clamp(end, 0.f, 1.f));
			path.resize(ie - ib);
			for (auto i = 0; i < path.size(); i++)
				path[i] = p + circle[ib + i] * radius;
			return fill(col);
		}

		vec2 CanvasPrivate::calc_text_size(FontAtlasPtr font_atlas, uint font_size, std::wstring_view str)
		{
			font_atlas = font_atlas ? font_atlas : default_font_atlas;
			auto font_scale = font_atlas->get_scale(font_size);
			auto p = vec2(0.f);
			auto max_x = 0.f;
			for (auto ch : str)
			{
				if (ch == L'\n')
				{
					p.y += font_size * font_scale;
					p.x = 0.f;
					continue;
				}
				if (ch >= ICON_BEGIN)
				{
					p.x += font_size * font_scale;
					max_x = max(max_x, p.x);
					continue;
				}

				auto& g = font_atlas->get_glyph(ch, font_size);
				p.x += g.advance * font_scale;
				max_x = max(max_x, p.x);
			}
			return vec2(max_x, p.y + font_size);
		}

		void CanvasPrivate::draw_text(FontAtlasPtr font_atlas, uint font_size, const vec2& pos, std::wstring_view str, const cvec4& col, float thickness, float border, const vec2& scl)
		{
			font_atlas = font_atlas ? font_atlas : default_font_atlas;
			auto font_scale = font_atlas->get_scale(font_size);
			auto scale = font_scale * scl;
			auto ds = font_atlas == default_font_atlas ? main_ds.get() : font_atlas->view->get_shader_read_src(nullptr);
			auto& cmd = font_atlas->type == FontAtlasBitmap ? get_blit_cmd(ds) : 
				get_sdf_cmd(ds, font_scale, clamp(thickness, -1.f, +1.f), clamp(border, 0.f, 0.25f));

			auto p = pos; vec2 sz(0.f);
			for (auto ch : str)
			{
				if (ch == L'\n')
				{
					p.y += font_size * scale.y;
					p.x = pos.x;
					continue;
				}
				if (ch >= ICON_BEGIN)
				{
					auto idx = ch - ICON_BEGIN;
					auto& img = icons[idx];
					auto s = vec2(font_size) * scale;
					path_rect(p, p + s);
					auto& cmd = get_blit_cmd(img.view->get_shader_read_src(nullptr));
					auto verts = fill_path(cmd, cvec4(255, 255, 255, col.a));
					path.clear();
					verts[0].uv = img.uvs.xy;
					verts[1].uv = img.uvs.xw;
					verts[2].uv = img.uvs.zw;
					verts[3].uv = img.uvs.zy;
					continue;
				}

				auto& g = font_atlas->get_glyph(ch, font_size);
				auto o = p + vec2(g.off) * scale;
				auto s = vec2(g.size) * scale;
				s.y *= -1.f;

				path_rect(o, o + s);
				auto verts = fill_path(cmd, col);
				path.clear();
				verts[0].uv = g.uv.xy;
				verts[1].uv = g.uv.xw;
				verts[2].uv = g.uv.zw;
				verts[3].uv = g.uv.zy;

				p.x += g.advance * scale.x;
			}
		}

		Canvas::DrawVert* CanvasPrivate::draw_image(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs, const cvec4& tint_col, SamplerPtr sp)
		{
			auto& cmd = get_blit_cmd(view->get_shader_read_src(sp));

			path_rect(a, b);
			auto verts = fill_path(cmd, tint_col);
			path.clear();
			verts[0].uv = uvs.xy;
			verts[1].uv = uvs.xw;
			verts[2].uv = uvs.zw;
			verts[3].uv = uvs.zy;
			return verts;
		}

		Canvas::DrawVert* CanvasPrivate::draw_image_stretched(ImageViewPtr iv, const vec2& p0, const vec2& p1, const vec4& uvs, const vec4& border, const vec4& border_uvs, const cvec4& tint_col, SamplerPtr sp)
		{
			Canvas::DrawVert* ret = nullptr;

			if (p1.x - p0.x > border.x + border.z && p1.y - p0.y > border.y + border.w)
			{
				auto uv0 = uvs.xy(); auto uv1 = uvs.zw();
				ret = // get the pointer to the first vertex
				draw_image(iv, vec2(p0.x + border.x, p0.y), vec2(p1.x - border.z, p0.y + border.y), vec4(mix(uv0, uv1, vec2(border_uvs.x, 0.f)), mix(uv0, uv1, vec2(border_uvs.z, border_uvs.y))), tint_col, sp); // top border
				draw_image(iv, vec2(p0.x + border.x, p1.y - border.w), vec2(p1.x - border.z, p1.y), vec4(mix(uv0, uv1, vec2(border_uvs.x, border_uvs.w)), mix(uv0, uv1, vec2(border_uvs.w, 1.f))), tint_col, sp); // bottom border
				draw_image(iv, vec2(p0.x, p0.y + border.y), vec2(p0.x + border.x, p1.y - border.w), vec4(mix(uv0, uv1, vec2(0.f, border_uvs.y)), mix(uv0, uv1, vec2(border_uvs.x, border_uvs.w))), tint_col, sp); // left border
				draw_image(iv, vec2(p1.x - border.z, p0.y + border.y), vec2(p1.x, p1.y - border.w), vec4(mix(uv0, uv1, vec2(border_uvs.w, border_uvs.y)), mix(uv0, uv1, vec2(1.f, border_uvs.w))), tint_col, sp); // right border
				draw_image(iv, vec2(p0.x, p0.y), vec2(p0.x + border.x, p0.y + border.y), vec4(mix(uv0, uv1, vec2(0.f, 0.f)), mix(uv0, uv1, vec2(border_uvs.x, border_uvs.y))), tint_col, sp); // left-top corner
				draw_image(iv, vec2(p1.x - border.z, p0.y), vec2(p1.x, p0.y + border.y), vec4(mix(uv0, uv1, vec2(border_uvs.w, 0.f)), mix(uv0, uv1, vec2(1.f, border_uvs.y))), tint_col, sp); // right-top corner
				draw_image(iv, vec2(p0.x, p1.y - border.w), vec2(p0.x + border.x, p1.y), vec4(mix(uv0, uv1, vec2(0.f, border_uvs.w)), mix(uv0, uv1, vec2(border_uvs.x, 1.f))), tint_col, sp); // left-bottom corner
				draw_image(iv, vec2(p1.x - border.z, p1.y - border.w), vec2(p1.x, p1.y), vec4(mix(uv0, uv1, vec2(border_uvs.w, border_uvs.w)), mix(uv0, uv1, vec2(1.f, 1.f))), tint_col, sp); // right-bottom corner
				draw_image(iv, vec2(p0.x + border.x, p0.y + border.y), vec2(p1.x - border.z, p1.y - border.w), vec4(mix(uv0, uv1, vec2(border_uvs.x, border_uvs.y)), mix(uv0, uv1, vec2(border_uvs.z, border_uvs.w))), tint_col, sp); // middle
			}

			return ret;
		}

		Canvas::DrawVert* CanvasPrivate::draw_image_rotated(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs, const cvec4& tint_col, float angle, SamplerPtr sp)
		{
			auto& cmd = get_blit_cmd(view->get_shader_read_src(sp));

			path_rect(a, b);
			auto verts = fill_path(cmd, tint_col);
			path.clear();
			auto c = (a + b) * 0.5f;
			auto r = rotate(mat3(1.f), radians(angle));
			{
				auto& vtx = verts[0];
				vtx.pos = vec2(r * vec3(vtx.pos - c, 1.f)) + c;
				vtx.uv = uvs.xy;
			}
			{
				auto& vtx = verts[1];
				vtx.pos = vec2(r * vec3(vtx.pos - c, 1.f)) + c;
				vtx.uv = uvs.xw;
			}
			{
				auto& vtx = verts[2];
				vtx.pos = vec2(r * vec3(vtx.pos - c, 1.f)) + c;
				vtx.uv = uvs.zw;
			}
			{
				auto& vtx = verts[3];
				vtx.pos = vec2(r * vec3(vtx.pos - c, 1.f)) + c;
				vtx.uv = uvs.zy;
			}
			return verts;
		}

		Canvas::DrawVert* CanvasPrivate::draw_image_polygon(ImageViewPtr view, std::span<vec2> pts, std::span<vec2> uvs, const cvec4& tint_col, SamplerPtr sp)
		{
			auto& cmd = get_blit_cmd(view->get_shader_read_src(sp));

			path.assign_range(pts);
			auto verts = fill_path(cmd, tint_col);
			path.clear();
			for (auto i = 0; i < pts.size(); i++)
				verts[i].uv = uvs[i];
			return verts;
		}

		void CanvasPrivate::render(int idx, CommandBufferPtr cb, const vec2& translate, const vec2& scaling)
		{
			if (idx < 0 || iv_tars.empty())
			{
				reset_drawing();
				return;
			}

			buf_vtx.upload(cb);
			buf_vtx.reset();
			buf_idx.upload(cb);
			buf_idx.reset();

			cb->begin_debug_label("Canvas");
			idx = fb_tars.size() > 1 ? idx : 0;
			auto vp = Rect(vec2(0.f), iv_tars.front()->image->extent.xy());
			cb->set_viewport_and_scissor(vp);
			if (clear_framebuffer)
			{
				vec4 cvs[] = { vec4(0.4f, 0.4f, 0.58f, 1.f), vec4(0.f) };
				cb->begin_renderpass(rp, fb_tars[idx], cvs);
			}
			else
			{
				cb->image_barrier(iv_tars[idx]->image, {}, ImageLayoutAttachment);
				vec4 cvs[] = { vec4(0.f), vec4(0.f) };
				cb->begin_renderpass(rp_load, fb_tars[idx], cvs);
			}
			cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
			cb->bind_index_buffer(buf_idx.buf.get(), IndiceTypeUint);
			cb->bind_pipeline(pl);
			prm.pc.mark_dirty_c("translate"_h).as<vec2>() = translate;
			prm.pc.mark_dirty_c("scale"_h).as<vec2>() = scaling * 2.f / vp.b;
			prm.push_constant(cb);
			auto last_pl = pl;
			auto idx_off = 0;
			auto stencil_state = StencilStateNone;
			auto curr_pl = pl;
			auto curr_pl_sdf = pl_sdf;
			for (auto& cmd : draw_cmds)
			{
				switch (cmd.type)
				{
				case DrawCmd::SetTranslate:
					prm.pc.mark_dirty_c("translate"_h).as<vec2>() = cmd.data.translate;
					prm.push_constant(cb);
					break;
				case DrawCmd::SetScissor:
					if (!(cmd.data.rect == vp))
					{
						vp = cmd.data.rect;
						cb->set_scissor(vp);
					}
					break;
				case DrawCmd::SetStencilState:
					if (cmd.data.stencil_state != stencil_state)
					{
						stencil_state = cmd.data.stencil_state;
						switch (stencil_state)
						{
						case StencilStateNone:
							curr_pl = pl;
							curr_pl_sdf = pl_sdf;
							break;
						case StencilStateWrite:
							curr_pl = pl_stencil_write;
							curr_pl_sdf = pl_sdf_stencil_write;
							break;
						case StencilStateCompare:
							curr_pl = pl_stencil_compare;
							curr_pl_sdf = pl_sdf_stencil_compare;
							break;
						}
					}
					break;
				case DrawCmd::Blit:
					if (cmd.idx_cnt > 0)
					{
						if (last_pl != curr_pl)
						{
							cb->bind_pipeline(curr_pl);
							last_pl = curr_pl;
						}

						cb->bind_descriptor_set(0, cmd.ds);
						cb->draw_indexed(cmd.idx_cnt, idx_off, 0, 1, 0);
						idx_off += cmd.idx_cnt;
					}
					break;
				case DrawCmd::DrawSdf:
					if (cmd.idx_cnt > 0)
					{
						if (last_pl != curr_pl_sdf)
						{
							cb->bind_pipeline(curr_pl_sdf);
							last_pl = curr_pl_sdf;
						}
						prm.pc.mark_dirty_c("data"_h).as<vec4>() = vec4(1.f / cmd.data.sdf.scale / 4.f, cmd.data.sdf.thickness, cmd.data.sdf.border, 0.f);
						prm.push_constant(cb);

						cb->bind_descriptor_set(0, cmd.ds);
						cb->draw_indexed(cmd.idx_cnt, idx_off, 0, 1, 0);
						idx_off += cmd.idx_cnt;
					}
					break;
				}
			}
			cb->end_renderpass();
			cb->end_debug_label();

			reset_drawing();
		}

		struct CanvasCreate : Canvas::Create
		{
			CanvasPtr operator()(bool hdr) override
			{
				return new CanvasPrivate(hdr);
			}
		}Canvas_create;
		Canvas::Create& Canvas::create = Canvas_create;
	}
}
