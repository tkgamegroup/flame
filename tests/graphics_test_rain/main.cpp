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

template<unsigned N>
struct fixed_string 
{
    char buf[N + 1] {};

    constexpr fixed_string(const char (&str)[N])
    {
        for (unsigned i = 0; i != N; i++) 
            buf[i] = str[i];
    }

    constexpr operator char const* () const { return buf; }
};

struct MyVertexBuffer
{
	UdtInfo* ui = nullptr;

	uint size = 0;
	uint array_capacity = 0;
	uint array_count = 0;

	std::unique_ptr<BufferT> buf;
	std::unique_ptr<BufferT> stagbuf;
	char* data;
};

struct LineVB : MyVertexBuffer
{
	void create(UdtInfo* _ui, uint n)
	{
		ui = _ui;
		size = ui->size;
		array_capacity = n;
		buf.reset(Buffer::create(nullptr,  array_capacity * size, BufferUsageTransferDst | BufferUsageVertex, MemoryPropertyDevice));
		stagbuf.reset(Buffer::create(nullptr, buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
		stagbuf->map();
		data = (char*)stagbuf->mapped;
	}

	inline void* add_vtx()
	{
		auto ret = data;
		array_count++;
		data += size;
		return ret;
	}

	inline void upload(CommandBufferPtr cb)
	{
		BufferCopy cpy;
		cpy.size = array_count * size;
		array_count = 0;
		data = (char*)stagbuf->mapped;
		cb->copy_buffer(stagbuf.get(), buf.get(), { &cpy, 1 });
		cb->buffer_barrier(buf.get(), AccessTransferWrite, AccessVertexAttributeRead, PipelineStageTransfer, PipelineStageVertexInput);
	}

	template<fixed_string n, class T>
	inline void set_var(void* p, const T& v)
	{
		auto get_offset = [&]()->int {
			auto vi = ui->find_variable((char const*)n);
			if (!vi)
				return -1;
			return vi->offset;
		};
		static int offset = get_offset();
		if (offset == -1)
		{
			assert(0);
			return;
		}
		*(T*)((char*)p + offset) = v;
	}
};

Device* d;
NativeWindow* nw;
Window* w;
Renderpass* rp;
std::vector<std::unique_ptr<Framebuffer>> fbs;
GraphicsPipeline* pl;
LineVB vtx_buf;

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
		for (auto& d : drops)
		{
			d.fall();
			d.draw();
		}
		{
			auto p = vtx_buf.add_vtx();
			vtx_buf.set_var<"i_pos:0">(p, vec3(0, 0, 0));
			vtx_buf.set_var<"i_col:1">(p, cvec4(0, 0, 0, 255));
		}
		{
			auto p = vtx_buf.add_vtx();
			vtx_buf.set_var<"i_pos:0">(p, vec3(1, 1, 0));
			vtx_buf.set_var<"i_col:1">(p, cvec4(0, 0, 0, 255));
		}
		vtx_buf.upload(cb);
		auto vp = Rect(vec2(0), w->native->size);
		cb->set_viewport(vp);
		cb->set_scissor(vp);
		auto cv = vec4(0.9f, 0.9f, 0.98f, 1.f);
		cb->begin_renderpass(nullptr, fbs[idx].get(), &cv);
		cb->bind_vertex_buffer(vtx_buf.buf.get(), 0);
		cb->bind_pipeline(pl);
		cb->push_constant_t(mat4(1));
		cb->draw(2, 1, 0, 0);
		cb->end_renderpass();
	});

	srand(time(0));
	projector.set(nw->size, 45.f, 1.f, 4.f);
	drops.resize(3000);

	pl = GraphicsPipeline::get(d, L"default_assets\\shaders\\plain\\line.pipeline", format_defines(
		"rp=0x" + to_string((uint64)rp)));
	vtx_buf.create(pl->info.shaders[0]->in_ui, drops.size() * 2);

	run([]() {
		w->dirty = true;
		w->update();
		return true;
	});

	return 0;
}

FLAME_EXE_MAIN(entry)
