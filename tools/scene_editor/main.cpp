#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>
#include <flame/universe/utils/ui_reflector.h>

#include "app.h"

#include <flame/universe/utils/ui_impl.h>

struct Action
{
	wchar_t* name;
	virtual void undo() = 0;
	virtual void redo() = 0;
};

struct Action_ChangeEntityName : Action
{
	Entity* target;
	std::string prev_id;
	std::string after_id;

	Action_ChangeEntityName()
	{
		name = L"Change Entity Name";
	}

	void undo() override
	{

	}

	void redo() override
	{

	}
};

static std::vector<std::unique_ptr<Action>> actions;
static auto action_idx = 0;

static void add_action(Action* a)
{
	if (actions.size() > action_idx)
		actions.erase(actions.begin() + action_idx, actions.end());
	actions.emplace_back(a);
	action_idx++;
}

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
			app.editor = new cSceneEditor;
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
	App::create();
}

void MyApp::select(Entity* e)
{
	if (selected != e)
	{
		selected = e;
		if (editor)
			editor->on_select();
		looper().add_event([](Capture&) {
			if (app.hierarchy)
				app.hierarchy->refresh_selected();
			if (app.inspector)
				app.inspector->refresh();
		}, Capture());
	}
}

void MyApp::load(const std::filesystem::path& _filepath)
{
	filepath = _filepath;
	prefab = filepath.empty() ? nullptr : Entity::create_from_file(main_window->world, filepath.c_str());
	selected = nullptr;
	looper().add_event([](Capture&) {
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
	}, Capture());
}

void MyApp::save()
{
	if (prefab)
		Entity::save_to_file(prefab, filepath.c_str());
}

MyApp app;

