#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/ui/utils.h>

#include "app.h"
#include "window/resource_explorer.h"

void MyApp::create()
{
	App::create("Editor", Vec2u(300, 200), WindowFrame | WindowResizable, true);

	TypeinfoDatabase::load(L"editor.exe", true, true);

	canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	ui::style_set_to_light();

	root = u->world(0)->root();

	ui::set_current_entity(root);
	ui::c_layout();

	ui::push_font_atlas(app.font_atlas_pixel);
	ui::set_current_root(root);
	ui::push_parent(root);

		ui::e_begin_layout(LayoutVertical, 0.f, false, false);
		ui::c_aligner(SizeFitParent, SizeFitParent);

			ui::e_begin_menu_bar();
				ui::e_begin_menubar_menu(L"Window");
					ui::e_menu_item(L"Resource Explorer", [](void* c) {
					}, new_mail_p(this));
					ui::e_menu_item(L"Inspector", [](void* c) {
					}, new_mail_p(this));
				ui::e_end_menubar_menu();
			ui::e_end_menu_bar();

			ui::e_begin_docker_static_container();
			ui::e_end_docker_static_container();

			ui::e_text(L"");
			add_fps_listener([](void* c, uint fps) {
				(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
			}, new_mail_p(ui::current_entity()->get_component(cText)));

		ui::e_end_layout();

	ui::pop_parent();

	open_resource_explorer(L"..", Vec2f(5, 724.f));
}

MyApp app;

Entity* create_drag_edit(bool is_float)
{
	auto e_layout = ui::e_begin_layout(LayoutVertical);
	e_layout->get_component(cLayout)->fence = 1;

		auto e_edit = ui::e_edit(50.f);
		e_edit->set_visibility(false);
		auto e_drag = ui::e_button(L"");
		e_drag->get_component(cElement)->size_.x() = 58.f;
		e_drag->get_component(cText)->auto_width_ = false;

	ui::e_end_layout();

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

	capture.d_er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
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

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
