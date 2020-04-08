#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>

#include "app.h"

MyApp app;

void add_window(pugi::xml_node n) 
{
	std::string name(n.name());
	if (name == "layout")
	{
		utils::e_begin_docker_layout(n.attribute("type").value() == std::string("h") ? LayoutHorizontal : LayoutVertical);
		for (auto c : n.children())
			add_window(c);
		utils::e_end_docker_layout();
	}
	else if (name == "docker")
	{
		utils::e_begin_docker();
		for (auto c : n.children())
		{
			if (c.name() == std::string("page"))
			{
				std::string window(c.attribute("name").value());
				if (window == "editor")
					app.editor = new cEditor;
				else if (window == "resource_explorer")
					app.resource_explorer = new cResourceExplorer;
				else if (window == "hierarchy")
					app.hierarchy = new cHierarchy;
				else if (window == "inspector")
					app.inspector = new cInspector;
			}
		}
		utils::e_end_docker();
	}
}

void MyApp::create()
{
	{
		auto config = parse_ini_file(L"config.ini");
		for (auto& e : config.get_section_entries(""))
		{
			if (e.key == "resource_path")
				resource_path = e.value;
			else if (e.key == "engine_path")
			{
				if (e.value == "{e}")
					engine_path = getenv("FLAME_PATH");
				else
					engine_path = e.value;
			}
		}
	}

	App::create("Scene Editor", Vec2u(300, 200), WindowFrame | WindowResizable, true, getenv("FLAME_PATH"), true);

	TypeinfoDatabase::load(L"scene_editor.exe", true, true);

	pugi::xml_document window_layout;
	pugi::xml_node window_layout_root;
	if (window_layout.load_file(L"window_layout.xml"))
		window_layout_root = window_layout.first_child();

	canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	utils::style_set_to_light();

	utils::push_parent(root);

	utils::e_begin_layout(LayoutVertical, 0.f, false, false);
	utils::c_aligner(SizeFitParent, SizeFitParent);

	utils::e_begin_menu_bar();
		utils::e_begin_menubar_menu(L"Scene");
			utils::e_menu_item(L"New Entity", [](void* c) {
				looper().add_event([](void* c, bool*) {
					auto e = Entity::create();
					e->set_name("unnamed");
					if (app.selected)
						app.selected->add_child(e);
					else
						app.prefab->add_child(e);
					if (app.hierarchy)
						app.hierarchy->refresh();
				}, Mail());
			}, Mail());
			utils::e_menu_item(L"Save", [](void* c) {

			}, Mail());
			utils::e_end_menubar_menu();
			utils::e_begin_menubar_menu(L"Edit");
			utils::e_menu_item(L"Delete", [](void* c) {
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
				}, Mail());
			}, Mail());
			utils::e_menu_item(L"Duplicate", [](void* c) {

			}, Mail());
		utils::e_end_menubar_menu();
		utils::e_begin_menubar_menu(L"Window");
		utils::e_menu_item(L"Editor", [](void* c) {
			if (!app.editor)
				app.editor = new cEditor;
		}, Mail::from_p(this));
		utils::e_menu_item(L"Resource Explorer", [](void* c) {
			if (!app.resource_explorer)
				app.resource_explorer = new cResourceExplorer;
		}, Mail::from_p(this));
		utils::e_menu_item(L"Hierarchy", [](void* c) {
			if (!app.hierarchy)
				app.hierarchy = new cHierarchy;
		}, Mail::from_p(this));
		utils::e_menu_item(L"Inspector", [](void* c) {
			if (!app.inspector)
				app.inspector = new cInspector;
		}, Mail::from_p(this));
		utils::e_end_menubar_menu();
	utils::e_end_menu_bar();

	utils::e_begin_docker_static_container();
	if (window_layout_root)
		add_window(window_layout_root.child("static").first_child());
	utils::e_end_docker_static_container();

	utils::e_end_layout();

	utils::pop_parent();
}

void MyApp::load(const std::filesystem::path& _filepath)
{
	filepath = _filepath;
	if (prefab)
		prefab->parent()->remove_child(prefab);
	prefab = Entity::create_from_file(world, filepath.c_str());
	if (editor)
		editor->e_scene->add_child(prefab);
}

int main(int argc, char **args)
{
	app.create();

	looper().loop([](void*) {
		app.run();
	}, Mail());

	return 0;
}
