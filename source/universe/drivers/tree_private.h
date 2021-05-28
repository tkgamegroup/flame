#pragma once

#include "../entity_private.h"
#include "tree.h"

namespace flame
{
	struct dTreePrivate : dTree
	{
		EntityPrivate* selected = nullptr;

		EntityPtr get_selected() const override { return selected; }
		void set_selected(EntityPtr e) override;

		void expand_to_selected() override;

		void on_load_finished() override;
		bool on_child_added(EntityPtr e) override;
	};

	struct dTreeLeafPrivate : dTreeLeaf
	{
		dTreePrivate* tree;
		cTextPrivate* title_text;

		std::wstring title;

		const wchar_t* get_title() const override { return title.c_str(); };
		void set_title(const wchar_t* title) override;

		void notify_selected(bool v);

		void on_load_finished() override;
	};

	struct dTreeNodePrivate : dTreeNode
	{
		dTreePrivate* tree;
		cElementPrivate* element;
		EntityPrivate* e_title;
		cTextPrivate* title_text;
		cTextPrivate* arrow_text;
		EntityPrivate* items;
		cElementPrivate* items_element;

		std::wstring title;

		bool first_add = true;

		const wchar_t* get_title() const override { return title.c_str(); };
		void set_title(const wchar_t* title) override;

		void notify_selected(bool v);

		void toggle_collapse() override;

		void on_load_finished() override;
		bool on_child_added(EntityPtr e) override;
	};
}
