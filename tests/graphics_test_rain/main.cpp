﻿#include <flame/graphics/application.h>
#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/image.h>
#include <flame/graphics/buffer_ext.h>
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

struct Drop
{
	vec3 p;
	float sp;
	float end;

	Drop()
	{
		reset();
		p.y = linearRand(0.f, 1.f) * end * 2.f - end;
	}

	void reset()
	{
		p.z = linearRand(projector.zNear, projector.zFar);
		p.x = linearRand(-1.f, 1.f) * ((p.z + 1.f) / projector.tanfovy * 0.5f) * projector.aspect;
		end = -p.z * projector.tanfovy;
		p.y = -end + 0.1;
		sp = linearRand(0.f, 1.f);
	}

	void fall()
	{
		sp += 0.8f * delta_time;
		p.y -= sp * delta_time;
		if (p.y < end)
			reset();
	}

	void draw()
	{
		auto c1 = projector.proj(p + vec3(0.f, 0.05f, 0.f));
		auto c2 = projector.proj(p - vec3(0.f, 0.05f, 0.f));
		auto w = 2.f / p.z;

		vtx_buf.set_var<"i_pos"_h>(c1 - vec2(w, 0));
		vtx_buf.set_var<"i_col"_h>(cvec4(159, 183, 227, 255));
		vtx_buf.next_item();

		vtx_buf.set_var<"i_pos"_h>(c2 - vec2(w, 0));
		vtx_buf.set_var<"i_col"_h>(cvec4(159, 183, 227, 255));
		vtx_buf.next_item();

		vtx_buf.set_var<"i_pos"_h>(c1 + vec2(w, 0));
		vtx_buf.set_var<"i_col"_h>(cvec4(159, 183, 227, 255));
		vtx_buf.next_item();

		vtx_buf.set_var<"i_pos"_h>(c1 + vec2(w, 0));
		vtx_buf.set_var<"i_col"_h>(cvec4(159, 183, 227, 255));
		vtx_buf.next_item();

		vtx_buf.set_var<"i_pos"_h>(c2 - vec2(w, 0));
		vtx_buf.set_var<"i_col"_h>(cvec4(159, 183, 227, 255));
		vtx_buf.next_item();

		vtx_buf.set_var<"i_pos"_h>(c2 + vec2(w, 0));
		vtx_buf.set_var<"i_col"_h>(cvec4(159, 183, 227, 255));
		vtx_buf.next_item();
	}
};
std::vector<Drop> drops;

int entry(int argc, char** args)
{
	app.create(true, "Graphics Test", uvec2(640, 360));
	app.main_window->native->resize_listeners.add([](const uvec2& size) {
		projector.set(size, 45.f, 1.f, 4.f);
	});
	app.main_window->imgui_callbacks.add([]() {
		ImGui::Button("Hello");
	});
	app.main_window->renderers.add([](uint idx, CommandBufferPtr cb) {
		for (auto& d : drops)
		{
			d.fall();
			d.draw();
		}

		vtx_buf.upload(cb);
		auto vp = Rect(vec2(0), app.main_window->native->size);
		cb->set_viewport(vp);
		cb->set_scissor(vp);
		auto cv = vec4(0.9f, 0.9f, 0.98f, 1.f);
		cb->begin_renderpass(nullptr, app.main_window->framebuffers[idx].get(), &cv);
		cb->bind_vertex_buffer(vtx_buf.buf.get(), 0);
		cb->bind_pipeline(pl);
		cb->push_constant_t(vec4(2.f / vec2(app.main_window->native->size), vec2(-1)));
		cb->draw(drops.size() * 6, 1, 0, 0);
		cb->end_renderpass();
	});

	srand(time(0));
	projector.set(app.main_window->native->size, 45.f, 1.f, 4.f);
	drops.resize(3000);

	pl = GraphicsPipeline::create(app.graphics_device, pl_str, 
		{ "rp=0x" + to_string((uint64)app.main_window->renderpass_clear) });
	vtx_buf.create(pl->vi_ui(), drops.size() * 6);

	app.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
