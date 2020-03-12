#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/ui/utils.h>

#include "app.h"

const auto dot_path = s2w(GRAPHVIZ_PATH) + L"/bin/dot.exe";

void MyApp::set_changed(bool v)
{
	if (changed != v)
	{
		changed = v;
		if (bp_editor)
			bp_editor->on_changed();
	}
}

void MyApp::deselect()
{
	if (bp_editor)
		bp_editor->on_deselect();

	sel_type = SelAir;
	selected.plain = nullptr;
}

void MyApp::select(SelType t, void* p)
{
	deselect();
	sel_type = t;
	selected.plain = p;

	if (bp_editor)
		bp_editor->on_select();
}

BP::Library* MyApp::add_library(const wchar_t* filename)
{
	auto l = bp->add_library(filename);
	if (l)
	{
		if (bp_editor)
			bp_editor->on_add_library(l);
		refresh_add_node_menu();
		set_changed(true);
	}
	return l;
}

BP::Node* MyApp::add_node(const char* type_name, const char* id)
{
	auto n = bp->add_node(type_name, id);
	if (bp_editor)
		bp_editor->on_add_node(n);
	set_changed(true);
	return n;
}

BP::SubGraph* MyApp::add_subgraph(const wchar_t* filename, const char* id)
{
	auto s = bp->add_subgraph(filename, id);
	if (s)
	{
		if (app.bp_editor)
			app.bp_editor->on_add_subgraph(s);
		set_changed(true);
	}
	return s;
}

void MyApp::remove_library(BP::Library* l)
{
	looper().add_event([](void* c, bool*) {
		auto l = *(BP::Library**)c;
		if (app.bp_editor)
			app.bp_editor->on_remove_library(l);
		app.bp->remove_library(l);
		app.refresh_add_node_menu();
		app.set_changed(true);
	}, new_mail_p(l));
}

bool MyApp::remove_node(BP::Node* n)
{
	if (n->id() == "test_dst" || n->id() == "test_cbs")
		return false;
	looper().add_event([](void* c, bool*) {
		auto n = *(BP::Node**)c;
		if (app.bp_editor)
			app.bp_editor->on_remove_node(n);
		n->scene()->remove_node(n);
	}, new_mail_p(n));
	set_changed(true);
	return true;
}

void MyApp::remove_subgraph(BP::SubGraph* s)
{
	looper().add_event([](void* c, bool*) {
		auto s = *(BP::SubGraph**)c;
		if (app.bp_editor)
			app.bp_editor->on_remove_subgraph(s);
		app.bp->remove_subgraph(s);
		app.set_changed(true);
	}, new_mail_p(s));
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
	case SelLibrary:
	{
		std::wstring str;
		auto l_db = selected.library->db();
		for (auto i = 0; i < bp->node_count(); i++)
		{
			auto n = bp->node(i);
			auto udt = n->udt();
			if (udt && udt->db() == l_db)
				str += L"id: " + s2w(n->id()) + L", type: " + s2w(n->type_name()) + L"\n";
		}

		ui::e_confirm_dialog((L"The node(s):\n" + str + L"will be removed, sure to remove the library?").c_str(), [](void* c, bool yes) {
			auto l = *(BP::Library**)c;
			if (yes)
				app.remove_library(l);
		}, new_mail_p(selected.library));
	}
	break;
	case SelSubGraph:
		break;
	case SelNode:
		if (!remove_node(selected.node))
			ui::e_message_dialog(L"Cannot Remove Test Nodes");
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

	for (auto i = 1; i < e_add_node_menu->child_count(); i++)
		e_add_node_menu->child(i)->set_visibility(true);
}

