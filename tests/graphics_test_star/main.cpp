#include <flame/foundation/window.h>
#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/buffer_ext.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/command.h>
#include <flame/graphics/window.h>
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

Device* d;
NativeWindow* nw;
Window* w;
Renderpass* rp;
std::vector<std::unique_ptr<Framebuffer>> fbs;
GraphicsPipeline* pl;
StorageBuffer<BufferUsageVertex> vtx_buf;

void build_fbs()
{
	auto sc = w->swapchain.get();
	fbs.resize(sc->images.size());
	for (auto i = 0; i < fbs.size(); i++)
	{
		auto iv = sc->images[i]->get_view();
		fbs[i].reset(Framebuffer::create(rp, { &iv, 1 }));
	}
}

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
		vtx_buf.set_var<"i_pos:0">(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col:1">(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();

		rad = radians(a + 240.f);
		vtx_buf.set_var<"i_pos:0">(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col:1">(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();

		rad = radians(a + 120.f);
		vtx_buf.set_var<"i_pos:0">(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col:1">(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();

		rad = radians(a + 60.f);
		vtx_buf.set_var<"i_pos:0">(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col:1">(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();

		rad = radians(a + 300.f);
		vtx_buf.set_var<"i_pos:0">(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col:1">(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();

		rad = radians(a + 180.f);
		vtx_buf.set_var<"i_pos:0">(vec2(c.x + cos(rad) * r, c.y + sin(rad) * r));
		vtx_buf.set_var<"i_col:1">(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		vtx_buf.next_item();
	}
};

std::vector<Star> stars;

int entry(int argc, char** args)
{
	d = Device::create(true);
	nw = NativeWindow::create("Graphics Test", uvec2(500, 500), WindowFrame);
	w = Window::create(d, nw);
	{
		RenderpassInfo info;
		auto& att = info.attachments.emplace_back();
		att.format = Swapchain::format;
		auto& sp = info.subpasses.emplace_back();
		sp.color_attachments = { 0 };
		rp = Renderpass::create(d, info);
	}
	build_fbs();
	nw->add_resize_listener([](const uvec2&) {
		Queue::get(nullptr)->wait_idle();
		build_fbs();
		});
	w->add_renderer([](uint idx, CommandBufferPtr cb) {
		for (auto& s : stars)
		{
			s.move();
			s.draw();
		}

		vtx_buf.upload(cb);
		auto vp = Rect(vec2(0), w->native->size);
		cb->set_viewport(vp);
		cb->set_scissor(vp);
		auto cv = vec4(0.4f, 0.4f, 0.58f, 1.f);
		cb->begin_renderpass(nullptr, fbs[idx].get(), &cv);
		cb->bind_vertex_buffer(vtx_buf.buf.get(), 0);
		cb->bind_pipeline(pl);
		cb->push_constant_t(vec4(2.f / vec2(w->native->size), vec2(-1)));
		cb->draw(stars.size() * 6, 1, 0, 0);
		cb->end_renderpass();
	});

	stars.resize(1000);
	projector.set(nw->size, 45.f, 1.f, 4.f);
	for (auto& s : stars)
		s.p.z = linearRand(0.f, 1.f) * (projector.zFar - projector.zNear) + projector.zNear;

	pl = GraphicsPipeline::create(d, pl_str, { "rp=0x" + to_string((uint64)rp) });
	vtx_buf.create(pl->info.shaders[0]->in_ui, stars.size() * 6);

	run([]() {
		w->dirty = true;
		w->update();
		return true;
	});

	return 0;
}

FLAME_EXE_MAIN(entry)
