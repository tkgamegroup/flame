#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/image.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/canvas.h>
#include <flame/graphics/application.h>

using namespace flame;
using namespace graphics;

struct App : GraphicsApplication 
{
	graphics::GraphicsPipelinePtr pl = nullptr;
	graphics::DescriptorSetPtr ds = nullptr;
	CanvasPtr canvas = nullptr;

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
		command_buffer->bind_descriptor_set(0, ds);
		command_buffer->push_constant_t(0.2f, 0);
		command_buffer->push_constant_t(0.3f, 4);
		command_buffer->push_constant_t(0.4f, 8);
		command_buffer->push_constant_t(1.0f, 12);
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
layout (set = 0, binding = 0) uniform sampler2D map;

layout(push_constant) uniform PushConstant
{
    float R;
	float G;
	float B;
	float A;
}pc;

layout (set = 0, binding = 0) uniform Camera
{
	float zNear;
	float zFar;

	mat4 view;
	mat4 proj;
}camera;

layout (set = 1, binding = 3) uniform sampler2D map2;
@

@vert

layout(location = 0) out vec2 o_uv;

void main()
{
	vec2 vs[] = {
		vec2(0.5, 0.0),
		vec2(0.0, 1.0),
		vec2(1.0, 0.5)
	};
	vec2 v = vs[gl_VertexIndex];
	o_uv = v;
	gl_Position = vec4(v * 2.0 - 1.0, 1.0, 1.0);
}
@

@frag
layout(location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_col;

void main()
{
	o_col = vec4(texture(map, i_uv).rgb, 1.0);
}
@
)^^^";

/*
cbuffer Camera : register(b0, space0)
{
	float camera_zNear : packoffset(c0);
	float camera_zFar : packoffset(c0.y);
	row_major float4x4 camera_view : packoffset(c1);
	row_major float4x4 camera_proj : packoffset(c5);
};

cbuffer SPIRV_CROSS_RootConstant_pc : register(b0, space15)
{
	float2 pc_translate : packoffset(c0);
	float2 pc_scale : packoffset(c0.z);
	float4 pc_data : packoffset(c1);
};
Texture2D<float4> sky_map : register(t1, space1);
SamplerState _sky_map_sampler : register(s1, space1);
Texture2D<float4> sky_map2 : register(t3, space1);
SamplerState _sky_map2_sampler : register(s3, space1);

static float4 o_col;

struct SPIRV_Cross_Output
{
	float4 o_col : SV_Target0;
};

void frag_main()
{
	o_col = float4(0.4000000059604644775390625f, 0.699999988079071044921875f, 0.89999997615814208984375f, 1.0f);
}

SPIRV_Cross_Output main()
{
	frag_main();
	SPIRV_Cross_Output stage_output;
	stage_output.o_col = o_col;
	return stage_output;
}

*/

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

	//app.canvas = Canvas::create();
	//app.canvas->bind_window(app.main_window);

	app.pl = GraphicsPipeline::create(pl_str, { "rp=" + str(Renderpass::get(L"flame\\shaders\\color.rp", { "col_fmt=R8G8B8A8_UNORM" })) });
	app.ds = DescriptorSet::create(nullptr, app.pl->layout->dsls.front());
	auto img = graphics::Image::get(L"flame/icon.png");
	app.ds->set_image_i(0, 0, img->get_view(), nullptr);
	app.ds->update();
	//vtx_buf.create(pl->vi_ui(), stars.size() * 6);

	app.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
