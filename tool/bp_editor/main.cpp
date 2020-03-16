#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>

#include "app.h"

const auto dot_path = s2w(GRAPHVIZ_PATH) + L"/bin/dot.exe";

void MyApp::set_changed(bool v)
{
	if (changed != v)
	{
		changed = v;
		if (editor)
			editor->on_changed();
	}
}

void MyApp::deselect()
{
	if (editor)
		editor->on_deselect();

	sel_type = SelAir;
	selected.plain = nullptr;
}

void MyApp::select(SelType t, void* p)
{
	deselect();
	sel_type = t;
	selected.plain = p;

	if (editor)
		editor->on_select();
}

BP::Library* MyApp::add_library(const wchar_t* filename)
{
	auto l = bp->add_library(filename);
	if (l)
	{
		refresh_add_node_menu();
		set_changed(true);
	}
	return l;
}

BP::Node* MyApp::add_node(const char* type_name, const char* id)
{
	auto n = bp->add_node(type_name, id);
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
		app.refresh_add_node_menu();
		app.set_changed(true);
	}, new_mail_p(l));
}

bool MyApp::remove_node(BP::Node* n)
{
	if (SUS::starts_with(n->id(), "test_"))
		return false;
	looper().add_event([](void* c, bool*) {
		auto n = *(BP::Node**)c;
		if (app.editor)
			app.editor->on_remove_node(n);
		n->scene()->remove_node(n);
	}, new_mail_p(n));
	set_changed(true);
	return true;
}

void MyApp::duplicate_selected()
{
	switch (sel_type)
	{
	case SelNode:
	{
		auto n = add_node(selected.node->type_name(), "");
		for (auto i = 0; i < n->input_count(); i++)
		{
			auto src = selected.node->input(i);
			auto dst = n->input(i);
			dst->set_data(src->data());
			dst->link_to(src->link(0));
		}
	}
	break;
	}
}

void MyApp::delete_selected()
{
	switch (sel_type)
	{
	case SelNode:
		if (!remove_node(selected.node))
			utils::e_message_dialog(L"Cannot Remove Test Nodes");
		break;
	case SelLink:
		selected.link->link_to(nullptr);
		set_changed(true);
		break;
	}

	deselect();
}

void MyApp::reset_add_node_menu_filter()
{
	add_node_menu_filter->text->set_text(L"");

	for (auto i = 1; i < e_node_menu_items->child_count(); i++)
		e_node_menu_items->child(i)->set_visibility(true);
}

void MyApp::refresh_add_node_menu()
{
	e_node_menu_items->remove_children(0, -1);

	std::vector<UdtInfo*> all_udts;
	auto add_udt = [&](UdtInfo* u) {
		{
			auto f = find_not_null_and_only(u->find_function("update"), u->find_function("active_update"));
			if (!f.first)
				return;
			if (!check_function(f.first, "D#void", { "D#uint" }))
				return;
		}
		auto no_input_output = true;
		for (auto i = 0; i < u->variable_count(); i++)
		{
			auto v = u->variable(i);
			auto flags = v->flags();
			if ((flags & VariableFlagInput) || (flags & VariableFlagOutput))
			{
				no_input_output = false;
				break;
			}
		}
		if (!no_input_output)
			all_udts.push_back(u);
	};
	for (auto i = 0; i < global_db_count(); i++)
	{
		auto udts = global_db(i)->get_udts();
		for (auto j = 0; j < udts.s; j++)
			add_udt(udts.v[j]);
	}
	for (auto i = 0; i < app.bp->db_count(); i++)
	{
		auto udts = app.bp->dbs()[i]->get_udts();
		for (auto j = 0; j < udts.s; j++)
			add_udt(udts.v[j]);
	}
	std::sort(all_udts.begin(), all_udts.end(), [](UdtInfo* a, UdtInfo* b) {
		return std::string(a->type()->name()) < std::string(b->type()->name());
	});

	utils::push_parent(e_node_menu_items);
		utils::e_begin_layout(LayoutHorizontal, 4.f);
			utils::e_text(Icon_SEARCH);
			{
				auto c_text = utils::e_edit(150.f)->get_component(cText);
				c_text->data_changed_listeners.add([](void* c, uint hash, void*) {
					auto menu = app.e_node_menu_items;
					auto str = (*(cText**)c)->text();
					for (auto i = 1; i < menu->child_count(); i++)
					{
						auto item = menu->child(i);
						item->set_visibility(str[0] ? (std::wstring(item->get_component(cText)->text()).find(str) != std::string::npos) : true);
					}
					return true;
				}, new_mail_p(c_text));
			}
			add_node_menu_filter = utils::current_entity()->get_component(cEdit);
		utils::e_end_layout();
		utils::e_menu_item(L"Enum", [](void* c) {
		}, new_mail_p(this));
		utils::e_menu_item(L"Var", [](void* c) {
		}, new_mail_p(this));
		utils::e_menu_item(L"Array", [](void* c) {
		}, new_mail_p(this));
		for (auto udt : all_udts)
		{
			utils::e_menu_item(s2w(udt->type()->name()).c_str(), [](void* c) {
				app.reset_add_node_menu_filter();
				if (app.editor)
					app.editor->set_add_pos_center();
				app.add_node((*(UdtInfo**)c)->type()->name(), "");
			}, new_mail_p(udt));
		}
	utils::pop_parent();
}

