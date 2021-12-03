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

Device* d;
NativeWindow* nw;
Window* w;
Renderpass* rp;
std::vector<std::unique_ptr<Framebuffer>> fbs;
GraphicsPipeline* pl;
SequentialBuffer<vec2> vtx_buf;

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

struct PerspectiveProjector
{
	vec2 screen_sz;
	float aspect;
	float fovy;
	float zNear;
	float zFar;
	float tanfovy;

	mat4 matp;

	void set(const vec2& sz, float fov, float n, float f)
	{
		screen_sz = sz;
		aspect = screen_sz.x / screen_sz.y;
		fovy = fov;
		zNear = n;
		zFar = f;
		tanfovy = tan(radians(fovy));

		matp = perspective(fovy, aspect, zNear, zFar);
	}

	vec2 proj(const vec3& p)
	{
		auto pp = matp * vec4(p, 1.f);
		pp /= pp.w;
		return (pp.xy() * 0.5f + 0.5f) * screen_sz;
	}

}projector;

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
		p.x = linearRand(-1.f, 1.f) * projector.aspect;
		p.z = linearRand(projector.zNear, projector.zFar);
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
	//	canvas->begin_path();
	//	auto c1 = projector.project(p);
	//	auto c2 = projector.project(p - vec3(0.f, 0.1f, 0.f));
	//	canvas->move_to(c1.x, c1.y);
	//	canvas->line_to(c2.x, c2.y);
	//	canvas->stroke(cvec4(83, 209, 227, 255), 4.f / p.z);
	}
};
std::vector<Drop> drops;

int entry(int argc, char** args)
{
	d = Device::create(true);
	nw = NativeWindow::create("Graphics Test", uvec2(640, 360), WindowFrame);
	w = Window::create(d, nw);
	{
		Attachment att;
		att.format = Swapchain::format;
		Subpass sp;
		sp.color_attachments = { 0 };
		rp = Renderpass::create(d, { &att, 1 }, { &sp, 1 });
	}
	build_fbs();
	nw->add_resize_listener([](const uvec2&) {
		Queue::get(nullptr)->wait_idle();
		build_fbs();
	});
	w->add_renderer([](uint idx, CommandBufferPtr cb) {
		auto cv = vec4(0.9f, 0.9f, 0.98f, 1.f);
		cb->begin_renderpass(nullptr, fbs[idx].get(), &cv);
		for (auto& d : drops)
		{
			d.fall();
			d.draw();
		}
		cb->end_renderpass();
	});

	srand(time(0));
	projector.set(nw->size, 45.f, 1.f, 4.f);
	drops.resize(3000);

	pl = GraphicsPipeline::get(d, L"default_assets\\shaders\\plain\\line.pipeline");
	vtx_buf.create(BufferUsageVertex, drops.size() * 2);

	run([]() {
		w->dirty = true;
		w->update();
		return true;
	});

	return 0;
}

FLAME_EXE_MAIN(entry)
