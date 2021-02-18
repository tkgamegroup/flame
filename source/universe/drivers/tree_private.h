#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/tree.h>

namespace flame
{
	struct cTextPrivate;
	struct cReceiverPrivate;

	struct dTreePrivate : dTree
	{
		EntityPrivate* selected = nullptr;

		Entity* get_selected() const override { return selected; }
		void set_selected(Entity* e) override;

		void expand_to_selected() override;

		void on_load_finished() override;
		bool on_child_added(Entity* e) override;
	};

	struct dTreeLeafPrivate : dTreeLeaf
	{
		dTreePrivate* tree;
		cTextPrivate* title_text;

		std::wstring title;

		const wchar_t* get_title() const override { return title.c_str(); };
		void set_title(const wchar_t* title) override;

		void on_load_finished() override;
	};

	struct dTreeNodePrivate : dTreeNode
	{
		dTreePrivate* tree;
		cTextPrivate* title_text;
		cTextPrivate* arrow_text;
		EntityPrivate* items;

		std::wstring title;

		const wchar_t* get_title() const override { return title.c_str(); };
		void set_title(const wchar_t* title) override;

		void toggle_collapse() override;

		void on_load_finished() override;
		bool on_child_added(Entity* e) override;
	};
}
