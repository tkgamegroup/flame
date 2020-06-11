#include "bp_editor.h"

cDetail::cDetail() :
	Component("cDetail")
{
	auto& ui = bp_editor.window->ui;

	e_page = ui.e_begin_docker_page(L"Detail").second;
	{
		ui.c_layout(LayoutVertical)->item_padding = 4.f;

		e_page->add_component(this);
	}

	ui.e_end_docker_page();

	on_after_select();
}

cDetail::~cDetail()
{
	bp_editor.detail = nullptr;
}

void cDetail::on_after_select()
{
	looper().add_event([](Capture&) {
		auto& ui = bp_editor.window->ui;

		bp_editor.detail->e_page->remove_children(0, -1);
		ui.parents.push(bp_editor.detail->e_page);
		if (bp_editor.selected_nodes.empty() && bp_editor.selected_links.empty())
			ui.e_text(L"nothing selected");
		else
		{
			if (!bp_editor.selected_nodes.empty())
			{
				if (bp_editor.selected_nodes.size() == 1)
				{
					auto n = bp_editor.selected_nodes[0];
					std::wstring str;
					std::string n_type_parameters;
					auto n_type = bp_break_node_type(n->type.str(), &n_type_parameters);
					auto n_name = n_type ? s2w(n_type_parameters) : s2w(n->type.str());
					auto udt = n->udt;
					if (udt)
						str = L"UDT (" + udt->db->library_name.str() + L")\n" + n_name;
					else
						str = node_type_prefix(n_type) + n_name;
					ui.e_text(str.c_str())->get_component(cText)->color = node_type_color(n_type);
					ui.e_begin_layout(LayoutHorizontal, 4.f);
						ui.e_text((L"ID: " + s2w(n->id.str())).c_str());
						ui.e_button(L"change", [](Capture& c) {
							auto& ui = bp_editor.window->ui;
							auto n = c.thiz<bpNode>();
							ui.e_input_dialog(L"ID", [](Capture& c, bool ok, const wchar_t* text) {
								if (ok && text[0])
									bp_editor.set_node_id(c.thiz<bpNode>(), w2s(text));
							}, Capture().set_thiz(n), s2w(n->id.str()).c_str());
						}, Capture().set_thiz(n));
					ui.e_end_layout();
				}
				else
					ui.e_text(wfmt(L"%d nodes selected", (int)bp_editor.selected_nodes.size()).c_str());
			}
			if (!bp_editor.selected_links.empty())
			{
				if (bp_editor.selected_links.size() == 1)
				{
					auto l = bp_editor.selected_links[0];
					ui.e_text(wfmt(L"%s -> %s", s2w(l->links[0]->get_address().v).c_str(), s2w(l->get_address().v).c_str()).c_str());
				}
				else
					ui.e_text(wfmt(L"%d links selected", (int)bp_editor.selected_links.size()).c_str());
			}
		}
		ui.parents.pop();
	}, Capture());
}
