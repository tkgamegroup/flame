#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/ui/utils.h>

#include "app.h"

MyApp app;

void MyApp::create()
{
	App::create("Scene Editor", Vec2u(300, 200), WindowFrame | WindowResizable, true, getenv("FLAME_PATH"), true);

	TypeinfoDatabase::load(L"scene_editor.exe", true, true);

	canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	ui::style_set_to_light();

	ui::set_current_entity(root);
	ui::c_layout();

	ui::push_font_atlas(app.font_atlas_pixel);
	ui::set_current_root(root);
	ui::push_parent(root);

	ui::e_begin_layout(LayoutVertical, 0.f, false, false);
	ui::c_aligner(SizeFitParent, SizeFitParent);

	ui::e_begin_menu_bar();
		ui::e_begin_menubar_menu(L"Scene");
			ui::e_menu_item(L"New Entity", [](void* c) {
				looper().add_event([](void* c, bool*) {
					auto e = Entity::create();
					e->set_name("unnamed");
					if (app.selected)
						app.selected->add_child(e);
					else
						app.prefab->add_child(e);
					if (app.hierarchy)
						app.hierarchy->refresh();
				}, Mail<>());
			}, Mail<>());
			ui::e_menu_item(L"Save", [](void* c) {

			}, Mail<>());
			ui::e_end_menubar_menu();
			ui::e_begin_menubar_menu(L"Edit");
			ui::e_menu_item(L"Delete", [](void* c) {
				looper().add_event([](void* c, bool*) {
					if (app.selected)
					{
						app.selected = nullptr;
						if (app.inspector)
							app.inspector->refresh();
						app.selected->parent()->remove_child(app.selected);
						if (app.hierarchy)
							app.hierarchy->refresh();
					}
				}, Mail<>());
			}, Mail<>());
			ui::e_menu_item(L"Duplicate", [](void* c) {

			}, Mail<>());
		ui::e_end_menubar_menu();
		ui::e_begin_menubar_menu(L"Window");
		ui::e_menu_item(L"Resource Explorer", [](void* c) {
			if (!app.resource_explorer)
				app.resource_explorer = new cResourceExplorer;
		}, new_mail_p(this));
		ui::e_menu_item(L"Scene Editor", [](void* c) {
			if (!app.editor)
				app.editor = new cEditor;
		}, new_mail_p(this));
		ui::e_menu_item(L"Hierarchy", [](void* c) {
			if (!app.hierarchy)
				app.hierarchy = new cHierarchy;
		}, new_mail_p(this));
		ui::e_menu_item(L"Inspector", [](void* c) {
			if (!app.inspector)
				app.inspector = new cInspector;
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
}

void MyApp::load(const std::filesystem::path& _filepath)
{
	filepath = _filepath;
	if (prefab)
		prefab->parent()->remove_child(prefab);
	prefab = Entity::create_from_file(u->world(0), filepath.c_str());
	if (editor)
		editor->e_scene->add_child(prefab);
}

int main(int argc, char **args)
{
	app.create();

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