void MyApp::refresh_add_node_menu()
{
	e_add_node_menu->remove_children(0, -1);

	std::vector<UdtInfo*> all_udts;
	auto add_udt = [&](UdtInfo* u) {
		{
			auto f = only_not_null(u->find_function("update"), u->find_function("active_update"));
			if (!f.first)
				return;
			if (!check_function(f.first, "D#void", {}))
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

	ui::push_parent(e_add_node_menu);
		ui::e_begin_layout(LayoutHorizontal, 4.f);
			ui::e_text(Icon_SEARCH);
			ui::e_edit(150.f)->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
				auto menu = *(Entity**)c;
				auto str = ((cText*)t)->text();
				for (auto i = 1; i < menu->child_count(); i++)
				{
					auto item = menu->child(i);
					item->set_visibility(str[0] ? (std::wstring(item->get_component(cText)->text()).find(str) != std::string::npos) : true);
				}
				return true;
			}, new_mail_p(e_add_node_menu));
			add_node_menu_filter = ui::current_entity()->get_component(cEdit);
		ui::e_end_layout();
		ui::e_menu_item(L"Enum", [](void* c) {
		}, new_mail_p(this));
		ui::e_menu_item(L"Var", [](void* c) {
		}, new_mail_p(this));
		ui::e_menu_item(L"Array", [](void* c) {
		}, new_mail_p(this));
		for (auto udt : all_udts)
		{
			ui::e_menu_item(s2w(udt->type()->name()).c_str(), [](void* c) {
				app.reset_add_node_menu_filter();
				if (app.bp_editor)
					app.bp_editor->set_add_pos_center();
				app.add_node((*(UdtInfo**)c)->type()->name(), "");
			}, new_mail_p(udt));
		}
	ui::pop_parent();
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
				ui::e_begin_menubar_menu(L"Blueprint");
					ui::e_menu_item(L"Save", [](void* c) {
						BP::save_to_file(app.bp, app.filepath.c_str());
						app.set_changed(false);
					}, Mail<>());
				ui::e_end_menubar_menu();
				ui::e_begin_menubar_menu(L"Add");
					ui::e_menu_item(L"Library", [](void* c) {
						ui::e_input_dialog(L"library", [](void* c, bool ok, const wchar_t* text) {
							if (ok && text[0])
							{
								if (app.bp_editor)
									app.bp_editor->set_add_pos_center();
								auto l = app.add_library(text);
								if (!l)
									ui::e_message_dialog(L"Add Library Failed");
							}
						}, Mail<>());
					}, Mail<>());
				app.e_add_node_menu = ui::e_begin_sub_menu(L"Nodes")->get_component(cMenu)->items;
				ui::e_end_sub_menu();
				ui::e_menu_item(L"Sub Graph", [](void* c) {
					ui::e_input_dialog(L"bp", [](void* c, bool ok, const wchar_t* text) {
						if (ok && text[0])
						{
							if (app.bp_editor)
								app.bp_editor->set_add_pos_center();
							auto s = app.add_subgraph(text, "");
							if (!s)
								ui::e_message_dialog(L"Add Sub Graph Failed");
						}
					}, Mail<>());
				}, Mail<>());
				ui::e_end_menubar_menu();
				ui::e_begin_menubar_menu(L"Edit");
				ui::e_menu_item(L"Duplicate", [](void* c) {
					if (app.bp_editor)
						app.bp_editor->set_add_pos_center();
					app.duplicate_selected();
				}, Mail<>());
				ui::e_menu_item(L"Delete", [](void* c) {
					app.delete_selected();
				}, Mail<>());
				ui::e_end_menubar_menu();
				ui::e_begin_menubar_menu(L"Tools");
				ui::e_menu_item(L"Generate Graph Image", [](void* c) {
					app.generate_graph_image();
				}, Mail<>());
				ui::e_menu_item(L"Auto Set Layout", [](void* c) {
					app.auto_set_layout();
				}, Mail<>());
				ui::e_menu_item(L"Link Test Nodes", [](void* c) {
					app.link_test_nodes();
				}, Mail<>());
				ui::e_end_menubar_menu();
				ui::e_begin_menubar_menu(L"Window");
				ui::e_menu_item(L"BP Editor", [](void* c) {
				}, Mail<>());
				ui::e_menu_item(L"Console", [](void* c) {
				}, Mail<>());
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
