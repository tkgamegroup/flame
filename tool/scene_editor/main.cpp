#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/ui/utils.h>

#include "app.h"
#include "window/resource_explorer.h"

void MyApp::create()
{
	App::create("Editor", Vec2u(300, 200), WindowFrame | WindowResizable, true);

	TypeinfoDatabase::load(L"scene_editor.exe", true, true);

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

int main(int argc, char **args)
{
	app.create();

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
