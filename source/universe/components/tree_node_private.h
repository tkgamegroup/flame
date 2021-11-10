#pragma once

#include "tree_node.h"

namespace flame
{
	struct cTreeNodePrivate : cTreeNode
	{
		std::wstring title;

		cTreePrivate* tree;
		cElementPrivate* element;
		EntityPrivate* e_title;
		cTextPrivate* title_text;
		cTextPrivate* arrow_text;
		EntityPrivate* items;
		cElementPrivate* items_element;

		bool first_add = true;

		const wchar_t* get_title() const override { return title.c_str(); }
		void set_title(const wchar_t* v) override { set_title(std::wstring(v)); }
		void set_title(std::wstring_view v);

		void notify_selected(bool v);

		void toggle_collapse() override;

		void on_load_finished() override;
		bool on_before_adding_child(EntityPtr e);
	};
}
