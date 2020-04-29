#include "bp_editor.h"

cDetail::cDetail() :
	Component("cDetail")
{
	e_page = utils::e_begin_docker_page(L"Detail").second;
	{
		utils::c_layout(LayoutVertical)->item_padding = 4.f;

		e_page->add_component(this);
	}

	utils::e_end_docker_page();

	on_after_select();
}

cDetail::~cDetail()
{
	bp_editor.detail = nullptr;
}

void cDetail::on_after_select()
{
	looper().add_event([](Capture&) {
		bp_editor.detail->e_page->remove_children(0, -1);
		utils::push_parent(bp_editor.detail->e_page);
		if (bp_editor.selected_nodes.empty() && bp_editor.selected_links.empty())
			utils::e_text(L"nothing selected");
		else
		{
			if (!bp_editor.selected_nodes.empty())
			{
				if (bp_editor.selected_nodes.size() == 1)
				{
					auto n = bp_editor.selected_nodes[0];
					std::wstring str;
					std::string n_type_parameters;
					auto n_type = BP::break_node_type(n->type(), &n_type_parameters);
					auto n_name = n_type ? s2w(n_type_parameters) : s2w(n->type());
					auto udt = n->udt();
					if (udt)
						str = L"UDT (" + std::wstring(udt->db()->module_name()) + L")\n" + n_name;
					else
						str = node_type_prefix(n_type) + n_name;
					utils::e_text(str.c_str())->get_component(cText)->color = node_type_color(n_type);
					utils::e_begin_layout(LayoutHorizontal, 4.f);
						utils::e_text((L"ID: " + s2w(n->id())).c_str());
						utils::e_button(L"change", [](Capture& c) {
							auto n = c.thiz<BP::Node>();
							utils::e_input_dialog(L"ID", [](Capture& c, bool ok, const wchar_t* text) {
								if (ok && text[0])
									bp_editor.set_node_id(c.thiz<BP::Node>(), w2s(text));
							}, Capture().set_thiz(n), s2w(n->id()).c_str());
						}, Capture().set_thiz(n));
					utils::e_end_layout();
				}
				else
					utils::e_text(wfmt(L"%d nodes selected", (int)bp_editor.selected_nodes.size()).c_str());
			}
			if (!bp_editor.selected_links.empty())
			{
				if (bp_editor.selected_links.size() == 1)
				{
					auto l = bp_editor.selected_links[0];
					utils::e_text(wfmt(L"%s -> %s", s2w(l->link()->get_address().v).c_str(), s2w(l->get_address().v).c_str()).c_str());
				}
				else
					utils::e_text(wfmt(L"%d links selected", (int)bp_editor.selected_links.size()).c_str());
			}
		}
		utils::pop_parent();
	}, Capture());
}
