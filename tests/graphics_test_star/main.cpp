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
StorageBuffer<FLAME_UID, BufferUsageVertex> vtx_buf;

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
		vtx_buf.set_var<"i_pos"_h>(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col"_h>(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();

		rad = radians(a + 240.f);
		vtx_buf.set_var<"i_pos"_h>(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col"_h>(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();

		rad = radians(a + 120.f);
		vtx_buf.set_var<"i_pos"_h>(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col"_h>(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();

		rad = radians(a + 60.f);
		vtx_buf.set_var<"i_pos"_h>(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col"_h>(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();

		rad = radians(a + 300.f);
		vtx_buf.set_var<"i_pos"_h>(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col"_h>(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();

		rad = radians(a + 180.f);
		vtx_buf.set_var<"i_pos"_h>(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col"_h>(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();
	}
};

std::vector<Star> stars;

int entry(int argc, char** args)
{
	app.create(true, "Graphics Test", uvec2(500, 500));
	app.main_window->native->resize_listeners.add([](const uvec2& size) {
		projector.set(size, 45.f, 1.f, 4.f);
	});
	app.main_window->imgui_callbacks.add([]() {
		ImGui::Button("Hello");
	});
	app.main_window->renderers.add([](uint idx, CommandBufferPtr cb) {
		for (auto& s : stars)
		{
			s.move();
			s.draw();
		}

		vtx_buf.upload(cb);
		auto vp = Rect(vec2(0), app.main_window->native->size);
		cb->set_viewport(vp);
		cb->set_scissor(vp);
		auto cv = vec4(0.4f, 0.4f, 0.58f, 1.f);
		cb->begin_renderpass(nullptr, app.main_window->framebuffers[idx].get(), &cv);
		cb->bind_vertex_buffer(vtx_buf.buf.get(), 0);
		cb->bind_pipeline(pl);
		cb->push_constant_t(vec4(2.f / vec2(app.main_window->native->size), vec2(-1)));
		cb->draw(stars.size() * 6, 1, 0, 0);
		cb->end_renderpass();
	});

	stars.resize(1000);
	projector.set(app.main_window->native->size, 45.f, 1.f, 4.f);
	for (auto& s : stars)
		s.p.z = linearRand(0.f, 1.f) * (projector.zFar - projector.zNear) + projector.zNear;

	pl = GraphicsPipeline::create(pl_str, { "rp=" + str(app.main_window->renderpass_clear) });
	vtx_buf.create(pl->vi_ui(), stars.size() * 6);

	app.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
