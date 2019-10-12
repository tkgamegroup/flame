#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/universe/default_style.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/image.h>
#include <flame/universe/components/event_dispatcher.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/window.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

#include "app.h"
#include "window/resource_explorer.h"

void App::create()
{
	w = Window::create("Editor", Vec2u(300, 200), WindowFrame | WindowResizable);
	w->set_maximized(true);
	d = Device::create(true);
	render_finished = Semaphore::create(d);
	scr = SwapchainResizable::create(d, w);
	fence = Fence::create(d);
	sc_cbs.resize(scr->sc()->images().size());
	for (auto i = 0; i < sc_cbs.size(); i++)
		sc_cbs[i] = Commandbuffer::create(d->gcp);

	bp = BP::create_from_file(L"../renderpath/canvas_make_cmd/bp", true);
	bp->graphics_device = d;
	auto n_scr = bp->add_node(cH("graphics::SwapchainResizable"), "scr");
	n_scr->find_input("in")->set_data_p(scr);
	bp->find_input("*.rt_dst.type")->set_data_i(TargetImages);
	bp->find_input("*.rt_dst.v")->link_to(n_scr->find_output("images"));
	bp->find_input("*.make_cmd.cbs")->set_data_p(&sc_cbs);
	bp->find_input("*.make_cmd.image_idx")->link_to(n_scr->find_output("image_idx"));
	bp->update();
	canvas = (Canvas*)bp->find_output("*.make_cmd.canvas")->data_p();

	auto font14 = Font::create(L"c:/windows/fonts/msyh.ttc", 14);
	auto font_awesome14 = Font::create(L"../asset/font_awesome.ttf", 14);
	auto font32 = Font::create(L"c:/windows/fonts/msyh.ttc", 32);
	auto font_awesome32 = Font::create(L"../asset/font_awesome.ttf", 32);
	font_atlas_pixel = FontAtlas::create(d, FontDrawPixel, { font14, font_awesome14 });
	font_atlas_sdf = FontAtlas::create(d, FontDrawSdf, { font32, font_awesome32 });
	font_atlas_pixel->index = canvas->set_image(-1, font_atlas_pixel->imageview());
	font_atlas_sdf->index = canvas->set_image(-1, font_atlas_sdf->imageview());
	
	canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	default_style.set_to_light();

	universe_serialization_set_data("font_atlas1", font_atlas_pixel);

	root = Entity::create();
	{
		c_element_root = cElement::create(canvas);
		root->add_component(c_element_root);

		root->add_component(cEventDispatcher::create(w));

		root->add_component(cLayout::create(LayoutFree));
	}

	auto e_fps = Entity::create();
	root->add_child(e_fps);
	{
		e_fps->add_component(cElement::create());

		auto c_text = cText::create(font_atlas_pixel);
		c_text_fps = c_text;
		e_fps->add_component(c_text);

		auto c_aligner = cAligner::create();
		c_aligner->x_align = AlignxLeft;
		c_aligner->y_align = AlignyBottom;
		e_fps->add_component(c_aligner);
	}

	dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_foundation.typeinfo"));
	dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_graphics.typeinfo"));
	dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_universe.typeinfo"));

	open_resource_explorer(L"..", Vec2f(5, 724.f));
}

void App::run()
{
	auto sc = scr->sc();

	if (sc)
		sc->acquire_image();

	fence->wait();
	looper().process_delay_events();

	if (sc)
	{
		c_element_root->size = w->size;
		c_text_fps->set_text(std::to_wstring(looper().fps));
		root->update();
	}
	bp->update();

	if (sc)
	{
		std::vector<Commandbuffer*> cbs;
		cbs.push_back(sc_cbs[sc->image_index()]);
		cbs.insert(cbs.begin(), extra_cbs.begin(), extra_cbs.end());
		extra_cbs.clear();
		d->gq->submit(cbs, sc->image_avalible(), render_finished, fence);
		d->gq->present(sc, render_finished);
	}

	scr->signal = false;
}

App app;

