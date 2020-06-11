#include <flame/universe/utils/ui_reflector.h>

#include "scene_editor.h"

struct Action
{
	wchar_t* name;
	virtual void undo() = 0;
	virtual void redo() = 0;
};

struct Action_ChangeEntityName : Action
{
	Guid guid;
	std::string before_id;
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

static void add_window(pugi::xml_node n)
{
	auto& ui = scene_editor.window->ui;

	auto parent_layout = ui.parents.top()->get_component(cLayout)->type == LayoutHorizontal;
	auto r = n.attribute("r").as_int(1);
	std::string name(n.name());
	if (name == "layout")
	{
		auto ca = ui.e_begin_docker_layout(n.attribute("type").value() == std::string("h") ? LayoutHorizontal : LayoutVertical)->get_component(cAligner);
		if (parent_layout)
			ca->width_factor = r;
		else
			ca->height_factor = r;
		for (auto c : n.children())
			add_window(c);
		ui.e_end_docker_layout();
	}
	else if (name == "docker")
	{
		auto ca = ui.e_begin_docker()->get_component(cAligner);
		if (parent_layout)
			ca->width_factor = r;
		else
			ca->height_factor = r;
		auto name = std::string(n.child("page").attribute("name").value());
		if (name == "editor")
			scene_editor.editor = new cSceneEditor;
		else if (name == "resource_explorer")
			scene_editor.resource_explorer = new cResourceExplorer;
		else if (name == "hierarchy")
			scene_editor.hierarchy = new cHierarchy;
		else if (name == "inspector")
			scene_editor.inspector = new cInspector;
		ui.e_end_docker();
	}
}

void SceneEditor::select(Entity* e)
{
	if (selected != e)
	{
		selected = e;
		if (editor)
			editor->on_select();
		looper().add_event([](Capture&) {
			if (scene_editor.hierarchy)
				scene_editor.hierarchy->refresh_selected();
			if (scene_editor.inspector)
				scene_editor.inspector->refresh();
		}, Capture());
	}
}

void SceneEditor::load(const std::filesystem::path& _filepath)
{
	filepath = _filepath;
	prefab = filepath.empty() ? nullptr : Entity::create_from_file(scene_editor.window->world, filepath.c_str());
	selected = nullptr;
	looper().add_event([](Capture&) {
		if (scene_editor.editor)
		{
			scene_editor.editor->on_select();
			auto e_base = scene_editor.editor->edt.base->entity;
			e_base->remove_children(0, -1);
			if (scene_editor.prefab)
				e_base->add_child(scene_editor.prefab);
		}
		if (scene_editor.hierarchy)
			scene_editor.hierarchy->refresh();
		if (scene_editor.inspector)
			scene_editor.inspector->refresh();
	}, Capture());
}

void SceneEditor::save()
{
	if (prefab)
		Entity::save_to_file(prefab, filepath.c_str());
}

MyApp app;

SceneEditorWindow::SceneEditorWindow() :
	GraphicsWindow(&app, true, true, "Scene Editor", Vec2u(300, 200), WindowFrame | WindowResizable, nullptr, true)
{
	scene_editor.window = this;

	setup_as_main_window();

	canvas->clear_color = Vec4f(100, 100, 100, 255) / 255.f;

	ui.init(world);
	ui.style_set_to_light();

	pugi::xml_document window_layout;
	pugi::xml_node window_layout_root;
	if (window_layout.load_file(L"scene_editor_layout.xml"))
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
						scene_editor.save();
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

	ui.parents.push(root);

	ui.e_begin_layout(LayoutVertical, 0.f, false, false);
	ui.c_aligner(AlignMinMax, AlignMinMax);

	ui.e_begin_menu_bar();
	ui.e_begin_menubar_menu(L"Scene");
	ui.e_menu_item(L"        New Entity", [](Capture&) {
		looper().add_event([](Capture&) {
			auto e = f_new<Entity>();
			e->name = "unnamed";
			if (scene_editor.selected)
				scene_editor.selected->add_child(e);
			else
				scene_editor.prefab->add_child(e);
			if (scene_editor.hierarchy)
				scene_editor.hierarchy->refresh();
		}, Capture());
	}, Capture());
	ui.e_menu_item((std::wstring(Icon_FLOPPY_O) + L"    Save").c_str(), [](Capture& c) {

	}, Capture());
	ui.e_menu_item(L"        Close", [](Capture& c) {
		scene_editor.load(L"");
	}, Capture());
	ui.e_end_menubar_menu();
	ui.e_begin_menubar_menu(L"Edit");
	ui.e_menu_item((std::wstring(Icon_UNDO) + L"    Undo").c_str(), [](Capture&) {

	}, Capture());
	ui.e_menu_item((std::wstring(Icon_REPEAT) + L"    Redo").c_str(), [](Capture&) {

	}, Capture());
	ui.e_menu_item((std::wstring(Icon_SCISSORS) + L"    Cut").c_str(), [](Capture&) {
	}, Capture());
	ui.e_menu_item((std::wstring(Icon_CLONE) + L"   Copy").c_str(), [](Capture&) {
	}, Capture());
	ui.e_menu_item((std::wstring(Icon_CLIPBOARD) + L"   Paste").c_str(), [](Capture&) {
	}, Capture());
	ui.e_menu_item((std::wstring(Icon_CLONE) + L"   Duplicate").c_str(), [](Capture&) {

	}, Capture());
	ui.e_menu_item((std::wstring(Icon_TIMES) + L"   Delete").c_str(), [](Capture&) {
		looper().add_event([](Capture&) {
			if (scene_editor.selected)
			{
				scene_editor.selected = nullptr;
				if (scene_editor.inspector)
					scene_editor.inspector->refresh();
				scene_editor.selected->parent->remove_child(scene_editor.selected);
				if (scene_editor.hierarchy)
					scene_editor.hierarchy->refresh();
			}
		}, Capture());
	}, Capture());
	ui.e_end_menubar_menu();
	ui.e_begin_menubar_menu(L"Window");
	ui.e_menu_item(L"Editor", [](Capture&) {
		if (!scene_editor.editor)
		{
			auto& ui = scene_editor.window->ui;
			ui.parents.push(scene_editor.window->root);
			ui.next_element_pos = Vec2f(100.f);
			ui.next_element_size = Vec2f(400.f, 300.f);
			ui.e_begin_docker_floating_container();
			ui.e_begin_docker();
			scene_editor.editor = new cSceneEditor;
			ui.e_end_docker();
			ui.e_end_docker_floating_container();
			ui.parents.pop();
		}
	}, Capture().set_thiz(this));
	ui.e_menu_item(L"Resource Explorer", [](Capture&) {
		if (!scene_editor.resource_explorer)
		{
			auto& ui = scene_editor.window->ui;
			ui.parents.push(scene_editor.window->root);
			ui.next_element_pos = Vec2f(100.f);
			ui.next_element_size = Vec2f(400.f, 300.f);
			ui.e_begin_docker_floating_container();
			ui.e_begin_docker();
			scene_editor.resource_explorer = new cResourceExplorer;
			ui.e_end_docker();
			ui.e_end_docker_floating_container();
			ui.parents.pop();
		}
	}, Capture().set_thiz(this));
	ui.e_menu_item(L"Hierarchy", [](Capture&) {
		if (!scene_editor.hierarchy)
		{
			auto& ui = scene_editor.window->ui;
			ui.parents.push(scene_editor.window->root);
			ui.next_element_pos = Vec2f(100.f);
			ui.next_element_size = Vec2f(400.f, 300.f);
			ui.e_begin_docker_floating_container();
			ui.e_begin_docker();
			scene_editor.hierarchy = new cHierarchy;
			ui.e_end_docker();
			ui.e_end_docker_floating_container();
			ui.parents.pop();
		}
	}, Capture().set_thiz(this));
	ui.e_menu_item(L"Inspector", [](Capture&) {
		if (!scene_editor.inspector)
		{
			auto& ui = scene_editor.window->ui;
			ui.parents.push(scene_editor.window->root);
			ui.next_element_pos = Vec2f(100.f);
			ui.next_element_size = Vec2f(400.f, 300.f);
			ui.e_begin_docker_floating_container();
			ui.e_begin_docker();
			scene_editor.inspector = new cInspector;
			ui.e_end_docker();
			ui.e_end_docker_floating_container();
			ui.parents.pop();
		}
	}, Capture().set_thiz(this));
	ui.e_end_menubar_menu();
	ui.e_begin_menubar_menu(L"Tools");
	ui.e_menu_item(L"Reflector", [](Capture& c) {
		auto& ui = scene_editor.window->ui;
		create_ui_reflector(ui);
	}, Capture());
	ui.e_end_menubar_menu();
	ui.e_end_menu_bar();

	ui.e_begin_docker_static_container();
	if (window_layout_root)
		add_window(window_layout_root.child("static").first_child());
	ui.e_end_docker_static_container();

	ui.e_end_layout();

	ui.parents.pop();
}

SceneEditorWindow::~SceneEditorWindow()
{
	scene_editor.window = nullptr;
}

SceneEditor scene_editor;
