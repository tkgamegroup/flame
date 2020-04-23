#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>
#include <flame/universe/utils/ui_reflector.h>

#include "app.h"

#include <flame/universe/utils/entity_impl.h>
#include <flame/universe/utils/ui_impl.h>

MyApp app;

struct Action
{
	wchar_t* name;
	virtual void undo() = 0;
	virtual void redo() = 0;
};

static void undo() 
{

}

static void redo()
{

}

static void cut_selected()
{

}

static void copy_selected()
{

}

static void paste()
{

}

static void duplicate_selected()
{

}

static void delete_selected()
{

}

void add_window(pugi::xml_node n) 
{
	auto parent_layout = utils::current_parent()->get_component(cLayout)->type == LayoutHorizontal;
	auto r = n.attribute("r").as_int(1);
	std::string name(n.name());
	if (name == "layout")
	{
		auto ca = utils::e_begin_docker_layout(n.attribute("type").value() == std::string("h") ? LayoutHorizontal : LayoutVertical)->get_component(cAligner);
		if (parent_layout)
			ca->width_factor = r;
		else
			ca->height_factor = r;
		for (auto c : n.children())
			add_window(c);
		utils::e_end_docker_layout();
	}
	else if (name == "docker")
	{
		auto ca = utils::e_begin_docker()->get_component(cAligner);
		if (parent_layout)
			ca->width_factor = r;
		else
			ca->height_factor = r;
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
	App::create("Scene Editor", Vec2u(300, 200), WindowFrame | WindowResizable, true, true);

	pugi::xml_document window_layout;
	pugi::xml_node window_layout_root;
	if (window_layout.load_file(L"window_layout.xml"))
		window_layout_root = window_layout.first_child();

	canvas->clear_color = Vec4f(100, 100, 100, 255) / 255.f;
	utils::style_set_to_light();
	{
		auto c_event_receiver = root->get_component(cEventReceiver);
		c_event_receiver->key_listeners.add([](void*, KeyStateFlags action, int value) {
			if (is_key_down(action))
			{
				switch (value)
				{
				case Key_S:
					if (app.s_event_dispatcher->key_states[Key_Ctrl] & KeyStateDown)
						app.save();
					break;
				case Key_Z:
					if (app.s_event_dispatcher->key_states[Key_Ctrl] & KeyStateDown)
						undo();
					break;
				case Key_Y:
					if (app.s_event_dispatcher->key_states[Key_Ctrl] & KeyStateDown)
						redo();
					break;
				case Key_X:
					if (app.s_event_dispatcher->key_states[Key_Ctrl] & KeyStateDown)
						cut_selected();
					break;
				case Key_C:
					if (app.s_event_dispatcher->key_states[Key_Ctrl] & KeyStateDown)
						copy_selected();
					break;
				case Key_V:
					if (app.s_event_dispatcher->key_states[Key_Ctrl] & KeyStateDown)
						paste();
					break;
				case Key_D:
					if (app.s_event_dispatcher->key_states[Key_Ctrl] & KeyStateDown)
						duplicate_selected();
					break;
				case Key_Del:
					delete_selected();
					break;
				}
			}
			return true;
		}, Mail());
		s_event_dispatcher->next_focusing = c_event_receiver;
	}

	utils::push_parent(root);

	utils::e_begin_layout(LayoutVertical, 0.f, false, false);
	utils::c_aligner(AlignMinMax, AlignMinMax);

	utils::e_begin_menu_bar();
		utils::e_begin_menubar_menu(L"Scene");
			utils::e_menu_item(L"        New Entity", [](void*) {
				looper().add_event([](void*, bool*) {
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
			utils::e_menu_item((std::wstring(Icon_FLOPPY_O) + L"    Save").c_str(), [](void* c) {

			}, Mail());
			utils::e_menu_item(L"        Close", [](void* c) {
				app.load(L"");
			}, Mail());
			utils::e_end_menubar_menu();
			utils::e_begin_menubar_menu(L"Edit");
			utils::e_menu_item((std::wstring(Icon_UNDO) + L"    Undo").c_str(), [](void*) {

			}, Mail());
			utils::e_menu_item((std::wstring(Icon_REPEAT) + L"    Redo").c_str(), [](void*) {

			}, Mail());
			utils::e_menu_item((std::wstring(Icon_SCISSORS) + L"    Cut").c_str(), [](void*) {
			}, Mail());
			utils::e_menu_item((std::wstring(Icon_CLONE) + L"   Copy").c_str(), [](void*) {
			}, Mail());
			utils::e_menu_item((std::wstring(Icon_CLIPBOARD) + L"   Paste").c_str(), [](void*) {
			}, Mail());
			utils::e_menu_item((std::wstring(Icon_CLONE) + L"   Duplicate").c_str(), [](void* c) {

			}, Mail());
			utils::e_menu_item((std::wstring(Icon_TIMES) + L"   Delete").c_str(), [](void*) {
				looper().add_event([](void*, bool*) {
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
		utils::e_end_menubar_menu();
		utils::e_begin_menubar_menu(L"Window");
		utils::e_menu_item(L"Editor", [](void*) {
			if (!app.editor)
			{
				utils::push_parent(app.root);
				utils::next_element_pos = Vec2f(100.f);
				utils::next_element_size = Vec2f(400.f, 300.f);
				utils::e_begin_docker_floating_container();
					utils::e_begin_docker();
						app.editor = new cEditor;
					utils::e_end_docker();
				utils::e_end_docker_floating_container();
				utils::pop_parent();
			}
		}, Mail::from_p(this));
		utils::e_menu_item(L"Resource Explorer", [](void*) {
			if (!app.resource_explorer)
			{
				utils::push_parent(app.root);
				utils::next_element_pos = Vec2f(100.f);
				utils::next_element_size = Vec2f(400.f, 300.f);
				utils::e_begin_docker_floating_container();
					utils::e_begin_docker();
						app.resource_explorer = new cResourceExplorer;
					utils::e_end_docker();
				utils::e_end_docker_floating_container();
				utils::pop_parent();
			}
		}, Mail::from_p(this));
		utils::e_menu_item(L"Hierarchy", [](void*) {
			if (!app.hierarchy)
			{
				utils::push_parent(app.root);
				utils::next_element_pos = Vec2f(100.f);
				utils::next_element_size = Vec2f(400.f, 300.f);
				utils::e_begin_docker_floating_container();
					utils::e_begin_docker();
						app.hierarchy = new cHierarchy;
					utils::e_end_docker();
				utils::e_end_docker_floating_container();
				utils::pop_parent();
			}
		}, Mail::from_p(this));
		utils::e_menu_item(L"Inspector", [](void*) {
			if (!app.inspector)
			{
				utils::push_parent(app.root);
				utils::next_element_pos = Vec2f(100.f);
				utils::next_element_size = Vec2f(400.f, 300.f);
				utils::e_begin_docker_floating_container();
					utils::e_begin_docker();
						app.inspector = new cInspector;
					utils::e_end_docker();
				utils::e_end_docker_floating_container();
				utils::pop_parent();
			}
		}, Mail::from_p(this));
		utils::e_end_menubar_menu();
		utils::e_begin_menubar_menu(L"Tools");
			utils::e_menu_item(L"Reflector", [](void* c) {
				utils::e_ui_reflector_window();
			}, Mail());
		utils::e_end_menubar_menu();
	utils::e_end_menu_bar();

	utils::e_begin_docker_static_container();
	if (window_layout_root)
		add_window(window_layout_root.child("static").first_child());
	utils::e_end_docker_static_container();

	utils::e_end_layout();

	utils::pop_parent();
}

void MyApp::select(Entity* e)
{
	if (selected != e)
	{
		selected = e;
		if (editor)
			editor->on_select();
		looper().add_event([](void*, bool*) {
			if (app.hierarchy)
				app.hierarchy->refresh_selected();
			if (app.inspector)
				app.inspector->refresh();
		}, Mail());
	}
}

void MyApp::load(const std::filesystem::path& _filepath)
{
	filepath = _filepath;
	prefab = filepath.empty() ? nullptr : Entity::create_from_file(world, filepath.c_str());
	selected = nullptr;
	looper().add_event([](void*, bool*) {
		if (app.editor)
		{
			app.editor->on_select();
			auto e_base = app.editor->edt.base->entity;
			e_base->remove_children(0, -1);
			if (app.prefab)
				e_base->add_child(app.prefab);
		}
		if (app.hierarchy)
			app.hierarchy->refresh();
		if (app.inspector)
			app.inspector->refresh();
	}, Mail());
}

void MyApp::save()
{

}

int main(int argc, char **args)
{
	app.create();

	looper().loop([](void*) {
		app.run();
	}, Mail());

	return 0;
}
