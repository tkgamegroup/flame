#include <flame/surface.h>
#include <flame/system.h>
#include <flame/filesystem.h>
#include <flame/math.h>
#include <flame/string.h>
#include <flame/select.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/UI/instance.h>

using namespace flame;

const float BP_node_titlebar_height = 18.f;

bool running = false;

struct Particle
{
	Vec2 coord;
	Vec2 velocity;
	float remaining_time;
};

std::vector<std::unique_ptr<Particle>> particles;

auto need_update_ubo = true;

void add_particle(const Vec2 &coord)
{
	auto p = new Particle;
	p->coord = coord;
	particles.emplace_back(p);
	need_update_ubo = true;
}

bool scene_clear_outside_particles = true;
float scene_clear_outside_particles_range = 50.f;

int main(int argc, char **args)
{
	srand(time(0));

	Vec2 res(1280, 720);

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res.x, res.y, SurfaceStyleFrame,
		"Effect Editor");

	auto d = graphics::create_device(true);

	auto sc = graphics::create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	Vec2 effect_size(800, 600);

	auto t = graphics::create_texture(d, effect_size.x, effect_size.y, 1, 1,
		graphics::Format_R8G8B8A8_UNORM,
		graphics::TextureUsageAttachment | graphics::TextureUsageShaderSampled,
		graphics::MemPropDevice);
	auto tv = graphics::create_textureview(d, t);

	auto rp_rtt = graphics::create_renderpass(d);
	rp_rtt->add_attachment(t->format, true);
	rp_rtt->add_subpass({ 0 }, -1);
	rp_rtt->build();

	auto fb_rtt = graphics::create_framebuffer(d, effect_size.x, effect_size.y, rp_rtt);
	fb_rtt->set_view(0, tv);
	fb_rtt->build();

	auto particle_liquid_vert = graphics::create_shader(d, "fullscreen.vert");
	particle_liquid_vert->add_define("USE_UV");
	particle_liquid_vert->build();
	auto particle_liquid_frag = graphics::create_shader(d, "effect/particle_liquid.frag");
	particle_liquid_frag->build();

	auto pl_particle_liquid = graphics::create_pipeline(d);
	pl_particle_liquid->set_size(effect_size.x, effect_size.y);
	pl_particle_liquid->set_cull_mode(graphics::CullModeNone);
	pl_particle_liquid->add_shader(particle_liquid_vert);
	pl_particle_liquid->add_shader(particle_liquid_frag);
	pl_particle_liquid->set_renderpass(rp_rtt, 0);
	pl_particle_liquid->build_graphics();

	struct Particle_glsl
	{
		Vec4 coord;
	};

	struct UBO_Particle
	{
		Ivec4 data;
		Particle_glsl particles[256];
	};

	auto ub_particle_liquid = graphics::create_buffer(d, sizeof(UBO_Particle),
		graphics::BufferUsageUniformBuffer,
		graphics::MemPropHost | graphics::MemPropHostCoherent);
	ub_particle_liquid->map();
	auto ubo_particle_liquid = (UBO_Particle*)ub_particle_liquid->mapped;
	ubo_particle_liquid->data.x = 0;
	ubo_particle_liquid->data.y = effect_size.x;
	ubo_particle_liquid->data.z = effect_size.y;

	auto ds_particle_liquid = d->dp->create_descriptorset(pl_particle_liquid, 0);
	ds_particle_liquid->set_uniformbuffer(0, 0, ub_particle_liquid);

	auto cb_effect = d->cp->create_commandbuffer();
	cb_effect->begin();
	cb_effect->begin_renderpass(rp_rtt, fb_rtt);
	cb_effect->bind_pipeline(pl_particle_liquid);
	cb_effect->bind_descriptorset(ds_particle_liquid);
	cb_effect->draw(3, 1, 0);
	cb_effect->end_renderpass();
	cb_effect->end();

	auto rp_ui = graphics::create_renderpass(d);
	rp_ui->add_attachment(sc->format, true);
	rp_ui->add_subpass({ 0 }, -1);
	rp_ui->build();

	graphics::Framebuffer *fbs_ui[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs_ui[i] = create_framebuffer(d, res.x, res.y, rp_ui);
		fbs_ui[i]->set_view_swapchain(0, sc, i);
		fbs_ui[i]->build();
	}
	auto cb_ui = d->cp->create_commandbuffer();
	cb_ui->begin();
	cb_ui->end();

	auto image_avalible = graphics::create_semaphore(d);
	auto effect_finished = graphics::create_semaphore(d);
	auto ui_finished = graphics::create_semaphore(d);

	auto ui = UI::create_instance(d, rp_ui, s);
	ui->set_texture(1, tv);

	auto update_ubo = [&]() {
		ubo_particle_liquid->data.x = particles.size();

		for (auto i = 0; i < particles.size(); i++)
		{
			ubo_particle_liquid->particles[i].coord =
				Vec4(particles[i]->coord + effect_size / 2, 0.f, 0.f);
		}
	};

	auto hovering_slot = [](BP_Node *n, const Vec2 &p)->BP_Slot* {
		for (auto i = 0; i < 2; i++)
		{
			for (auto &s : n->slots[i])
			{
				if (p.x > s->pos.x - 4 && p.x < s->pos.x + 4 &&
					p.y > s->pos.y - 4 && p.y < s->pos.y + 4)
					return s.get();
			}
		}
		return nullptr;
	};

	struct BP_Node_Intervaler : BP_Node
	{
		float interval_time;

		float accumulated_time;
		bool signal;

		BP_Node_Intervaler()
		{
			rect = Rect(0, 0, 200, 60) + Vec2(10);

			auto sl_output = new BP_Slot;
			strcpy(sl_output->name.data, "Output");
			add_slot(1, sl_output);

			interval_time = 1.f;
			accumulated_time = 0.f;
			signal = false;
		}

		virtual const char*name() override
		{
			return "Intervaler";
		}

		virtual Vec4 color() override
		{
			return Vec4(0.5f, 0.5f, 1.f, 1.f);
		}

		virtual void show(UI::Instance *ui, const Vec2 &off) override
		{
			ui->push_item_width(100.f);
			ui->set_cursor_pos(rect.min + Vec2(8.f, BP_node_titlebar_height + 8.f) + off);
			ui->dragfloat("sec", &interval_time, 0.05f, 0.f, 10000.f);
			ui->pop_item_width();
		}

		virtual void solve(float elapsed_time) override
		{
			if (uptodate)
				return;

			signal = false;

			accumulated_time += elapsed_time;
			if (accumulated_time >= interval_time)
			{
				signal = true;
				accumulated_time = 0.f;
			}

			uptodate = true;
		}

		virtual bool get_bool() override
		{
			return signal;
		}
	};

	struct BP_Node_ParticleGenerator : BP_Node
	{
		Vec2 coord;

		BP_Node_ParticleGenerator()
		{
			rect = Rect(0, 0, 150, 80) + Vec2(10);

			auto sl_trigger = new BP_Slot;
			strcpy(sl_trigger->name.data, "Trigger");
			add_slot(0, sl_trigger);

			auto sl_coord = new BP_Slot;
			strcpy(sl_coord->name.data, "Coord");
			add_slot(0, sl_coord);

			coord = Vec2(0.f);
		}

		virtual const char*name() override
		{
			return "Particle Generator";
		}

		virtual Vec4 color() override
		{
			return Vec4(1.f, 0.5f, 0.5f, 1.f);
		}

		virtual void show(UI::Instance *ui, const Vec2 &off) override
		{

		}

		virtual void solve(float elapsed_time) override
		{
			if (uptodate)
				return;

			bool trigger = false;
			for (auto &s : slots[0][0]->links)
			{
				auto n = s->n;
				if (!n->uptodate)
					n->solve(elapsed_time);
				trigger |= n->get_bool();
			}

			coord = Vec2(0.f);
			for (auto &s : slots[0][1]->links)
			{
				auto n = s->n;
				if (!n->uptodate)
					n->solve(elapsed_time);
				coord = n->get_vec2();
			}

			if (trigger)
			{
				add_particle(coord);
				printf("ParticleGenerator: Triggered x:%f y:%f\n", coord.x, coord.y);
			}

			uptodate = true;
		}
	};

	std::vector<std::unique_ptr<BP_Node>> bp_nodes;

	enum SelType
	{
		SelPtc,
		SelBPn
	};

	Select sel;

	enum TabType
	{
		TabScene,
		TabBluePrint
	};

	auto curr_tab = TabScene;

	sm->run([&](){
		ui->begin(res.x, res.y, sm->elapsed_time);

		ui->begin_mainmenu();
		if (ui->begin_menu("File"))
		{
			if (ui->menuitem("New", "Ctrl+N"))
			{
			}
			if (ui->menuitem("Open", "Ctrl+O"))
			{
			}
			if (ui->menuitem("Save", "Ctrl+S"))
			{
			}
			ui->end_menu();
		}
		if (ui->begin_menu("Add"))
		{
			switch (curr_tab)
			{
			case TabScene:
				if (ui->menuitem("Particle"))
					add_particle(Vec2(0.f));
				break;
			case TabBluePrint:
				if (ui->menuitem("Make Vec2"))
				{
					auto n = new BP_Node_MakeVec2;
					bp_nodes.emplace_back(n);
				}
				if (ui->menuitem("Random Number"))
				{
					auto n = new BP_Node_RandomNumber;
					bp_nodes.emplace_back(n);
				}
				if (ui->menuitem("Intervaler"))
				{
					auto n = new BP_Node_Intervaler;
					bp_nodes.emplace_back(n);
				}
				if (ui->menuitem("Particle Generator"))
				{
					auto n = new BP_Node_ParticleGenerator;
					bp_nodes.emplace_back(n);
				}
				break;
			}
			ui->end_menu();
		}
		if (ui->begin_menu("Edit"))
		{
			if (ui->menuitem("Undo", "Ctrl+Z"))
				;
			if (ui->menuitem("Redo", "Ctrl+Y"))
				;
			if (ui->menuitem("Cut", "Ctrl+X"))
				;
			if (ui->menuitem("Copy", "Ctrl+C"))
				;
			if (ui->menuitem("Paste", "Ctrl+V"))
				;
			if (ui->menuitem("Delete", "Del"))
			{
				switch (curr_tab)
				{
				case TabScene:
					if (sel.type == SelPtc)
					{
						for (auto it = particles.begin(); it != particles.end(); it++)
						{
							if (it->get() == (Particle*)sel.v)
							{
								particles.erase(it);
								break;
							}
						}
						sel.reset();
						need_update_ubo = true;
					}
					break;
				case TabBluePrint:
					break;
				}
			}
			ui->end_menu();
		}
		if (ui->begin_menu("Setting"))
		{
			if (ui->menuitem("Scene Options"))
			{

			}

			ui->end_menu();
		}
		auto menu_rect = ui->get_curr_window_rect();
		ui->end_mainmenu();

		ui->begin_status_window();
		ui->text("Ready. (%d particles in scene)", particles.size());
		auto status_rect = ui->get_curr_window_rect();
		ui->end_window();

		Vec2 ws_pos(0.f, menu_rect.max.y);
		Vec2 ws_size(res.x, res.y -
			(menu_rect.max.y - menu_rect.min.y) -
			(status_rect.max.y - status_rect.min.y));
		ui->begin_plain_window("ws", ws_pos, ws_size);

		if (!running)
		{
			if (ui->button("Run"))
			{
				running = true;
				need_update_ubo = true;
			}
			ui->sameline();
		}
		if (running)
		{
			if (sel.type == SelPtc)
				sel.reset();
			if (ui->button("Pause"))
			{
				running = false;
				need_update_ubo = true;
			}
			ui->sameline();
		}
		if (ui->button("Reset"))
		{
			if (sel.type == SelPtc)
				sel.reset();
			particles.clear();
			need_update_ubo = true;
		}
		ui->separator();

		ui->begin_tabbar("tabbar");

		if (ui->tabitem("Scene"))
		{
			curr_tab = TabScene;

			ui->image(1, effect_size);
			auto img_rect = ui->get_last_item_rect();
			auto dl_ws = ui->get_curr_window_drawlist();

			auto mpos = Vec2(s->mouse_x, s->mouse_y);
			mpos -= effect_size / 2;
			mpos -= Vec2(img_rect.min.x, img_rect.min.y);

			if (ui->is_last_item_hovered() && s->just_down_M(0))
			{
				auto clicked_blank = true;
				for (int i = particles.size() - 1; i >= 0; i--)
				{
					Rect r(particles[i]->coord - Vec2(50), particles[i]->coord + Vec2(50));
					if (r.contains(mpos))
					{
						sel.set(SelPtc, particles[i].get());
						clicked_blank = false;
						break;
					}
				}
				if (clicked_blank)
					sel.reset();
			}

			if (sel.type == SelPtc)
			{
				auto sel_ptc = (Particle*)sel.v;
				if (ui->is_last_item_hovered() && (s->mouse_buttons[0] & KeyStateDown) != 0)
				{
					if (s->mouse_disp_x != 0 || s->mouse_disp_y != 0)
					{
						sel_ptc->coord.x += s->mouse_disp_x;
						sel_ptc->coord.y += s->mouse_disp_y;
						update_ubo();
					}
				}

				dl_ws.add_rect(Rect(sel_ptc->coord - Vec2(50), sel_ptc->coord + Vec2(50)) +
					effect_size / 2 + Vec2(img_rect.min.x, img_rect.min.y), Vec4(1, 1, 0, 1));
			}
		}

		if (ui->tabitem("Blue Print"))
		{
			curr_tab = TabBluePrint;

			ui->begin_child("bp", Vec2(800, 600), true);

			auto wnd_rect = ui->get_curr_window_rect();

			auto mpos = Vec2(s->mouse_x, s->mouse_y);
			mpos -= wnd_rect.min;

			auto dl = ui->get_curr_window_drawlist();

			static BP_Slot *dragging_slot = nullptr;
			auto node_selected_idx_in_this_frame = -1;

			auto node_num = 0;
			for (auto &n : bp_nodes)
			{
				auto pos = n->rect.min + wnd_rect.min;
				auto size = n->rect.max - n->rect.min;

				ui->push_ID((int)n.get());

				if (sel.v == n.get())
				{
					dl.add_rect(n->rect.get_expanded(4.f) + wnd_rect.min,
						Vec4(1.f, 1.f, 0.f, 1.f));
				}

				dl.add_rect_filled(Rect(pos, pos + Vec2(size.x, BP_node_titlebar_height)),
					n->color(), 4.f, true, true, false, false);
				dl.add_rect_filled(Rect(pos + Vec2(0.f, BP_node_titlebar_height), pos + size), 
					Vec4(0.3f, 0.3f, 0.3f, 1.f), 4.f,
					false, false);
				dl.add_text(pos + Vec2(6.f, 2.f), Vec4(0.f, 0.f, 0.f, 1.f), n->name());

				n->show(ui, wnd_rect.min);

				for (auto i = 0; i < 2; i++)
				{
					auto disp = (n->rect.height() - BP_node_titlebar_height) / (n->slots[i].size() + 1);
					auto y = disp;
					for (auto &s : n->slots[i])
					{
						s->pos = Vec2(s->io == 0 ? n->rect.min.x + 8.f : n->rect.max.x - 8.f,
							BP_node_titlebar_height + y + n->rect.min.y);
						dl.add_circle_filled(s->pos + wnd_rect.min,
							4, Vec4(1.f, 1.f, 0.f, 1.f));
						ui->set_cursor_pos(s->pos +
							Vec2(s->io == 1 ?  -8.f : 8.f, -7.f) + wnd_rect.min);
						if (s->io == 1)
							ui->text_unformatted_RA(s->name.data);
						else
							ui->text_unformatted(s->name.data);

						if (i == 1)
						{
							for (auto ss : s->links)
							{
								auto p1 = s->pos + wnd_rect.min;
								auto p2 = ss->pos + wnd_rect.min;
								
								dl.add_bezier(p1, p1 + Vec2(50.f, 0.f),
									p2 + Vec2(-50.f, 0.f), p2, Vec4(1.f), 3);
							}
						}

						y += disp;
					}
				}

				ui->set_cursor_pos(pos);
				ui->invisibleitem("node", size);

				auto active = ui->is_last_item_active();
				if (active)
				{
					sel.set(SelBPn, n.get());
					node_selected_idx_in_this_frame = node_num;

					auto slot = hovering_slot(n.get(), mpos);
					if (slot)
						dragging_slot = slot;

					if (!dragging_slot && (s->mouse_disp_x != 0 || s->mouse_disp_y != 0))
						n->rect += Vec2(s->mouse_disp_x, s->mouse_disp_y);
				}

				ui->pop_ID();

				node_num++;
			}

			if (s->just_down_M(0))
			{
				if (node_selected_idx_in_this_frame == -1)
					sel.reset();
				else
				{
					if (node_selected_idx_in_this_frame != bp_nodes.size() - 1)
						std::swap(bp_nodes[node_selected_idx_in_this_frame], bp_nodes[bp_nodes.size() - 1]);
				}
			}

			if (dragging_slot)
			{
				dl.add_circle(dragging_slot->pos + wnd_rect.min,
					6, Vec4(1.f, 1.f, 0.f, 1.f));
				if (s->pressing_M(0))
				{
					auto p1 = dragging_slot->pos + wnd_rect.min;
					auto p2 = mpos + wnd_rect.min;
					dl.add_bezier(p1, p1 + Vec2(50.f * (dragging_slot->io == 1 ? 1.f : -1.f), 0.f),
						p2, p2, Vec4(1.f), 3);
				}
				else
				{
					for (int i = bp_nodes.size() - 1; i >= 0; i--)
					{
						auto n = bp_nodes[i].get();
						auto s = hovering_slot(n, mpos);
						if (s && dragging_slot != s)
						{
							if (dragging_slot != s)
							{
								auto already_exist = false;
								for (auto ss : dragging_slot->links)
								{
									if (ss == s)
									{
										already_exist = true;
										break;
									}
								}
								if (!already_exist)
								{
									for (auto ss : s->links)
									{
										if (ss == dragging_slot)
										{
											already_exist = true;
											break;
										}
									}

									if (!already_exist)
									{
										dragging_slot->links.push_back(s);
										s->links.push_back(dragging_slot);
									}
								}
							}
						}
					}
					dragging_slot = nullptr;
				}
			}

			ui->end_child();
		}

		ui->end_tabbar();

		ui->end_window();

		ui->end();

		if (running)
		{
			for (auto &n : bp_nodes)
				n->uptodate = false;

			for (auto &n : bp_nodes)
				n->solve(sm->elapsed_time);
		}

		if (need_update_ubo)
		{
			update_ubo();
			need_update_ubo = false;
		}

		auto index = sc->acquire_image(image_avalible);

		cb_ui->begin();
		ui->record_commandbuffer(cb_ui, rp_ui, fbs_ui[index]);
		cb_ui->end();

		d->q->submit(cb_effect, image_avalible, effect_finished);
		d->q->submit(cb_ui, effect_finished, ui_finished);
		d->q->wait_idle();
		d->q->present(index, sc, ui_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
