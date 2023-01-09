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

auto circle_pos = vec2(0.f);
auto circle_radius = 20.f;
auto sector_pos = vec2(200.f);
auto sector_radius = 100.f;
auto sector_angle = 30.f;
auto sector_dir = 0.f;

int entry(int argc, char** args)
{
	app.create("Graphics Test", uvec2(640, 360), WindowFrame | WindowResizable, true);
	app.main_window->renderers.add([](uint idx, CommandBufferPtr cb) {
		static auto circle_pts = get_circle_points(3);
		auto n_circle_pts = (int)circle_pts.size();
		for (auto i = 0; i < n_circle_pts; i++)
		{
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(circle_pos);
				pv.item("i_col"_h).set(cvec4(128, 64, 100, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(circle_pos + circle_pts[i] * circle_radius);
				pv.item("i_col"_h).set(cvec4(128, 64, 100, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(circle_pos + circle_pts[i == n_circle_pts - 1 ? 0 : i + 1] * circle_radius);
				pv.item("i_col"_h).set(cvec4(128, 64, 100, 255));
			}
		}
		for (auto i = 0; i < n_circle_pts; i++)
		{
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(sector_pos);
				pv.item("i_col"_h).set(cvec4(128, 64, 100, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(sector_pos + circle_pts[i] * sector_radius);
				pv.item("i_col"_h).set(cvec4(128, 64, 100, 255));
			}
			{
				auto pv = vtx_buf.add();
				pv.item("i_pos"_h).set(sector_pos + circle_pts[i == n_circle_pts - 1 ? 0 : i + 1] * sector_radius);
				pv.item("i_col"_h).set(cvec4(128, 64, 100, 255));
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
	});
	graphics::gui_callbacks.add([]() {
		ImGui::Begin("ABC");
		ImGui::DragFloat2("Circle Pos", &circle_pos[0]);
		ImGui::End();
	});

	pl = GraphicsPipeline::create(pl_str, { "rp=" + str(Renderpass::get(L"flame\\shaders\\color.rp", { "col_fmt=" + TypeInfo::serialize_t(graphics::Swapchain::format) })) });
	vtx_buf.create(pl->vi_ui(), 1000);

	app.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
