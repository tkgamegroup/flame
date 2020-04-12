#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>

#include "app.h"

MyApp app;

void add_window(pugi::xml_node n) 
{
	auto parent_layout = utils::current_parent()->get_component(cLayout)->type == LayoutHorizontal;
	auto r = n.attribute("r").as_int(1);
	std::string name(n.name());
	if (name == "layout")
	{
		auto ca = utils::e_begin_docker_layout(n.attribute("type").value() == std::string("h") ? LayoutHorizontal : LayoutVertical)->get_component(cAligner);
		if (parent_layout)
			ca->width_factor_ = r;
		else
			ca->height_factor_ = r;
		for (auto c : n.children())
			add_window(c);
		utils::e_end_docker_layout();
	}
	else if (name == "docker")
	{
		auto ca = utils::e_begin_docker()->get_component(cAligner);
		if (parent_layout)
			ca->width_factor_ = r;
		else
			ca->height_factor_ = r;
		auto name = std::string(n.child("page").attribute("name").value());
		if (name == "editor")
			app.editor = new cEditor;
		else if (name == "resource_explorer")
			app.resource_explorer = new cResourceExplorer;
		else if (name == "hierarchy")
			app.hierarchy = new cHierarchy;
		else if (name == "inspector")
			app.inspector = new cInspector;
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
	set_engine_path(engine_path.c_str());

	App::create("Scene Editor", Vec2u(300, 200), WindowFrame | WindowResizable, true, engine_path, true);

	pugi::xml_document window_layout;
	pugi::xml_node window_layout_root;
	if (window_layout.load_file(L"window_layout.xml"))
		window_layout_root = window_layout.first_child();

	canvas->clear_color = Vec4f(100, 100, 100, 255) / 255.f;
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
			{
				utils::next_element_pos = Vec2f(100.f);
				utils::next_element_size = Vec2f(400.f, 300.f);
				utils::e_begin_docker_floating_container();
					utils::e_begin_docker();
						app.editor = new cEditor;
					utils::e_end_docker();
				utils::e_end_docker_floating_container();
			}
		}, Mail::from_p(this));
		utils::e_menu_item(L"Resource Explorer", [](void* c) {
			if (!app.resource_explorer)
			{
				utils::next_element_pos = Vec2f(100.f);
				utils::next_element_size = Vec2f(400.f, 300.f);
				utils::e_begin_docker_floating_container();
					utils::e_begin_docker();
						app.resource_explorer = new cResourceExplorer;
					utils::e_end_docker();
				utils::e_end_docker_floating_container();
			}
		}, Mail::from_p(this));
		utils::e_menu_item(L"Hierarchy", [](void* c) {
			if (!app.hierarchy)
			{
				utils::next_element_pos = Vec2f(100.f);
				utils::next_element_size = Vec2f(400.f, 300.f);
				utils::e_begin_docker_floating_container();
					utils::e_begin_docker();
						app.hierarchy = new cHierarchy;
					utils::e_end_docker();
				utils::e_end_docker_floating_container();
			}
		}, Mail::from_p(this));
		utils::e_menu_item(L"Inspector", [](void* c) {
			if (!app.inspector)
			{
				utils::next_element_pos = Vec2f(100.f);
				utils::next_element_size = Vec2f(400.f, 300.f);
				utils::e_begin_docker_floating_container();
					utils::e_begin_docker();
						app.inspector = new cInspector;
					utils::e_end_docker();
				utils::e_end_docker_floating_container();
			}
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
	selected = nullptr;
	if (editor)
		editor->e_base->add_child(prefab);
	if (hierarchy)
		hierarchy->refresh();
}

int main(int argc, char **args)
{
	app.create();

	looper().loop([](void*) {
		app.run();
	}, Mail());

	return 0;
}