MainWindow::MainWindow() :
	App::Window(&app, true, true, "Scene Editor", Vec2u(300, 200), WindowFrame | WindowResizable, nullptr, true)
{
	main_window = this;

	setup_as_main_window();

	utils::set_current_root(root);
	utils::set_current_entity(root);

	canvas->clear_color = Vec4f(100, 100, 100, 255) / 255.f;
	utils::style_set_to_light();

	pugi::xml_document window_layout;
	pugi::xml_node window_layout_root;
	if (window_layout.load_file(L"window_layout.xml"))
		window_layout_root = window_layout.first_child();

	{
		auto c_event_receiver = root->get_component(cEventReceiver);
		c_event_receiver->key_listeners.add([](Capture& c, KeyStateFlags action, int value) {
			if (is_key_down(action))
			{
				auto ed = c.current<cEventReceiver>()->dispatcher;
				switch (value)
				{
				case Key_S:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						app.save();
					break;
				case Key_Z:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						undo();
					break;
				case Key_Y:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						redo();
					break;
				case Key_X:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						cut_selected();
					break;
				case Key_C:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						copy_selected();
					break;
				case Key_V:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						paste();
					break;
				case Key_D:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						duplicate_selected();
					break;
				case Key_Del:
					delete_selected();
					break;
				}
			}
			return true;
		}, Capture());
		s_event_dispatcher->next_focusing = c_event_receiver;
	}

	utils::push_parent(root);

	utils::e_begin_layout(LayoutVertical, 0.f, false, false);
	utils::c_aligner(AlignMinMax, AlignMinMax);

	utils::e_begin_menu_bar();
	utils::e_begin_menubar_menu(L"Scene");
	utils::e_menu_item(L"        New Entity", [](Capture&) {
		looper().add_event([](Capture&) {
			auto e = Entity::create();
			e->set_name("unnamed");
			if (app.selected)
				app.selected->add_child(e);
			else
				app.prefab->add_child(e);
			if (app.hierarchy)
				app.hierarchy->refresh();
		}, Capture());
	}, Capture());
	utils::e_menu_item((std::wstring(Icon_FLOPPY_O) + L"    Save").c_str(), [](Capture& c) {

	}, Capture());
	utils::e_menu_item(L"        Close", [](Capture& c) {
		app.load(L"");
	}, Capture());
	utils::e_end_menubar_menu();
	utils::e_begin_menubar_menu(L"Edit");
	utils::e_menu_item((std::wstring(Icon_UNDO) + L"    Undo").c_str(), [](Capture&) {

	}, Capture());
	utils::e_menu_item((std::wstring(Icon_REPEAT) + L"    Redo").c_str(), [](Capture&) {

	}, Capture());
	utils::e_menu_item((std::wstring(Icon_SCISSORS) + L"    Cut").c_str(), [](Capture&) {
	}, Capture());
	utils::e_menu_item((std::wstring(Icon_CLONE) + L"   Copy").c_str(), [](Capture&) {
	}, Capture());
	utils::e_menu_item((std::wstring(Icon_CLIPBOARD) + L"   Paste").c_str(), [](Capture&) {
	}, Capture());
	utils::e_menu_item((std::wstring(Icon_CLONE) + L"   Duplicate").c_str(), [](Capture&) {

	}, Capture());
	utils::e_menu_item((std::wstring(Icon_TIMES) + L"   Delete").c_str(), [](Capture&) {
		looper().add_event([](Capture&) {
			if (app.selected)
			{
				app.selected = nullptr;
				if (app.inspector)
					app.inspector->refresh();
				app.selected->parent()->remove_child(app.selected);
				if (app.hierarchy)
					app.hierarchy->refresh();
			}
		}, Capture());
	}, Capture());
	utils::e_end_menubar_menu();
	utils::e_begin_menubar_menu(L"Window");
	utils::e_menu_item(L"Editor", [](Capture&) {
		if (!app.editor)
		{
			utils::push_parent(main_window->root);
			utils::next_element_pos = Vec2f(100.f);
			utils::next_element_size = Vec2f(400.f, 300.f);
			utils::e_begin_docker_floating_container();
			utils::e_begin_docker();
			app.editor = new cSceneEditor;
			utils::e_end_docker();
			utils::e_end_docker_floating_container();
			utils::pop_parent();
		}
	}, Capture().set_thiz(this));
	utils::e_menu_item(L"Resource Explorer", [](Capture&) {
		if (!app.resource_explorer)
		{
			utils::push_parent(main_window->root);
			utils::next_element_pos = Vec2f(100.f);
			utils::next_element_size = Vec2f(400.f, 300.f);
			utils::e_begin_docker_floating_container();
			utils::e_begin_docker();
			app.resource_explorer = new cResourceExplorer;
			utils::e_end_docker();
			utils::e_end_docker_floating_container();
			utils::pop_parent();
		}
	}, Capture().set_thiz(this));
	utils::e_menu_item(L"Hierarchy", [](Capture&) {
		if (!app.hierarchy)
		{
			utils::push_parent(main_window->root);
			utils::next_element_pos = Vec2f(100.f);
			utils::next_element_size = Vec2f(400.f, 300.f);
			utils::e_begin_docker_floating_container();
			utils::e_begin_docker();
			app.hierarchy = new cHierarchy;
			utils::e_end_docker();
			utils::e_end_docker_floating_container();
			utils::pop_parent();
		}
	}, Capture().set_thiz(this));
	utils::e_menu_item(L"Inspector", [](Capture&) {
		if (!app.inspector)
		{
			utils::push_parent(main_window->root);
			utils::next_element_pos = Vec2f(100.f);
			utils::next_element_size = Vec2f(400.f, 300.f);
			utils::e_begin_docker_floating_container();
			utils::e_begin_docker();
			app.inspector = new cInspector;
			utils::e_end_docker();
			utils::e_end_docker_floating_container();
			utils::pop_parent();
		}
	}, Capture().set_thiz(this));
	utils::e_end_menubar_menu();
	utils::e_begin_menubar_menu(L"Tools");
	utils::e_menu_item(L"Reflector", [](Capture& c) {
		utils::e_ui_reflector_window();
	}, Capture());
	utils::e_end_menubar_menu();
	utils::e_end_menu_bar();

	utils::e_begin_docker_static_container();
	if (window_layout_root)
		add_window(window_layout_root.child("static").first_child());
	utils::e_end_docker_static_container();

	utils::e_end_layout();

	utils::pop_parent();
}

MainWindow* main_window = nullptr;

int main(int argc, char **args)
{
	app.create();

	new MainWindow;

	looper().loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
