#pragma once

#include "tree.h"

namespace flame
{
	struct cTreePrivate : cTree
	{
		EntityPtr selected = nullptr;

		EntityPtr get_selected() const override { return selected; }
		void set_selected(EntityPtr v) override;

		void expand_to_selected() override;

		void on_load_finished() override;
		bool on_before_adding_child(EntityPtr e) override;
	};

	void populate_tree(cTreePrivate* t, EntityPrivate* e);
}
