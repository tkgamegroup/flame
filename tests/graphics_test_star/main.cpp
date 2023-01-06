#include <flame/graphics/application.h>
#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
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

PerspectiveProjector projector;

struct Star
{
	vec3 p;
	float a;

	Star()
	{
		reset();
	}

	void reset()
	{
		p.x = linearRand(-4.f, 4.f);
		p.y = linearRand(-4.f, 4.f);
		p.z = projector.zFar;
		a = linearRand(0.f, 360.f);
	}

	void move()
	{
		p.z -= 1.5f * delta_time;
		if (p.z <= projector.zNear)
			reset();
	}

	void draw()
	{
		auto r = 4.f / p.z;
		auto c = projector.proj(p);

		float rad;

		rad = radians(a + 0.f);
		{
			auto pv = vtx_buf.add();
			pv.item("i_pos"_h).set(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
			pv.item("i_col"_h).set(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		}

		rad = radians(a + 240.f);
		{
			auto pv = vtx_buf.add();
			pv.item("i_pos"_h).set(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
			pv.item("i_col"_h).set(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		}

		rad = radians(a + 120.f);
		{
			auto pv = vtx_buf.add();
			pv.item("i_pos"_h).set(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
			pv.item("i_col"_h).set(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		}

		rad = radians(a + 60.f);
		{
			auto pv = vtx_buf.add();
			pv.item("i_pos"_h).set(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
			pv.item("i_col"_h).set(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		}

		rad = radians(a + 300.f);
		{
			auto pv = vtx_buf.add();
			pv.item("i_pos"_h).set(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
			pv.item("i_col"_h).set(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		}

		rad = radians(a + 180.f);
		{
			auto pv = vtx_buf.add();
			pv.item("i_pos"_h).set(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
			pv.item("i_col"_h).set(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		}
	}
};

std::vector<Star> stars;

int entry(int argc, char** args)
{
	app.create("Graphics Test", uvec2(500, 500), WindowFrame | WindowResizable, true);
	app.main_window->native->resize_listeners.add([](const uvec2& size) {
		projector.set(size, 45.f, 1.f, 4.f);
	});
	app.main_window->renderers.add([](uint idx, CommandBufferPtr cb) {
		for (auto& s : stars)
		{
			s.move();
			s.draw();
		}

		vtx_buf.upload(cb);
		vtx_buf.buf_top = vtx_buf.stag_top = 0;
		auto vp = Rect(vec2(0), app.main_window->native->size);
		cb->set_viewport_and_scissor(vp);
		auto cv = vec4(0.4f, 0.4f, 0.58f, 1.f);
		cb->begin_renderpass(nullptr, app.main_window->swapchain->images[idx]->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), &cv);
		cb->bind_vertex_buffer(vtx_buf.buf.get(), 0);
		cb->bind_pipeline(pl);
		cb->push_constant_t(vec4(2.f / vec2(app.main_window->native->size), vec2(-1)));
		cb->draw(stars.size() * 6, 1, 0, 0);
		cb->end_renderpass();
	});
	graphics::gui_callbacks.add([]() {
		ImGui::Button("Hello");
	});

	stars.resize(1000);
	projector.set(app.main_window->native->size, 45.f, 1.f, 4.f);
	for (auto& s : stars)
		s.p.z = linearRand(0.f, 1.f) * (projector.zFar - projector.zNear) + projector.zNear;

	pl = GraphicsPipeline::create(pl_str, { "rp=" + str(Renderpass::get(L"flame\\shaders\\color.rp", { "col_fmt=" + TypeInfo::serialize_t(graphics::Swapchain::format) })) });
	vtx_buf.create(pl->vi_ui(), stars.size() * 6);

	app.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
