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

auto line_segment_start = vec2(70.f);
auto line_segment_end = vec2(170.f, 250.f);
auto circle_pos = vec2(50.f);
auto circle_radius = 20.f;
auto sector_pos = vec2(200.f);
auto sector_radius_start = 20.f;
auto sector_radius_end = 100.f;
auto sector_angle = 30.f;
auto sector_dir = 0.f;

bool line_segment_circle_check(const vec2& p, const vec2& q, const vec2& o, float r)
{
	auto min_dist = std::numeric_limits<float>::max();
	auto pq = q - p; auto op = p - o; auto oq = q - o;
	if (dot(op, pq) < 0.f && dot(oq, pq) > 0.f)
		min_dist = length(cross(vec3(pq, 0.f), vec3(op, 0.f))) / length(pq);
	else
		min_dist = sqrt(min(dot(op, op), dot(oq, oq)));
	return min_dist < r;
}

bool circle_sector_check(const vec2& o, float r)
{
	return false;
}

int entry(int argc, char** args)
{
	app.create("Graphics Test", uvec2(640, 360), WindowFrame | WindowResizable, true);
	app.main_window->renderers.add([](uint idx, CommandBufferPtr cb) {
		auto line_segment_circle_intersected = line_segment_circle_check(line_segment_start, line_segment_end, circle_pos, circle_radius);

		static auto circle_pts = get_circle_points(3);
		auto n_circle_pts = (int)circle_pts.size();
		auto get_circle_pts_idx = [&](int i) {
			i = i % n_circle_pts;
			if (i < 0) i += n_circle_pts;
			return i;
		};
		for (auto i = 0; i < n_circle_pts; i++)
		{
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(circle_pos);
				pv.item("i_col"_h).set(cvec4(128, 64, 100, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(circle_pos + circle_pts[get_circle_pts_idx(i)] * circle_radius);
				pv.item("i_col"_h).set(cvec4(128, 64, 100, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(circle_pos + circle_pts[get_circle_pts_idx(i + 1)] * circle_radius);
				pv.item("i_col"_h).set(cvec4(128, 64, 100, 255));
			}
		}
		{
			auto ang0 = sector_dir - sector_angle;
			auto i_beg = int((ang0 / 360.f) * n_circle_pts);
			auto ang1 = sector_dir + sector_angle;
			auto i_end = int((ang1 / 360.f) * n_circle_pts);

			for (auto i = i_beg; i < i_end; i++)
			{
				{
					auto pv = vtx_buf.add();
					pv.item("i_pos"_h).set(sector_pos + circle_pts[get_circle_pts_idx(i)] * sector_radius_start);
					pv.item("i_col"_h).set(cvec4(64, 128, 100, 255));
				}
				{
					auto pv = vtx_buf.add();
					pv.item("i_pos"_h).set(sector_pos + circle_pts[get_circle_pts_idx(i)] * sector_radius_end);
					pv.item("i_col"_h).set(cvec4(64, 128, 100, 255));
				}
				{
					auto pv = vtx_buf.add();
					pv.item("i_pos"_h).set(sector_pos + circle_pts[get_circle_pts_idx(i + 1)] * sector_radius_end);
					pv.item("i_col"_h).set(cvec4(64, 128, 100, 255));
				}
				{
					auto pv = vtx_buf.add();
					pv.item("i_pos"_h).set(sector_pos + circle_pts[get_circle_pts_idx(i)] * sector_radius_start);
					pv.item("i_col"_h).set(cvec4(64, 128, 100, 255));
				}
				{
					auto pv = vtx_buf.add();
					pv.item("i_pos"_h).set(sector_pos + circle_pts[get_circle_pts_idx(i + 1)] * sector_radius_end);
					pv.item("i_col"_h).set(cvec4(64, 128, 100, 255));
				}
				{
					auto pv = vtx_buf.add();
					pv.item("i_pos"_h).set(sector_pos + circle_pts[get_circle_pts_idx(i + 1)] * sector_radius_start);
					pv.item("i_col"_h).set(cvec4(64, 128, 100, 255));
				}
			}
		}

		{
			auto n = vec2(normalize(cross(vec3(line_segment_end - line_segment_start, 0.f).xzy(), vec3(0.f, 1.f, 0.f))).xzy());
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(line_segment_start);
				pv.item("i_col"_h).set(line_segment_circle_intersected ? cvec4(255, 255, 255, 255) : cvec4(0, 0, 0, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(line_segment_start + n * 1.f);
				pv.item("i_col"_h).set(line_segment_circle_intersected ? cvec4(255, 255, 255, 255) : cvec4(0, 0, 0, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(line_segment_end);
				pv.item("i_col"_h).set(line_segment_circle_intersected ? cvec4(255, 255, 255, 255) : cvec4(0, 0, 0, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(line_segment_start + n * 1.f);
				pv.item("i_col"_h).set(line_segment_circle_intersected ? cvec4(255, 255, 255, 255) : cvec4(0, 0, 0, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(line_segment_end + n * 1.f);
				pv.item("i_col"_h).set(line_segment_circle_intersected ? cvec4(255, 255, 255, 255) : cvec4(0, 0, 0, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(line_segment_end);
				pv.item("i_col"_h).set(line_segment_circle_intersected ? cvec4(255, 255, 255, 255) : cvec4(0, 0, 0, 255));
			}
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
		ImGui::DragFloat2("line_segment Start", &line_segment_start[0]);
		ImGui::DragFloat2("line_segment End", &line_segment_end[0]);
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
