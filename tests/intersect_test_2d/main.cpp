#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/application.h>
#include <flame/graphics/image.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/shader.h>

using namespace flame;
using namespace graphics;

auto pl_str = R"^^^(
layout
  @pll
shaders
  @vert
 ---
  @frag
renderpass
  {rp}

@pll
layout(push_constant) uniform PushConstant
{
	vec2 scale;
	vec2 translate;
}pc;
@

@vert
layout (location = 0) in vec2 i_pos;
layout (location = 1) in vec4 i_col;

layout (location = 0) out vec4 o_col;

void main()
{
	o_col = i_col;
	gl_Position = vec4(i_pos * pc.scale + pc.translate, 0, 1);
}
@

@frag
layout (location = 0) in vec4 i_col;

layout (location = 0) out vec4 o_col;

void main()
{
	o_col = i_col;
}
@
)^^^";

GraphicsApplication app;

GraphicsPipeline* pl;
VertexBuffer vtx_buf;

auto segment_start = vec2(70.f);
auto segment_end = vec2(170.f, 250.f);
auto circle_pos = vec2(50.f);
auto circle_radius = 20.f;
auto sector_pos = vec2(200.f);
auto sector_radius_start = 20.f;
auto sector_radius_end = 100.f;
auto sector_angle = 30.f;
auto sector_dir = 0.f;

int entry(int argc, char** args)
{
	app.create("Graphics Test", uvec2(640, 360), WindowFrame | WindowResizable, true);
	app.main_window->renderers.add([](int idx, CommandBufferPtr cb) {
		auto segment_circle_intersected = segment_circle_overlap(segment_start, segment_end, circle_pos, circle_radius);
		auto circle_sector_intersected = circle_sector_overlap(circle_pos, circle_radius, sector_pos, sector_radius_start, sector_radius_end, sector_angle, sector_dir);

		auto add_vtx = [](const vec2& vtx, const cvec4& col) {
			auto pv = vtx_buf.add();
			pv.item("i_pos"_h).set(vtx);
			pv.item("i_col"_h).set(col);
		};
		auto add_rect = [&](const vec2& vtx0, const vec2& vtx1, const vec2& vtx2, const vec2& vtx3, const cvec4& col) {
			add_vtx(vtx0, col);
			add_vtx(vtx1, col);
			add_vtx(vtx3, col);

			add_vtx(vtx3, col);
			add_vtx(vtx1, col);
			add_vtx(vtx2, col);
		};

		graphics::GuiCircleDrawer circle_draw(3);

		{
			auto color = (circle_sector_intersected) && false ? cvec4(255, 255, 0, 255) : cvec4(64, 128, 100, 255);
			int i_beg = circle_draw.get_idx(sector_dir - sector_angle);
			int i_end = circle_draw.get_idx(sector_dir + sector_angle);

			for (auto i = i_beg; i < i_end; i++)
			{
				auto circle_pt0 = circle_draw.get_pt(i);
				auto circle_pt1 = circle_draw.get_pt(i + 1);
				add_rect(sector_pos + circle_pt1 * sector_radius_start,
					sector_pos + circle_pt0 * sector_radius_start,
					sector_pos + circle_pt0 * sector_radius_end,
					sector_pos + circle_pt1 * sector_radius_end, color);
			}
		}
		{
			auto color = (segment_circle_intersected || circle_sector_intersected) && false ? cvec4(255, 255, 0, 255) : cvec4(128, 64, 100, 255);
			for (auto i = 0; i < circle_draw.pts.size(); i++)
			{
				add_vtx(circle_pos, color);
				add_vtx(circle_pos + circle_draw.get_pt(i) * circle_radius, color);
				add_vtx(circle_pos + circle_draw.get_pt(i + 1) * circle_radius, color);
			}
		}

		{
			auto color = (segment_circle_intersected) ? cvec4(255, 255, 0, 255) : cvec4(0, 0, 0, 255);
			auto n = vec2(normalize(cross(vec3(segment_end - segment_start, 0.f).xzy(), vec3(0.f, 1.f, 0.f))).xzy());
			add_rect(segment_start,
				segment_start + n * 1.f,
				segment_end + n * 1.f,
				segment_end, color);
		}

		auto n_vtx = vtx_buf.stag_top;

		vtx_buf.upload(cb);
		vtx_buf.buf_top = vtx_buf.stag_top = 0;
		auto vp = Rect(vec2(0), app.main_window->native->size);
		cb->set_viewport_and_scissor(vp);
		auto cv = vec4(0.9f, 0.9f, 0.98f, 1.f);
		cb->begin_renderpass(nullptr, app.main_window->swapchain->images[idx]->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), &cv);
		cb->bind_vertex_buffer(vtx_buf.buf.get(), 0);
		cb->bind_pipeline(pl);
		cb->push_constant_t(vec4(2.f / vec2(app.main_window->native->size), vec2(-1)));
		cb->draw(n_vtx, 1, 0, 0);
		cb->end_renderpass();
	}, 0, 0);
	graphics::gui_callbacks.add([]() {
		ImGui::DragFloat2("segment Start", &segment_start[0]);
		ImGui::DragFloat2("segment End", &segment_end[0]);
		ImGui::DragFloat2("Circle Pos", &circle_pos[0]);
		ImGui::DragFloat("Circle Radius", &circle_radius);
		ImGui::DragFloat2("Sector Pos", &sector_pos[0]);
		ImGui::DragFloat("Sector Radius Start", &sector_radius_start);
		ImGui::DragFloat("Sector Radius End", &sector_radius_end);
		ImGui::DragFloat("Sector Angle", &sector_angle);
		ImGui::DragFloat("Sector Dir", &sector_dir);
	});

	pl = GraphicsPipeline::create(pl_str, { "rp=" + str(Renderpass::get(L"flame\\shaders\\color.rp", { "col_fmt=" + TypeInfo::serialize_t(graphics::Swapchain::format) })) });
	vtx_buf.create(pl->vi_ui(), 10000);

	app.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