Entity* create_drag_edit(FontAtlas* font_atlas, float sdf_scale, bool is_float)
{
	auto e_layout = Entity::create();
	{
		e_layout->add_component(cElement::create());

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->fence = 1;
		e_layout->add_component(c_layout);
	}

	auto e_edit = create_standard_edit(50.f, font_atlas, sdf_scale);
	e_layout->add_child(e_edit);
	e_edit->visible = false;

	auto e_drag = Entity::create();
	e_layout->add_child(e_drag);
	{
		auto c_element = cElement::create();
		c_element->inner_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
		c_element->size.x() = 58.f;
		e_drag->add_component(c_element);

		auto c_text = cText::create(font_atlas);
		c_text->sdf_scale = sdf_scale;
		c_text->auto_width = false;
		e_drag->add_component(c_text);

		e_drag->add_component(cEventReceiver::create());

		e_drag->add_component(cStyleColor::create(default_style.button_color_normal, default_style.button_color_hovering, default_style.button_color_active));
	}

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
	capture.e_t = (cText*)e_edit->find_component(cH("Text"));
	capture.e_e = (cEdit*)e_edit->find_component(cH("Edit"));
	capture.e_er = (cEventReceiver*)e_edit->find_component(cH("EventReceiver"));
	capture.d = e_drag;
	capture.d_er = (cEventReceiver*)e_drag->find_component(cH("EventReceiver"));
	capture.is_float = is_float;

	capture.e_er->add_focus_listener([](void* c, FocusType type) {
		auto& capture = *(Capture*)c;
		if (type == Focus_Lost)
		{
			capture.e->visible = false;
			capture.d->visible = true;
		}
	}, new_mail(&capture));

	capture.d_er->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
		auto& capture = *(Capture*)c;
		if (is_mouse_clicked(action, key) && pos == 0)
		{
			capture.e->visible = true;
			capture.d->visible = false;
			capture.d_er->event_dispatcher->next_focusing = capture.e_er;
		}
		else if (capture.d_er->active && is_mouse_move(action, key))
		{
			if (capture.is_float)
			{
				auto v = std::stof(capture.e_t->text());
				v += pos.x() * 0.05f;
				capture.e_t->set_text(std::to_wstring(v));
			}
			else
			{
				auto v = std::stoi(capture.e_t->text());
				v += pos.x();
				capture.e_t->set_text(std::to_wstring(v));
			}
			capture.e_e->on_changed();
		}
	}, new_mail(&capture));

	return e_layout;
}

void create_enum_combobox(EnumInfo* info, float width, FontAtlas* font_atlas, float sdf_scale, Entity* parent)
{
	std::vector<std::wstring> items;
	for (auto i = 0; i < info->item_count(); i++)
		items.push_back(s2w(info->item(i)->name()));

	parent->add_child(create_standard_combobox(120.f, font_atlas, sdf_scale, app.root, items));
}

void create_enum_checkboxs(EnumInfo* info, FontAtlas* font_atlas, float sdf_scale, Entity* parent)
{
	for (auto i = 0; i < info->item_count(); i++)
		parent->add_child(wrap_standard_text(create_standard_checkbox(), false, font_atlas, sdf_scale, s2w(info->item(i)->name())));
}

void popup_confirm_dialog(Entity* e, const std::wstring& title, void (*callback)(void* c, bool yes), const Mail<>& _capture)
{
	auto t = create_topmost(e, false, false, true, Vec4c(255, 255, 255, 235), true);
	{
		t->add_component(cLayout::create(LayoutFree));
	}

	auto e_dialog = Entity::create();
	t->add_child(e_dialog);
	{
		e_dialog->add_component(cElement::create());

		auto c_aligner = cAligner::create();
		c_aligner->x_align = AlignxMiddle;
		c_aligner->y_align = AlignyMiddle;
		e_dialog->add_component(c_aligner);

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->item_padding = 4.f;
		e_dialog->add_component(c_layout);
	}

	auto e_text = Entity::create();
	e_dialog->add_child(e_text);
	{
		e_text->add_component(cElement::create());

		auto c_text = cText::create(app.font_atlas_pixel);
		c_text->set_text(title);
		e_text->add_component(c_text);
	}

	auto e_buttons = Entity::create();
	e_dialog->add_child(e_buttons);
	{
		e_buttons->add_component(cElement::create());

		auto c_layout = cLayout::create(LayoutHorizontal);
		c_layout->item_padding = 4.f;
		e_buttons->add_component(c_layout);
	}

	struct Capture
	{
		Entity* e;
		void (*c)(void* c, bool yes);
		Mail<> m;
	}capture;
	capture.e = e;
	capture.c = callback;
	capture.m = _capture;

	auto e_yes = create_standard_button(app.font_atlas_pixel, 1.f, L"Yes");
	e_buttons->add_child(e_yes);
	{
		((cEventReceiver*)e_yes->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto& capture = *(Capture*)c;

			if (is_mouse_clicked(action, key))
			{
				destroy_topmost(capture.e, false);

				capture.c(capture.m.p, true);
				delete_mail(capture.m);
			}
		}, new_mail(&capture));
	}

	auto e_no = create_standard_button(app.font_atlas_pixel, 1.f, L"No");
	e_buttons->add_child(e_no);
	{
		((cEventReceiver*)e_no->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto& capture = *(Capture*)c;

			if (is_mouse_clicked(action, key))
			{
				destroy_topmost(capture.e, false);

				capture.c(capture.m.p, false);
				delete_mail(capture.m);
			}
		}, new_mail(&capture));
	}
}

