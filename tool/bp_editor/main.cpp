#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>

#include "app.h"

struct Action
{
	virtual void undo() = 0;
	virtual void redo() = 0;
};

struct Action_ChangeID : Action
{
	std::string prev_id;
	std::string after_id;

	void undo() override
	{
		auto n = app.bp->find_node(after_id.c_str());
		if (n)
		{
			n->set_id(prev_id.c_str());
			if (app.editor)
				app.editor->on_id_changed(n);
		}
	}

	void redo() override
	{
		auto n = app.bp->find_node(prev_id.c_str());
		if (n)
		{
			n->set_id(after_id.c_str());
			if (app.editor)
				app.editor->on_id_changed(n);
		}
	}
};

struct Action_MoveNode : Action
{
	std::string id;
	Vec2f prev_pos;
	Vec2f after_pos;

	void undo() override
	{
		auto n = app.bp->find_node(id.c_str());
		if (n)
		{
			n->pos = prev_pos;
			if (app.editor)
				app.editor->on_pos_changed(n);
		}
	}

	void redo() override
	{
		auto n = app.bp->find_node(id.c_str());
		if (n)
		{
			n->pos = after_pos;
			if (app.editor)
				app.editor->on_pos_changed(n);
		}
	}
};

struct Action_AddNode : Action
{
	std::string type;
	std::string id;
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

void undo()
{
	if (action_idx > 0)
	{
		action_idx--;
		actions[action_idx]->undo();

		if (action_idx == 0)
			app.set_changed(false);
	}
}

void redo()
{
	if (action_idx < actions.size())
	{
		actions[action_idx]->redo();
		action_idx++;

		app.set_changed(true);
	}
}

void MyApp::set_changed(bool v)
{
	if (changed != v)
	{
		changed = v;
		if (editor)
			editor->on_bp_changed();
	}
}

void MyApp::set_id(BP::Node* n, const std::string& id)
{
	std::string prev_id = n->id();

	if (!n->set_id(id.c_str()))
		return;

	auto a = new Action_ChangeID;
	a->prev_id = prev_id;
	a->after_id = id;
	add_action(a);

	set_changed(true);
}

void MyApp::set_node_pos(BP::Node* n, const Vec2f& pos)
{
	auto a = new Action_MoveNode;
	a->id = n->id();
	a->prev_pos = n->pos;
	a->after_pos = pos;
	add_action(a);

	n->pos = pos;

	set_changed(true);
}

void MyApp::deselect()
{
	if (editor)
		editor->on_deselect();

	selected_nodes.clear();
	selected_links.clear();
}

void MyApp::select(const std::vector<BP::Node*>& nodes)
{
	deselect();
	selected_nodes = nodes;

	if (editor)
		editor->on_select();
}

void MyApp::select(const std::vector<BP::Slot*>& links)
{
	deselect();
	selected_links = links;

	if (editor)
		editor->on_select();
}

BP::Library* MyApp::add_library(const wchar_t* filename)
{
	auto l = bp->add_library(filename);
	if (l)
		set_changed(true);
	return l;
}

BP::Node* MyApp::add_node(const char* type_name, const char* id, const Vec2f& pos)
{
	auto n = bp->add_node(type_name, id);
	n->pos = pos;
	if (editor)
		editor->on_add_node(n);
	if (!SUS::starts_with(id, "test_"))
		set_changed(true);
	return n;
}

void MyApp::remove_library(BP::Library* l)
{
	looper().add_event([](void* c, bool*) {
		auto l = *(BP::Library**)c;
		app.bp->remove_library(l);
		app.set_changed(true);
	}, new_mail_p(l));
}

void MyApp::remove_node(BP::Node* n)
{
	if (app.editor)
		app.editor->on_remove_node(n);
	n->scene()->remove_node(n);
	set_changed(true);
}

void MyApp::duplicate_selected()
{
	std::vector<BP::Node*> new_nodes;
	for (auto& s : selected_nodes)
	{
		auto n = add_node(s->type(), "", s->pos + Vec2f(20.f));
		new_nodes.push_back(n);
		for (auto i = 0; i < n->input_count(); i++)
			n->input(i)->set_data(s->input(i)->data());
	}
	for (auto i = 0; i < new_nodes.size(); i++)
	{
		auto n = new_nodes[i];
		for (auto j = 0; j < n->input_count(); j++)
		{
			auto link = selected_nodes[i]->input(j)->link(0);
			if (link)
			{
				auto nn = link->node();
				for (auto k = 0; k < selected_nodes.size(); k++)
				{
					if (nn == selected_nodes[k])
					{
						link = new_nodes[k]->output(link->index());
						break;
					}
				}
			}
			n->input(j)->link_to(link);
		}
	}
	select(new_nodes);
}

void MyApp::delete_selected()
{
	if (!selected_nodes.empty())
	{
		for (auto& s : selected_nodes)
			remove_node(s);
		selected_nodes.clear();
	}
	if (!selected_links.empty())
	{
		for (auto& s : selected_links)
			s->link_to(nullptr);
		selected_links.clear();
		set_changed(true);
	}
}

void MyApp::link_test_nodes()
{
	auto n_rt = app.bp->find_node("test_rt");
	if (!n_rt)
	{
		n_rt = app.add_node("TestRenderTarget", "test_rt", Vec2f(0.f, 0.f));
		n_rt->used_by_editor = true;
	}
	{
		auto s = app.bp->find_input("rt_dst.type");
		if (s)
			s->link_to(n_rt->find_output("type"));
	}
	{
		auto s = app.bp->find_input("rt_dst.v");
		if (s)
			s->link_to(n_rt->find_output("view"));
	}
	{
		auto s = app.bp->find_input("make_cmd.cbs");
		if (s)
			s->link_to(n_rt->find_output("out"));
	}
}

void MyApp::update_gv()
{
	auto gv_filename = fileppath / L"bp.gv";
	if (!std::filesystem::exists(gv_filename) || std::filesystem::last_write_time(gv_filename) < std::filesystem::last_write_time(filepath))
	{
		if (!GRAPHVIZ_PATH[0])
			assert(0);

		std::string gv = "digraph bp {\nrankdir=LR\nnode [shape = Mrecord];\n";
		for (auto i = 0; i < bp->node_count(); i++)
		{
			auto n = bp->node(i);
			auto name = n->id();
			auto str = sfmt("\t%s[label = \"%s|%s|{{", name, name, n->type());
			for (auto j = 0; j < n->input_count(); j++)
			{
				auto input = n->input(j);
				auto name = input->name();
				str += sfmt("<%s>%s", name, name);
				if (j != n->input_count() - 1)
					str += "|";
			}
			str += "}|{";
			for (auto j = 0; j < n->output_count(); j++)
			{
				auto output = n->output(j);
				auto name = output->name();
				str += sfmt("<%s>%s", name, name);
				if (j != n->output_count() - 1)
					str += "|";
			}
			str += "}}\"];\n";

			gv += str;
		}
		for (auto i = 0; i < bp->node_count(); i++)
		{
			auto n = bp->node(i);

			for (auto j = 0; j < n->input_count(); j++)
			{
				auto input = n->input(j);
				if (input->link())
				{
					auto in_sp = SUS::split(input->get_address().str(), '.');
					auto out_sp = SUS::split(input->link()->get_address().str(), '.');

					gv += "\t" + out_sp[0] + ":" + out_sp[1] + " -> " + in_sp[0] + ":" + in_sp[1] + ";\n";
				}
			}
		}
		gv += "}\n";

		std::ofstream file(gv_filename);
		file << gv;
		file.close();
	}
}

const auto dot_path = s2w(GRAPHVIZ_PATH) + L"/bin/dot.exe";

bool MyApp::generate_graph_image()
{
	update_gv();
	auto png_filename = fileppath / L"bp.png";
	if (!std::filesystem::exists(png_filename) || std::filesystem::last_write_time(png_filename) < std::filesystem::last_write_time(filepath))
		exec(dot_path.c_str(), (wchar_t*)(L"-Tpng " + fileppath.wstring() + L"/bp.gv -y -o " + png_filename.wstring()).c_str(), true);

	return std::filesystem::exists(png_filename);
}

bool MyApp::auto_set_layout()
{
	update_gv();
	auto graph_filename = fileppath / L"bp.graph";
	if (!std::filesystem::exists(graph_filename) || std::filesystem::last_write_time(graph_filename) < std::filesystem::last_write_time(filepath))
		exec(dot_path.c_str(), (wchar_t*)(L"-Tplain" + fileppath.wstring() + L"/bp.gv -y -o " + graph_filename.wstring()).c_str(), true);
	if (!std::filesystem::exists(graph_filename))
		return false;

	auto str = get_file_content(L"bp.graph.txt");
	for (auto it = str.begin(); it != str.end(); )
	{
		if (*it == '\\')
		{
			it = str.erase(it);
			if (it != str.end())
			{
				if (*it == '\r')
				{
					it = str.erase(it);
					if (it != str.end() && *it == '\n')
						it = str.erase(it);
				}
				else if (*it == '\n')
					it = str.erase(it);
			}
		}
		else
			it++;
	}

	std::regex reg_node(R"(node ([\w]+) ([\d\.]+) ([\d\.]+))");
	std::smatch res;
	while (std::regex_search(str, res, reg_node))
	{
		auto n = bp->find_node(res[1].str().c_str());
		if (n)
		{
			n->pos = Vec2f(std::stof(res[2].str().c_str()), std::stof(res[3].str().c_str())) * 100.f;
			((Entity*)n->user_data)->get_component(cElement)->set_pos(n->pos);
		}

		str = res.suffix();
	}

	return true;
}

bool MyApp::create(const char* filename)
{
	App::create("BP Editor", Vec2u(300, 200), WindowFrame | WindowResizable, true, getenv("FLAME_PATH"), true);

	TypeinfoDatabase::load(L"bp_editor.exe", true, true);

	filepath = filename;
	fileppath = app.filepath.parent_path();
	bp = BP::create_from_file(app.filepath.c_str());
	if (!bp)
		return false;

	app.window->set_title(("BP Editor - " + filepath.string()).c_str());

	pugi::xml_document window_layout;
	pugi::xml_node window_layout_root;
	auto window_layout_ok = window_layout.load_file(L"window_layout.xml") && (window_layout_root = window_layout.first_child()).name() == std::string("layout");

	canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	utils::style_set_to_light();

	utils::set_current_entity(root);
	{
		auto c_event_receiver = utils::c_event_receiver();
		c_event_receiver->key_listeners.add([](void*, KeyStateFlags action, int value) {
			if (is_key_down(action))
			{
				switch (value)
				{
				case Key_Z:
					if (app.s_event_dispatcher->key_states[Key_Ctrl] & KeyStateDown)
						undo();
					break;
				case Key_Y:
					if (app.s_event_dispatcher->key_states[Key_Ctrl] & KeyStateDown)
						redo();
					break;
				case Key_D:
					if (app.s_event_dispatcher->key_states[Key_Ctrl] & KeyStateDown)
						app.duplicate_selected();
					break;
				case Key_Del:
					app.delete_selected();
					break;
				}
			}
			return true;
		}, Mail<>());
		s_event_dispatcher->next_focusing = c_event_receiver;
	}
	utils::c_layout();

	utils::push_font_atlas(app.font_atlas_pixel);
	utils::set_current_root(root);
	utils::push_parent(root);

		utils::e_begin_layout(LayoutVertical, 0.f, false, false);
		utils::c_aligner(SizeFitParent, SizeFitParent);

			utils::e_begin_menu_bar();
				utils::e_begin_menubar_menu(L"Blueprint");
					utils::e_menu_item((std::wstring(Icon_FLOPPY_O) + L"    Save").c_str(), [](void* c) {
						BP::save_to_file(app.bp, app.filepath.c_str());
						app.set_changed(false);
					}, Mail<>());
					utils::e_menu_item((std::wstring(Icon_BOOK) + L"    Libraries").c_str(), [](void* c) {
						utils::e_begin_dialog();
							utils::e_text(L"Libraries");
							utils::e_begin_scroll_view1(ScrollbarVertical, Vec2f(200.f, 100.f), 4.f);
							auto e_list = utils::e_begin_list(true);
							for (auto i = 0; i < app.bp->library_count(); i++)
							{
								auto l = app.bp->library(i);
								utils::e_list_item(l->filename());
								utils::c_data_keeper()->add_voidp_item(FLAME_CHASH("library"), l);
							}
							utils::e_end_list();
							utils::e_end_scroll_view1();
							utils::e_begin_layout(LayoutHorizontal, 4.f);
							utils::e_button(L"Add", [](void* c) {
								utils::e_input_dialog(L"file", [](void* c, bool ok, const wchar_t* text) {
									if (ok && text[0])
									{
										auto l = app.add_library(text);
										if (!l)
											utils::e_message_dialog(L"Failed");
										else
										{
											utils::push_parent(*(Entity**)c);
											utils::e_list_item(l->filename());
											utils::c_data_keeper()->add_voidp_item(FLAME_CHASH("library"), l);
											utils::pop_parent();
										}
									}
								}, new_mail_p(*(void**)c));
							}, new_mail_p(e_list));
							utils::e_button(L"Remove", [](void* c) {
								auto e_list = *(Entity**)c;
								auto c_list = e_list->get_component(cList);
								auto e_item = c_list->selected;
								if (e_item)
								{
									auto l = (BP::Library*)e_item->get_component(cDataKeeper)->get_voidp_item(FLAME_CHASH("library"));
									std::wstring str;
									auto l_db = l->db();
									for (auto i = 0; i < app.bp->node_count(); i++)
									{
										auto n = app.bp->node(i);
										auto udt = n->udt();
										if (udt && udt->db() == l_db)
											str += L"id: " + s2w(n->id()) + L", type: " + s2w(n->type()) + L"\n";
									}

									utils::e_confirm_dialog((L"The node(s):\n" + str + L"will be removed, sure to remove the library?").c_str(), [](void* c, bool yes) {
										if (yes)
										{
											auto e_item = *(void**)c;
											looper().add_event([](void* c, bool*) {
												auto e_item = *(Entity**)c;
												auto l = (BP::Library*)e_item->get_component(cDataKeeper)->get_voidp_item(FLAME_CHASH("library"));
												app.remove_library(l);
												e_item->parent()->remove_child(e_item);
											}, new_mail_p(e_item));
										}
									}, new_mail_p(e_item));
								}
							}, new_mail_p(e_list));
							utils::e_button(L"Close", [](void*) {
								utils::remove_top_layer(app.root);
							}, Mail<>());
							utils::e_end_layout();
						utils::e_end_dialog();
					}, Mail<>());
				utils::e_end_menubar_menu();
				utils::e_begin_menubar_menu(L"Edit");
					utils::e_menu_item((std::wstring(Icon_UNDO) + L"    Undo").c_str(), [](void* c) {
						undo();
					}, Mail<>());
					utils::e_menu_item((std::wstring(Icon_REPEAT) + L"    Redo").c_str(), [](void* c) {
						redo();
					}, Mail<>());
					utils::e_menu_item((std::wstring(Icon_CLONE) + L"    Duplicate").c_str(), [](void* c) {
						app.duplicate_selected();
					}, Mail<>());
					utils::e_menu_item((std::wstring(Icon_TIMES) + L"    Delete").c_str(), [](void* c) {
						app.delete_selected();
					}, Mail<>());
				utils::e_end_menubar_menu();
				utils::e_begin_menubar_menu(L"View");
					utils::e_menu_item(L"Editor", [](void* c) {
					}, Mail<>());
					utils::e_menu_item(L"Console", [](void* c) {
					}, Mail<>());
				utils::e_end_menubar_menu();
				utils::e_begin_menubar_menu(L"Tools");
					utils::e_menu_item(L"Generate Graph Image", [](void* c) {
						app.generate_graph_image();
					}, Mail<>());
					utils::e_menu_item(L"Auto Set Layout", [](void* c) {
						app.auto_set_layout();
					}, Mail<>());
					utils::e_menu_item(L"Link Test Nodes", [](void* c) {
						app.link_test_nodes();
					}, Mail<>());
					utils::e_menu_item(L"Reflector", [](void* c) {
						utils::e_reflector_window(app.s_event_dispatcher);
					}, Mail<>());
				utils::e_end_menubar_menu();
			utils::e_end_menu_bar();

			{
				auto c_element = utils::e_begin_layout(LayoutHorizontal, 8.f)->get_component(cElement);
				c_element->inner_padding_ = 4.f;
				c_element->color_ = utils::style_4c(utils::FrameColorNormal);
				utils::c_aligner(SizeFitParent, SizeFixed);
				auto c_checkbox = utils::e_checkbox(L"Auto ")->get_component(cCheckbox);
				c_checkbox->data_changed_listeners.add([](void* c, uint hash, void*) {
					if (hash == FLAME_CHASH("checked"))
						app.auto_update = (*(cCheckbox**)c)->checked;
					return true;
				}, new_mail_p(c_checkbox));
				utils::e_button(L"Update", [](void*) {
					app.bp->update();
				}, Mail<>());
				utils::e_button(L"Reset Time");
				utils::e_end_layout();
			}

			utils::e_begin_docker_static_container();
			if (window_layout_ok)
			{
				for (auto n_window : window_layout_root.child("static"))
				{
					std::string name = n_window.name();
					utils::e_begin_docker();
					if (name == "editor")
						editor = new cEditor();
					utils::e_end_docker();
				}
			}
			utils::e_end_docker_static_container();

		utils::e_end_layout();

		if (window_layout_ok)
		{
			for (auto n_window : window_layout_root.child("floating"))
			{

			}
		}

	utils::pop_parent();

	link_test_nodes();

	return true;
}

MyApp app;

int main(int argc, char **args)
{
	if (argc < 2)
		return 0;

	if (!app.create(args[1]))
		return 0;

	looper().add_event([](void*, bool* go_on) {
		if (app.auto_update)
			app.bp->update();
		*go_on = true;
	}, Mail<>(), 0.f);

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
