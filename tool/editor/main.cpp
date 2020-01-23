#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/ui/utils.h>

#include "../renderpath/canvas/canvas.h"

#include "app.h"
#include "window/resource_explorer.h"

void App::create()
{
	w = Window::create("Editor", Vec2u(300, 200), WindowFrame | WindowResizable);
	w->set_maximized(true);
	d = Device::create(true);
	scr = SwapchainResizable::create(d, w);
	fence = Fence::create(d);
	sc_cbs.resize(scr->sc()->image_count());
	for (auto i = 0; i < sc_cbs.s; i++)
		sc_cbs.v[i] = Commandbuffer::create(d->gcp);
	render_finished = Semaphore::create(d);

	wchar_t* fonts[] = { 
		L"c:/windows/fonts/msyh.ttc", 
		L"../art/font_awesome.ttf"
	};
	font_atlas_pixel = FontAtlas::create(d, FontDrawPixel, 2, fonts);
	ui::style_set_to_light();

	app.u = Universe::create();
	app.u->add_object(app.w);

	auto w = World::create(app.u);
	w->add_system(sLayoutManagement::create());
	w->add_system(sEventDispatcher::create());

	s_2d_renderer = s2DRenderer::create(L"../renderpath/canvas/bp", scr, FLAME_CHASH("SwapchainResizable"), &sc_cbs);
	w->add_system(s_2d_renderer);
	s_2d_renderer->canvas->add_font(app.font_atlas_pixel);
	s_2d_renderer->canvas->set_clear_color(Vec4c(100, 100, 100, 255));

	root = w->root();

	ui::set_current_entity(root);
	c_element_root = ui::c_element();
	ui::c_layout();

	ui::push_font_atlas(app.font_atlas_pixel);
	ui::push_parent(root);
	ui::set_current_root(root);

	ui::e_begin_menu_bar();
	ui::e_begin_menu_top(L"Window");
	ui::e_menu_item(L"Resource Explorer", [](void* c, Entity*) {
	}, new_mail_p(this));
	ui::e_end_menu_bar();
	ui::e_end_menu_bar();

	ui::e_text(L"");
	ui::c_aligner(AlignxLeft, AlignyBottom);
	add_fps_listener([](void* c, uint fps) {
		(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
	}, new_mail_p(ui::current_entity()->get_component(cText)));

	ui::pop_parent();

	dbs.push_back(TypeinfoDatabase::load(dbs.size(), dbs.data(), L"flame_foundation.typeinfo"));
	dbs.push_back(TypeinfoDatabase::load(dbs.size(), dbs.data(), L"flame_graphics.typeinfo"));
	dbs.push_back(TypeinfoDatabase::load(dbs.size(), dbs.data(), L"flame_universe.typeinfo"));

	open_resource_explorer(L"..", Vec2f(5, 724.f));
}

void App::run()
{
	auto sc = scr->sc();

	if (sc)
		sc->acquire_image();

	fence->wait();
	looper().process_events();

	c_element_root->set_size(Vec2f(w->size));
	u->update();

	if (sc)
	{
		std::vector<Commandbuffer*> cbs;
		cbs.push_back(sc_cbs.v[sc->image_index()]);
		cbs.insert(cbs.begin(), extra_cbs.begin(), extra_cbs.end());
		extra_cbs.clear();
		d->gq->submit(cbs.size(), cbs.data(), sc->image_avalible(), render_finished, fence);
		d->gq->present(sc, render_finished);
	}
}

App app;

Entity* create_drag_edit(bool is_float)
{
	auto e_layout = ui::e_begin_layout(Vec2f(0.f), LayoutVertical);
	e_layout->get_component(cLayout)->fence = 1;

	auto e_edit = ui::e_edit(50.f);
	e_edit->set_visibility(false);
	auto e_drag = ui::e_button(L"");
	e_drag->get_component(cElement)->size_.x() = 58.f;
	e_drag->get_component(cText)->auto_width_ = false;

	struct Capture
	{
		Entity* e;
		cText* e_t;
		cEdit* e_e;
		cEventReceiver* e_er;
		Entity* d;
		cEventReceiver* d_er;
		bool is_float;
	}capture;
	capture.e = e_edit;
	capture.e_t = e_edit->get_component(cText);
	capture.e_e = e_edit->get_component(cEdit);
	capture.e_er = e_edit->get_component(cEventReceiver);
	capture.d = e_drag;
	capture.d_er = e_drag->get_component(cEventReceiver);
	capture.is_float = is_float;

	capture.e_er->data_changed_listeners.add([](void* c, Component* er, uint hash, void*) {
		auto& capture = *(Capture*)c;
		if (hash == FLAME_CHASH("focusing") && ((cEventReceiver*)er)->focusing == false)
		{
			capture.e->set_visibility(false);
			capture.d->set_visibility(true);
		}
	}, new_mail(&capture));

	capture.d_er->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
		auto& capture = *(Capture*)c;
		if (is_mouse_clicked(action, key) && pos == 0)
		{
			capture.e->set_visibility(true);
			capture.d->set_visibility(false);
			auto dp = capture.d_er->dispatcher;
			dp->next_focusing = capture.e_er;
			dp->pending_update = true;
		}
		else if (capture.d_er->active && is_mouse_move(action, key))
		{
			if (capture.is_float)
			{
				auto v = std::stof(capture.e_t->text());
				v += pos.x() * 0.05f;
				capture.e_t->set_text(to_wstring(v, 2).c_str());
			}
			else
			{
				auto v = std::stoi(capture.e_t->text());
				v += pos.x();
				capture.e_t->set_text(std::to_wstring(v).c_str());
			}
		}
	}, new_mail(&capture));

	ui::e_end_layout();

	return e_layout;
}

void create_enum_combobox(EnumInfo* info, float width)
{
	ui::e_begin_combobox(120.f);
	for (auto i = 0; i < info->item_count(); i++)
		ui::e_combobox_item(s2w(info->item(i)->name()).c_str());
	ui::e_end_combobox();
}

void create_enum_checkboxs(EnumInfo* info)
{
	for (auto i = 0; i < info->item_count(); i++)
		ui::e_checkbox(s2w(info->item(i)->name()).c_str());
}

int main(int argc, char **args)
{
	app.create();

	looper().loop([](void* c) {
		auto app = (*(App**)c);
		app->run();
	}, new_mail_p(&app));

	return 0;
}
