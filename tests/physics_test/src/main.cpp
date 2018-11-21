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
#include <flame/filesystem.h>
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
#include <flame/physics/device.h>
#include <flame/physics/material.h>
#include <flame/physics/scene.h>
#include <flame/physics/rigid.h>
#include <flame/physics/shape.h>
#include <flame/UI/UI.h>

#include <algorithm>
#include <Windows.h>

int main(int argc, char **args)
{
	using namespace flame;
	using namespace glm;

	auto near_plane = 0.1f;
	auto far_plane = 1000.f;
	auto fovy = 60.f;

	vec2 res(1280, 720);

	auto aspect = (float)res.x / res.y;

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res.x, res.y, SurfaceStyleFrame,
		"Hello");

	auto d = graphics::create_device(false);

	auto sc = graphics::create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	struct UBO_matrix
	{
		mat4 proj;
		mat4 view;
	};

	auto ub_matrix = graphics::create_buffer(d, sizeof(UBO_matrix),  graphics::BufferUsageUniformBuffer, 
		graphics::MemPropHost | graphics::MemPropHostCoherent);
	ub_matrix->map();

	struct UBO_matrix_ins
	{
		mat4 model[65536];
	};

	auto ub_matrix_ins = graphics::create_buffer(d, sizeof(UBO_matrix_ins), graphics::BufferUsageTransferDst |
		graphics::BufferUsageStorageBuffer, graphics::MemPropDevice);

	auto ub_stag = graphics::create_buffer(d, ub_matrix_ins->size, graphics::BufferUsageTransferSrc,
		graphics::MemPropHost | graphics::MemPropHostCoherent);
	ub_stag->map();

	struct CopyBufferUpdate
	{
		graphics::Buffer *src;
		graphics::Buffer *dst;
		std::vector<graphics::BufferCopy> ranges;
	};

	std::vector<CopyBufferUpdate> updates;

	auto ubo_matrix = (UBO_matrix*)ub_matrix->mapped;
	ubo_matrix->proj = mat4(
		vec4(1.f, 0.f, 0.f, 0.f),
		vec4(0.f, -1.f, 0.f, 0.f),
		vec4(0.f, 0.f, 1.f, 0.f),
		vec4(0.f, 0.f, 0.f, 1.f)
	) * perspective(radians(fovy), aspect, near_plane, far_plane);

	auto ubo_matrix_ins = (UBO_matrix_ins*)ub_stag->mapped;

	ModelDescription desc;
	desc.set_to_default();
	auto m = create_cube_model(&desc, 0.5f);
	m->root_node->calc_global_matrix();

	auto depth_tex = graphics::create_texture(d, res.x, res.y, 1, 1, graphics::Format_Depth16,
		graphics::TextureUsageAttachment, graphics::MemPropDevice);
	auto depth_tex_view = graphics::create_textureview(d, depth_tex);

	auto rp = graphics::create_renderpass(d);
	rp->add_attachment(sc->format, true);
	rp->add_attachment(graphics::Format_Depth16, true);
	rp->add_subpass({0}, 1);
	rp->build();

	auto rp_ui = graphics::create_renderpass(d);
	rp_ui->add_attachment(sc->format, false);
	rp_ui->add_subpass({0}, -1);
	rp_ui->build();

	auto test_vert = graphics::create_shader(d, "test/test.vert");
	test_vert->build();
	auto test_frag = graphics::create_shader(d, "test/test.frag");
	test_frag->build();

	auto p = graphics::create_pipeline(d);
	p->set_renderpass(rp, 0);
	p->set_vertex_attributes({{
			graphics::VertexAttributeFloat3,
			//graphics::VertexAttributeFloat2,
			graphics::VertexAttributeFloat3
	}});
	p->set_size(res.x, res.y);
	p->set_depth_test(true);
	p->set_depth_write(true);
	p->add_shader(test_vert);
	p->add_shader(test_frag);
	p->build_graphics();

	auto ds = d->dp->create_descriptorset(p, 0);
	ds->set_uniformbuffer(0, 0, ub_matrix);
	ds->set_storagebuffer(1, 0, ub_matrix_ins);

	auto vb = graphics::create_buffer(d, m->vertex_count * m->vertex_buffers[0].size * sizeof(float), graphics::BufferUsageVertexBuffer |
		graphics::BufferUsageTransferDst, graphics::MemPropDevice);
	auto ib = create_buffer(d, m->indice_count * sizeof(int), graphics::BufferUsageIndexBuffer |
		graphics::BufferUsageTransferDst, graphics::MemPropDevice);
	auto sb = graphics::create_buffer(d, vb->size + ib->size, graphics::BufferUsageTransferSrc,
		graphics::MemPropHost | graphics::MemPropHostCoherent);
	sb->map();
	{
		auto c = d->cp->create_commandbuffer();
		c->begin(true);
		memcpy(sb->mapped, m->vertex_buffers[0].pVertex, vb->size);
		graphics::BufferCopy r1 = {0, 0, vb->size};
		c->copy_buffer(sb, vb, 1, &r1);
		memcpy((unsigned char*)sb->mapped + vb->size, m->pIndices, ib->size);
		graphics::BufferCopy r2 = {vb->size, 0, ib->size};
		c->copy_buffer(sb, ib, 1, &r2);
		c->end();
		d->q->submit(c, nullptr, nullptr);
		d->q->wait_idle();
		d->cp->destroy_commandbuffer(c);
	}
	sb->unmap();
	destroy_buffer(d, sb);

	auto sampler = graphics::create_sampler(d, graphics::FilterLinear, graphics::FilterLinear,
		false);

	graphics::Framebuffer *fbs[2];
	graphics::Framebuffer *fbs_ui[2];
	graphics::Commandbuffer *cbs[2];
	graphics::Commandbuffer *cbs_ui[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs[i] = create_framebuffer(d, res.x, res.y, rp);
		fbs[i]->set_view_swapchain(0, sc, i);
		fbs[i]->set_view(1, depth_tex_view);
		fbs[i]->build();

		fbs_ui[i] = create_framebuffer(d, res.x, res.y, rp_ui);
		fbs_ui[i]->set_view_swapchain(0, sc, i);
		fbs_ui[i]->build();

		cbs[i] = d->cp->create_commandbuffer();
		cbs_ui[i] = d->cp->create_commandbuffer();
		cbs_ui[i]->begin();
		cbs_ui[i]->end();
	}

	auto p_d = physics::create_device();
	auto material = physics::create_material(p_d, 0.f, 0.f, 0.f);
	auto scene = physics::create_scene(p_d, -0.98f/*0.f*/, 1);

	struct Ins
	{
		bool dynamic;
		vec3 coord;
		vec4 quat;
		vec3 size;
		physics::Rigid *r;
		physics::Shape *s;

		void create(physics::Device *d, bool _dynamic, physics::Material *m, physics::Scene *sc)
		{
			dynamic = _dynamic;
			if (dynamic)
				r = physics::create_dynamic_rigid(d, coord);
			else
				r = physics::create_static_rigid(d, coord);
			s = physics::create_box_shape(d, m, vec3(0.f), size.x, size.y, size.z);
			r->attach_shape(s);
			sc->add_rigid(r);
		}

		void destroy()
		{
			physics::destroy_shape(s);
			physics::destroy_rigid(r);
		}
	};

	std::vector<Ins> inses;

	auto update_main_cmd = [&](){
		for (auto i = 0; i < 2; i++)
		{
			cbs[i]->begin();
			cbs[i]->begin_renderpass(rp, fbs[i]);
			cbs[i]->bind_pipeline(p);
			cbs[i]->bind_descriptorset(ds);
			cbs[i]->bind_vertexbuffer(vb);
			cbs[i]->bind_indexbuffer(ib, graphics::IndiceTypeUint);
			for (auto j = 0; j < inses.size(); j++)
				cbs[i]->draw_indexed(m->indice_count, 0, 0, 1, j);
			cbs[i]->end_renderpass();
			cbs[i]->end();
		}
	};
	update_main_cmd();

	auto cb_update = d->cp->create_commandbuffer();

	auto image_avalible = graphics::create_semaphore(d);
	auto render_finished = graphics::create_semaphore(d);
	auto ui_finished = graphics::create_semaphore(d);

	auto ui = UI::create_instance(d, rp_ui);

	auto x_ang = 0.f;
	auto view_need_update = true;
	s->add_mousemove_listener([&](Surface *s, int, int){
		if (!ui->processed_mouse_input && (s->mouse_buttons[0] & KeyStateDown))
		{
			x_ang += s->mouse_disp_x;
			view_need_update = true;
		}
	});

	s->add_keydown_listener([&](Surface *s, int vk){
		switch (vk)
		{
			case VK_F1:
				for (auto it = inses.begin(); it != inses.end(); it++)
				{
					if (it->dynamic)
						it->r->add_force(vec3(10.f, 0.f, 0.f));
				}
				break;
		}
	});


	auto matrix_need_update = true;

	sm->run([&](){
		ui->begin(res.x, res.y, sm->elapsed_time, s->mouse_x, s->mouse_y,
			(s->mouse_buttons[0] & KeyStateDown) != 0,
			(s->mouse_buttons[1] & KeyStateDown) != 0,
			(s->mouse_buttons[2] & KeyStateDown) != 0,
			s->mouse_scroll);
		ui->begin_window("Control Panel");
		static vec3 coord = vec3(0.f);
		static vec3 size = vec3(0.5f);
		static bool dynamic = false;
		ui->dragfloat("x", &coord.x, 0.1f);
		ui->dragfloat("y", &coord.y, 0.1f);
		ui->dragfloat("z", &coord.z, 0.1f);
		ui->dragfloat("sx", &size.x, 0.1f);
		ui->dragfloat("sy", &size.y, 0.1f);
		ui->dragfloat("sz", &size.z, 0.1f);
		ui->checkbox("dynamic", &dynamic);
		if (ui->button("Create"))
		{
			Ins i;
			i.coord = coord;
			i.quat = vec4(0.f, 0.f, 0.f, 1.f);
			i.size = size;
			i.create(p_d, dynamic, material, scene);
			inses.push_back(i);

			update_main_cmd();
			matrix_need_update = true;
		}
		if (ui->button("Clear"))
		{
			for (auto &i : inses)
				i.destroy();
			inses.clear();

			update_main_cmd();
			matrix_need_update = true;
		}
		if (ui->button("Save Scene"))
		{
			std::ofstream out("physics test scene.txt");
			for (auto &i : inses)
			{
				write_fmt(out, "%d %f %f %f %f %f %f %f %f %f %f\n", (int)i.dynamic, i.coord.x, i.coord.y, i.coord.z,
					i.quat.x, i.quat.y, i.quat.z, i.quat.w,
					i.size.x, i.size.y, i.size.z);
			}
		}
		if (ui->button("Load Scene"))
		{
			std::ifstream in("physics test scene.txt");
			for (auto &i : inses)
				i.destroy();
			inses.clear();
			while (!in.eof())
			{
				std::string line;
				std::getline(in, line);
				Ins i;
				int dynamic;
				if (sscanf(line.c_str(), "%d %f %f %f %f %f %f %f %f %f %f", &dynamic, &i.coord.x, &i.coord.y, &i.coord.z,
					&i.quat.x, &i.quat.y, &i.quat.z, &i.quat.w,
					&i.size.x, &i.size.y, &i.size.z) >= 11)
				{
					i.dynamic = dynamic;
					i.create(p_d, i.dynamic, material, scene);
					inses.push_back(i);
				}
			}

			update_main_cmd();
			matrix_need_update = true;
		}
		ui->end_window();
		ui->end();

		for (auto i = 0; i < 2; i++)
		{
			cbs_ui[i]->begin();
			ui->record_commandbuffer(cbs_ui[i], rp_ui, fbs_ui[i]);
			cbs_ui[i]->end();
		}

		auto t = get_now_ns();

		static long long last_ns0 = 0;
		if (t - last_ns0 >= 41666666)
		{
			scene->update(1.f / 24);

			auto need_update_cmd = false;
			for (auto it = inses.begin(); it != inses.end();)
			{
				if (it->dynamic)
				{
					it->r->get_pose(it->coord, it->quat);
					if (it->coord.y < -4.f)
					{
						it->destroy();
						it = inses.erase(it);
						need_update_cmd = true;
						continue;
					}
				}
				it++;
			}
			if (need_update_cmd)
				update_main_cmd();

			last_ns0 = t;

			matrix_need_update = true;
		}

		if (matrix_need_update)
		{
			for (auto i = 0; i < inses.size(); i++)
			{
				ubo_matrix_ins->model[i] = translate(inses[i].coord) * mat4(quat_to_mat3(inses[i].quat))
					* scale(inses[i].size / vec3(0.5f));
			}

			CopyBufferUpdate upd;
			upd.src = ub_stag;
			upd.dst = ub_matrix_ins;
			upd.ranges.push_back({0, 0, int(inses.size() * sizeof(mat4))});
			updates.push_back(upd);

			matrix_need_update = false;
		}

		if (view_need_update)
		{
			ubo_matrix->view = lookAt(vec3(rotate(radians(-x_ang), vec3(0.f, 1.f, 0.f)) * vec4(0.f, 0.f, 10.f, 1.f)),
				vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));

			view_need_update = false;
		}

		if (!updates.empty())
		{
			cb_update->begin(true);
			for (auto &u : updates)
				cb_update->copy_buffer(u.src, u.dst, u.ranges.size(), u.ranges.data());
			cb_update->end();
			d->q->submit(cb_update, nullptr, nullptr);
			d->q->wait_idle();
			updates.clear();
		}

		auto index = sc->acquire_image(image_avalible);

		d->q->submit(cbs[index], image_avalible, render_finished);
		d->q->submit(cbs_ui[index], render_finished, ui_finished);
		d->q->wait_idle();
		d->q->present(index, sc, ui_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