void popup_input_dialog(Entity* e, const std::wstring& title, void (*callback)(void* c, bool ok, const std::wstring& text), const Mail<>& _capture)
{
	auto t = create_topmost(e, false, false, true, Vec4c(255, 255, 255, 235), true);
	{
		t->add_component(cLayout::create(LayoutFree));
	}

	auto e_dialog = Entity::create();
	t->add_child(e_dialog);
	{
		e_dialog->add_component(cElement::create());

		auto c_aligner = cAligner::create();
		c_aligner->x_align = AlignxMiddle;
		c_aligner->y_align = AlignyMiddle;
		e_dialog->add_component(c_aligner);

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->item_padding = 4.f;
		e_dialog->add_component(c_layout);
	}

	auto e_input = create_standard_edit(100.f, app.font_atlas_pixel, 1.f);
	e_dialog->add_child(wrap_standard_text(e_input, false, app.font_atlas_pixel, 1.f, title));

	auto e_buttons = Entity::create();
	e_dialog->add_child(e_buttons);
	{
		e_buttons->add_component(cElement::create());

		auto c_layout = cLayout::create(LayoutHorizontal);
		c_layout->item_padding = 4.f;
		e_buttons->add_component(c_layout);
	}

	struct Capture
	{
		Entity* e;
		void (*c)(void* c, bool ok, const std::wstring& text);
		Mail<> m;
		cText* t;
	}capture;
	capture.e = e;
	capture.c = callback;
	capture.m = _capture;
	capture.t = (cText*)e_input->find_component(cH("Text"));

	auto e_ok = create_standard_button(app.font_atlas_pixel, 1.f, L"Ok");
	e_buttons->add_child(e_ok);
	{
		((cEventReceiver*)e_ok->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto& capture = *(Capture*)c;

			if (is_mouse_clicked(action, key))
			{
				auto text = capture.t->text();
				destroy_topmost(capture.e, false);

				capture.c(capture.m.p, true, text);
				delete_mail(capture.m);
			}
		}, new_mail(&capture));
	}

	auto e_cancel = create_standard_button(app.font_atlas_pixel, 1.f, L"Cancel");
	e_buttons->add_child(e_cancel);
	{
		((cEventReceiver*)e_cancel->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto& capture = *(Capture*)c;

			if (is_mouse_clicked(action, key))
			{
				destroy_topmost(capture.e, false);

				capture.c(capture.m.p, false, L"");
				delete_mail(capture.m);
			}
		}, new_mail(&capture));
	}
}

extern "C" void mainCRTStartup();
static void* init_crt_ev = nullptr;
extern "C" __declspec(dllexport) void init_crt(void* ev)
{
	init_crt_ev = ev;
	mainCRTStartup();
}

int main(int argc, char **args)
{
	if (init_crt_ev)
	{
		set_event(init_crt_ev);
		for (;;)
			sleep(60000);
	}

	app.create();

	looper().loop([](void* c) {
		auto app = (*(App**)c);
		app->run();
	}, new_mail_p(&app));

	return 0;
}