void MyApp::link_test_nodes()
{
	auto n_dst = app.bp->find_node("test_dst");
	if (!n_dst)
	{
		n_dst = app.add_node("D#DstImage", "test_dst");
		n_dst->pos = Vec2f(0.f, -200.f);
		n_dst->used_by_editor = true;
	}
	{
		auto s = app.bp->find_input("*.rt_dst.type");
		if (s)
			s->link_to(n_dst->find_output("type"));
	}
	{
		auto s = app.bp->find_input("*.rt_dst.v");
		if (s)
			s->link_to(n_dst->find_output("view"));
	}
	auto n_cbs = app.bp->find_node("test_cbs");
	if (!n_cbs)
	{
		n_cbs = app.add_node("D#CmdBufs", "test_cbs");
		n_dst->pos = Vec2f(2000.f, -200.f);
		n_cbs->used_by_editor = true;
	}
	{
		auto s = app.bp->find_input("*.make_cmd.cbs");
		if (s)
			s->link_to(n_cbs->find_output("out"));
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
			auto str = sfmt("\t%s[label = \"%s|%s|{{", name, name, n->type_name());
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
	utils::c_layout();

	utils::push_font_atlas(app.font_atlas_pixel);
	utils::set_current_root(root);
	utils::push_parent(root);

		utils::e_begin_layout(LayoutVertical, 0.f, false, false);
		utils::c_aligner(SizeFitParent, SizeFitParent);

			utils::e_begin_menu_bar();
				utils::e_begin_menubar_menu(L"Blueprint");
					utils::e_menu_item(L"Save", [](void* c) {
						BP::save_to_file(app.bp, app.filepath.c_str());
						app.set_changed(false);
					}, Mail<>());
					utils::e_menu_item(L"Libraries", [](void* c) {
						utils::e_begin_dialog();
							utils::e_text(L"Libraries");
							utils::e_begin_scroll_view1(ScrollbarVertical, Vec2f(200.f, 100.f), 4.f, 2.f);
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
										if (app.editor)
											app.editor->set_add_pos_center();
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
											str += L"id: " + s2w(n->id()) + L", type: " + s2w(n->type_name()) + L"\n";
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
				app.e_node_menu_items = utils::e_begin_menubar_menu(L"Add")->get_component(cMenu)->items;
				utils::e_end_menubar_menu();
				utils::e_begin_menubar_menu(L"Edit");
					utils::e_menu_item(L"Duplicate", [](void* c) {
						if (app.editor)
							app.editor->set_add_pos_center();
						app.duplicate_selected();
					}, Mail<>());
					utils::e_menu_item(L"Delete", [](void* c) {
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

	refresh_add_node_menu();

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
		if (app.running)
			app.bp->update();
		*go_on = true;
	}, Mail<>(), 0.f);

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
