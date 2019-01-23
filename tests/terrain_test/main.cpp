// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/time.h>
#include <flame/system.h>
#include <flame/surface.h>
#include <flame/image.h>
#include <flame/math.h>
#include <flame/model/model.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/sampler.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>

#include <algorithm>

int main(int argc, char **args)
{
	using namespace flame;
	using namespace graphics;
	using namespace glm;

	auto near_plane = 0.1f;
	auto far_plane = 1000.f;
	auto fovy = 60.f;

	vec2 res(1280, 720);

	auto aspect = (float)res.x / res.y;

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res.x, res.y, SurfaceStyleFrame,
		"Hello");

	auto d = create_device(true);

	auto sc = create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto q = create_queue(d);
	auto cp = create_commandpool(d);

	struct UBO_terrain
	{
		vec3 coord;
		float dummy0;
		ivec2 count;
		float size;
		float height;
		vec2 resolution;
		float tessellation_factor;
		float dummy1;
		mat4 view_matrix;
		mat4 proj_matrix;
		vec4 frustum_planes[6];
	};

	auto ub_terrain = create_buffer(d, sizeof(UBO_terrain), BufferUsageUniformBuffer,
		MemPropHost | MemPropHostCoherent);
	ub_terrain->map();

	auto ubo_terrain = (UBO_terrain*)ub_terrain->mapped;

	ubo_terrain->coord = vec3(-32.f, 0.f, -32.f);
	ubo_terrain->count = ivec2(64);
	ubo_terrain->size = 1.f;
	ubo_terrain->height = 1.f;
	ubo_terrain->resolution = vec2(1280.f, 720.f);
	ubo_terrain->tessellation_factor = 1.f;
	ubo_terrain->proj_matrix = mat4(
		vec4(1.f, 0.f, 0.f, 0.f),
		vec4(0.f, -1.f, 0.f, 0.f),
		vec4(0.f, 0.f, 1.f, 0.f),
		vec4(0.f, 0.f, 0.f, 1.f)
	) * perspective(radians(fovy), aspect, near_plane, far_plane);

	Format depth_format;
	depth_format.v = Format::Depth16;

	auto depth_tex = create_texture(d, res.x, res.y, 1, 1, depth_format, 
		TextureUsageAttachment, MemPropDevice);
	auto depth_tex_view = create_textureview(d, depth_tex);

	auto rp = create_renderpass(d);
	rp->add_attachment_swapchain(sc, true);
	rp->add_attachment(depth_format, true);
	rp->add_subpass({0}, 1);
	rp->build();

	auto terrain_vert = create_shader(d, "test/terrain.vert");
	terrain_vert->build();
	auto terrain_tesc = create_shader(d, "test/terrain.tesc");
	terrain_tesc->build();
	auto terrain_tese = create_shader(d, "test/terrain.tese");
	terrain_tese->build();
	auto terrain_geom = create_shader(d, "test/terrain.geom");
	terrain_geom->build();
	auto terrain_frag = create_shader(d, "test/terrain.frag");
	terrain_frag->build();
	auto terrain_mod_comp = create_shader(d, "test/terrain_mod.comp");
	terrain_mod_comp->build();

	auto pipeline = create_pipeline(d);
	pipeline->set_size(res.x, res.y);
	pipeline->set_renderpass(rp, 0);
	pipeline->set_patch_control_points(4);
	pipeline->set_depth_test(true);
	pipeline->set_depth_write(true);
	pipeline->set_cull_mode(CullModeFront);
	pipeline->set_primitive_topology(PrimitiveTopologyPatchList);
	pipeline->add_shader(terrain_vert);
	pipeline->add_shader(terrain_tesc);
	pipeline->add_shader(terrain_tese);
	pipeline->add_shader(terrain_geom);
	pipeline->add_shader(terrain_frag);
	pipeline->build_graphics();

	auto pipeline_terrain_mod = create_pipeline(d);
	pipeline_terrain_mod->add_shader(terrain_mod_comp);
	pipeline_terrain_mod->build_compute();

	auto dp = create_descriptorpool(d);

	auto ds = dp->create_descriptorset(pipeline, 0);
	ds->set_uniformbuffer(0, 0, ub_terrain);

	auto sampler = create_sampler(d, FilterLinear, FilterLinear,
		false);

	Format h_format;
	h_format.v = Format::R16;
	auto h_map = create_texture(d, 2048, 2048, 1, 1, h_format,
		TextureUsageShaderStorage | TextureUsageShaderSampled, MemPropDevice);
	auto h_map_view = create_textureview(d, h_map);
	ds->set_texture(1, 0, h_map_view, sampler);

	auto ds_mod = dp->create_descriptorset(pipeline_terrain_mod, 0);
	ds_mod->set_storagetexture(0, 0, h_map_view);

	Framebuffer *fbs[2];
	Commandbuffer *cbs[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs[i] = create_framebuffer(d, res.x, res.y, rp);
		fbs[i]->set_view_swapchain(0, sc, i);
		fbs[i]->set_view(1, depth_tex_view);
		fbs[i]->build();

		cbs[i] = cp->create_commandbuffer();
		cbs[i]->begin();
		cbs[i]->begin_renderpass(rp, fbs[i]);
		cbs[i]->bind_pipeline(pipeline);
		cbs[i]->bind_descriptorset(ds);
		cbs[i]->draw(4, ubo_terrain->count.x * ubo_terrain->count.y, 0);
		cbs[i]->end_renderpass();
		cbs[i]->end();
	}

	auto image_avalible = create_semaphore(d);
	auto render_finished = create_semaphore(d);

	auto cb_mod = cp->create_commandbuffer();
	cb_mod->begin(true);
	cb_mod->change_texture_layout(h_map, TextureLayoutUndefined, TextureLayoutShaderReadOnly);
	cb_mod->end();
	q->submit(cb_mod, nullptr, nullptr);
	q->wait_idle();

	auto x_ang = 0.f;
	auto view_changed = true;
	s->add_mousemove_listener([&](Surface *s, int, int){
		if (s->mouse_buttons[0] & KeyStateDown)
		{
			x_ang += s->mouse_disp_x;
			view_changed = true;
		}
	});

	struct PC
	{
		vec2 center;
		float size;
		float press;
	}pc;

	s->add_mousedown_listener([&](Surface *s, int, int x, int y){
		cb_mod->begin(true);
		cb_mod->change_texture_layout(h_map, TextureLayoutShaderReadOnly, TextureLayoutShaderStorage);
		cb_mod->bind_pipeline(pipeline_terrain_mod);
		cb_mod->bind_descriptorset(ds_mod);
		pc.center = vec2(x, y) * 0.5f;
		pc.size = 100.f;
		pc.press = 10.f;
		cb_mod->push_constant(ShaderComp, 0, sizeof(PC), &pc);
		cb_mod->dispatch(h_map->cx, h_map->cy, 1);
		cb_mod->change_texture_layout(h_map, TextureLayoutShaderStorage, TextureLayoutShaderReadOnly);
		cb_mod->end();
		q->submit(cb_mod, nullptr, nullptr);
		q->wait_idle();
	});


	sm->run([&](){
		if (view_changed)
		{
			ubo_terrain->view_matrix = lookAt(vec3(0.f, 50.f, 50.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)) *
				rotate(radians(x_ang), vec3(0.f, 1.f, 0.f));

			auto proj_view = ubo_terrain->proj_matrix * ubo_terrain->view_matrix;
			ubo_terrain->frustum_planes[0].x = proj_view[0].w + proj_view[0].x;
			ubo_terrain->frustum_planes[0].y = proj_view[1].w + proj_view[1].x;
			ubo_terrain->frustum_planes[0].z = proj_view[2].w + proj_view[2].x;
			ubo_terrain->frustum_planes[0].w = proj_view[3].w + proj_view[3].x;
			ubo_terrain->frustum_planes[1].x = proj_view[0].w - proj_view[0].x;
			ubo_terrain->frustum_planes[1].y = proj_view[1].w - proj_view[1].x;
			ubo_terrain->frustum_planes[1].z = proj_view[2].w - proj_view[2].x;
			ubo_terrain->frustum_planes[1].w = proj_view[3].w - proj_view[3].x;
			ubo_terrain->frustum_planes[2].x = proj_view[0].w + proj_view[0].y;
			ubo_terrain->frustum_planes[2].y = proj_view[1].w + proj_view[1].y;
			ubo_terrain->frustum_planes[2].z = proj_view[2].w + proj_view[2].y;
			ubo_terrain->frustum_planes[2].w = proj_view[3].w + proj_view[3].y;
			ubo_terrain->frustum_planes[3].x = proj_view[0].w - proj_view[0].y;
			ubo_terrain->frustum_planes[3].y = proj_view[1].w - proj_view[1].y;
			ubo_terrain->frustum_planes[3].z = proj_view[2].w - proj_view[2].y;
			ubo_terrain->frustum_planes[3].w = proj_view[3].w - proj_view[3].y;
			ubo_terrain->frustum_planes[4].x = proj_view[0].w + proj_view[0].z;
			ubo_terrain->frustum_planes[4].y = proj_view[1].w + proj_view[1].z;
			ubo_terrain->frustum_planes[4].z = proj_view[2].w + proj_view[2].z;
			ubo_terrain->frustum_planes[4].w = proj_view[3].w + proj_view[3].z;
			ubo_terrain->frustum_planes[5].x = proj_view[0].w - proj_view[0].z;
			ubo_terrain->frustum_planes[5].y = proj_view[1].w - proj_view[1].z;
			ubo_terrain->frustum_planes[5].z = proj_view[2].w - proj_view[2].z;
			ubo_terrain->frustum_planes[5].w = proj_view[3].w - proj_view[3].z;

			view_changed = false;
		}

		auto index = sc->acquire_image(image_avalible);
		q->submit(cbs[index], image_avalible, render_finished);
		q->present(index, sc, render_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
