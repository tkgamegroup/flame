#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/image.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/application.h>

using namespace flame;
using namespace graphics;

struct App : GraphicsApplication 
{
	graphics::GraphicsPipelinePtr pl = nullptr;

	void on_render() override
	{
		//for (auto& s : stars)
		//{
		//	s.move();
		//	s.draw();
		//}

		//vtx_buf.upload(command_buffer);
		//vtx_buf.buf_top = vtx_buf.stag_top = 0;

		auto dst = main_window->swapchain->current_image();
		auto vp = Rect(vec2(0), (vec2)dst->extent);
		command_buffer->set_viewport_and_scissor(vp);
		auto cv = vec4(0.4f, 0.4f, 0.58f, 1.f);
		command_buffer->begin_renderpass(nullptr, dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), &cv);
		command_buffer->bind_pipeline(pl);
		command_buffer->draw(3, 1, 0, 0);
		//command_buffer->bind_vertex_buffer(vtx_buf.buf.get(), 0);
		//command_buffer->bind_pipeline(pl);
		//command_buffer->push_constant_t(vec4(2.f / vp.b, vec2(-1)));
		//command_buffer->draw(stars.size() * 6, 1, 0, 0);
		command_buffer->end_renderpass();
	}
}app;

auto pl_str = R"^^^(
layout
  @pll
shaders
  @vert
 ---
  @frag
renderpass
  {rp}
cull_mode
  None
depth_test
  false

@pll

@

@vert
void main()
{
	vec2 vs[] = {
		vec2(0.5, 0.0),
		vec2(0.0, 1.0),
		vec2(1.0, 0.5)
	};
	vec2 v = vs[gl_VertexIndex];
	gl_Position = vec4(v * 2.0 - 1.0, 1.0, 1.0);
}
@

@frag
layout (location = 0) out vec4 o_col;

void main()
{
	o_col = vec4(0.4, 0.7, 0.9, 1.0);
}
@
)^^^";

//auto pl_str = R"^^^(
//layout
//  @pll
//shaders
//  @vert
// ---
//  @frag
//renderpass
//  {rp}
//
//@pll
//layout(push_constant) uniform PushConstant
//{
//	vec2 scale;
//	vec2 translate;
//}pc;
//@
//
//@vert
//layout (location = 0) in vec2 i_pos;
//layout (location = 1) in vec4 i_col;
//
//layout (location = 0) out vec4 o_col;
//
//void main()
//{
//	o_col = i_col;
//	gl_Position = vec4(i_pos * pc.scale + pc.translate, 0, 1);
//}
//@
//
//@frag
//layout (location = 0) in vec4 i_col;
//
//layout (location = 0) out vec4 o_col;
//
//void main()
//{
//	o_col = i_col;
//}
//@
//)^^^";
//
//GraphicsApplication app;
//
//GraphicsPipeline* pl;
//VertexBuffer vtx_buf;
//
//PerspectiveProjector projector;
//
//struct Star
//{
//	vec3 p;
//	float a;
//
//	Star()
//	{
//		reset();
//	}
//
//	void reset()
//	{
//		p.x = linearRand(-4.f, 4.f);
//		p.y = linearRand(-4.f, 4.f);
//		p.z = projector.zFar;
//		a = linearRand(0.f, 360.f);
//	}
//
//	void move()
//	{
//		p.z -= 1.5f * delta_time;
//		if (p.z <= projector.zNear)
//			reset();
//	}
//
//	void draw()
//	{
//		auto r = 4.f / p.z;
//		auto c = projector.proj(p);
//
//		float rad;
//
//		rad = radians(a + 0.f);
//		{
//			auto vtx = vtx_buf.add();
//			vtx.child("i_pos"_h).as<vec2>() = vec2(c.x + cos(rad) * r, c.y + sin(rad) * r);
//			vtx.child("i_col"_h).as<cvec4>() = cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f));
//		}
//
//		rad = radians(a + 240.f);
//		{
//			auto vtx = vtx_buf.add();
//			vtx.child("i_pos"_h).as<vec2>() = vec2(c.x + cos(rad) * r, c.y + sin(rad) * r);
//			vtx.child("i_col"_h).as<cvec4>() = cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f));
//		}
//
//		rad = radians(a + 120.f);
//		{
//			auto vtx = vtx_buf.add();
//			vtx.child("i_pos"_h).as<vec2>() = vec2(c.x + cos(rad) * r, c.y + sin(rad) * r);
//			vtx.child("i_col"_h).as<cvec4>() = cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f));
//		}
//
//		rad = radians(a + 60.f);
//		{
//			auto vtx = vtx_buf.add();
//			vtx.child("i_pos"_h).as<vec2>() = vec2(c.x + cos(rad) * r, c.y + sin(rad) * r);
//			vtx.child("i_col"_h).as<cvec4>() = cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f));
//		}
//
//		rad = radians(a + 300.f);
//		{
//			auto vtx = vtx_buf.add();
//			vtx.child("i_pos"_h).as<vec2>() = vec2(c.x + cos(rad) * r, c.y + sin(rad) * r);
//			vtx.child("i_col"_h).as<cvec4>() = cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f));
//		}
//
//		rad = radians(a + 180.f);
//		{
//			auto vtx = vtx_buf.add();
//			vtx.child("i_pos"_h).as<vec2>() = vec2(c.x + cos(rad) * r, c.y + sin(rad) * r);
//			vtx.child("i_col"_h).as<cvec4>() = cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f));
//		}
//	}
//};
//
//std::vector<Star> stars;

int entry(int argc, char** args)
{
	app.create("Graphics Test", uvec2(500, 500), WindowStyleFrame, false, true);
	//app.main_window->native->resize_listeners.add([](const uvec2& size) {
	//	projector.set(size, 45.f, 1.f, 4.f);
	//});

	//stars.resize(1000);
	//projector.set(app.main_window->native->size, 45.f, 1.f, 4.f);
	//for (auto& s : stars)
	//	s.p.z = linearRand(0.f, 1.f) * (projector.zFar - projector.zNear) + projector.zNear;

	app.pl = GraphicsPipeline::create(pl_str, { "rp=" + str(Renderpass::get(L"flame\\shaders\\color.rp", { "col_fmt=R8G8B8A8_UNORM" })) });
	//vtx_buf.create(pl->vi_ui(), stars.size() * 6);

	app.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
