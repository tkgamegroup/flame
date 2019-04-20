// MIT License
// 
// Copyright (c) 2019 wjs
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
#include <flame/file.h>
#include <flame/system.h>
#include <flame/surface.h>
#include <flame/img.h>
#include <flame/math.h>
#include <flame/model/model.h>
#include <flame/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/sampler.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/renderpath.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/graphics/camera.h>
#include <flame/graphics/staging_buffer.h>
#include <flame/graphics/model_manager.h>
#include <flame/physics/device.h>
#include <flame/physics/material.h>
#include <flame/physics/scene.h>
#include <flame/physics/rigid.h>
#include <flame/physics/shape.h>
#include <flame/UI/UI.h>
#include <flame/UI/instance.h>
#include <flame/UI/widgets/text.h>

#include <algorithm>

using namespace flame;

int main(int argc, char **args)
{
	Ivec2 res(800, 600);

	graphics::Camera camera((float)res.x / res.y);
	camera.update_proj();
	camera.calc_dir();
	camera.update_view();

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res, SurfaceStyleFrame, "Shooting");

	auto d = graphics::create_device(true);

	auto sc = graphics::create_swapchain(d, s);

	//graphics::Renderpath *render_path;
	//{
	//	auto fn = "forward.rp";
	//	if (!std::filesystem::exists(fn))
	//		return 0;

	//	auto bps = blueprint::create_scene();
	//	bps->register_from_path("blueprint_nodes/graphics");
	//	bps->load(fn);
	//	render_path = graphics::create_renderpath(d, bps);
	//}
	//auto img_dst = render_path->get_image_by_tag("dst");
	//auto rp = render_path->get_renderpass_by_tag("rp");
	//auto fb = render_path->get_framebuffer_by_tag("rp");
	//auto p = render_path->get_pipeline_by_tag("pl");
	//auto ds = render_path->get_descriptorset_by_tag("ds");

	//graphics::ModelManager mm(d);
	//mm.add_model("models/shooting_machine.blend");

	//auto m_machine = mm.get_model("models/shooting_machine.blend");

	//auto sampler = graphics::create_sampler(d, graphics::FilterLinear, graphics::FilterLinear,
	//	false);

	//auto cb = d->gcp->create_commandbuffer();
	//cb->begin();
	//cb->end();

	//auto p_d = physics::create_device();
	//auto material = physics::create_material(p_d, 0.f, 0.f, 0.f);
	//auto scene = physics::create_scene(p_d, -0.98f/*0.f*/, 1);
	//auto basket_trriger_s = physics::create_box_shape(p_d, material, Vec3(0.f), 0.75f, 0.1f, 0.75f);
	//basket_trriger_s->set_trigger(true);
	//auto basket_trriger_r = physics::create_static_rigid(p_d, Vec3(0.f, -3.f, 0.f));
	//basket_trriger_r->attach_shape(basket_trriger_s);
	//scene->add_rigid(basket_trriger_r);

	//auto update_main_cmd = [&]() {
	//	cb->begin();
	//	cb->begin_renderpass(rp, fb);
	//	cb->bind_pipeline(p);
	//	cb->bind_descriptorset(ds, 1);
	//	cb->bind_vertexbuffer(mm.vb);
	//	cb->bind_indexbuffer(mm.ib, graphics::IndiceTypeUint);
	//	cb->draw_indexed(m_machine->indice_count, 0, 0, 1, 0);
	//	cb->end_renderpass();
	//	cb->end();
	//};
	//update_main_cmd();

	/*
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

	auto score = 0;
	scene->enable_callback();
	scene->set_trigger_callback([&](physics::Rigid *r, physics::Shape *s,
		physics::Rigid *, physics::Shape *, physics::TouchType tt)
	{
		if (r == basket_trriger_r)
		{
			if (tt == physics::TouchFound)
				score++;
		}
	});

	//auto cb_update = d->gcp->create_commandbuffer();

	//s->add_keydown_listener([&](Surface *s, int vk){
	//	switch (vk)
	//	{
	//		case VK_F1:
	//			for (auto it = inses.begin(); it != inses.end(); it++)
	//			{
	//				if (it->dynamic)
	//					it->r->add_force(vec3(10.f, 0.f, 0.f));
	//			}
	//			break;
	//	}
	//});

	auto matrix_need_update = true;
	*/

	auto ui = UI::create_instance(d, sc->get_rp(true), s);
	//auto dst_img_ui_idx = ui->find_and_set_img_view(img_dst->get_view());

	auto image_avalible = graphics::create_semaphore(d);
	auto render_finished = graphics::create_semaphore(d);
	auto ui_finished = graphics::create_semaphore(d);

	auto t_fps = new UI::Text(ui);
	t_fps->pos = Vec2(0.f);
	ui->root()->add_widget(-1, t_fps, UI::LayoutAlignLittleEnd);

	sm->run([&](){
		//auto view_changed = false;

		//auto x_ang = 0.f;
		//auto y_ang = 0.f;
		//if (s->pressing_M(1))
		//{
		//	x_ang -= (s->mouse_disp.x) * 180.f / s->size.x;
		//	y_ang -= (s->mouse_disp.y) * 180.f / s->size.y;
		//	view_changed = true;
		//}
		//if (view_changed)
		//{
		//	camera.x_ang += x_ang;
		//	camera.y_ang += y_ang;
		//	camera.calc_dir();
		//	//camera.pos += z_sp * sp * camera.view_dir + x_sp * sp * camera.x_dir + y_sp * sp * camera.up_dir;
		//	camera.update_view();
		//	camera.update_frustum_plane();
		//	ubo_matrix.update(sizeof(Mat4), sizeof(Mat4), &camera.view);
		//}

		//auto t = get_now_ns();

		//static long long last_ns0 = 0;
		//if (t - last_ns0 >= 41666666)
		//{
		//	scene->update(1.f / 24);

		//	auto need_update_cmd = false;
		//	for (auto it = inses.begin(); it != inses.end();)
		//	{
		//		if (it->dynamic)
		//		{
		//			it->r->get_pose(it->coord, it->quat);
		//			if (it->coord.y < -4.f)
		//			{
		//				it->destroy();
		//				it = inses.erase(it);
		//				need_update_cmd = true;
		//				continue;
		//			}
		//		}
		//		it++;
		//	}
		//	if (need_update_cmd)
		//		update_main_cmd();

		//	last_ns0 = t;

		//	matrix_need_update = true;
		//}

		//if (!updates.empty())
		//{
		//	cb_update->begin(true);
		//	for (auto &u : updates)
		//		cb_update->copy_buffer(u.src, u.dst, u.ranges.size(), u.ranges.data());
		//	cb_update->end();
		//	d->q->submit(cb_update, nullptr, nullptr);
		//	d->q->wait_idle();
		//	updates.clear();
		//}

		auto index = sc->acquire_image(image_avalible);

		ui->begin(sm->elapsed_time);
		ui->end(sc->get_rp(true), sc->get_fb(index));

		d->gq->submit(ui->get_cb(), image_avalible, ui_finished);
		d->gq->wait_idle();
		d->gq->present(index, sc, ui_finished);

		static wchar_t buf[16];
		swprintf(buf, L"%lld", sm->fps);
		t_fps->set_text_and_size(buf);
	});

	return 0;
}
