#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/tree.h>

namespace flame
{
	struct cTextPrivate;
	struct cReceiverPrivate;

	struct cTreePrivate : dTree
	{
		EntityPrivate* selected = nullptr;

		cReceiverPrivate* receiver = nullptr;

		void* mouse_listener = nullptr;

		void on_gain_receiver();
		void on_lost_receiver();

		Entity* get_selected() const override { return selected; }
		void set_selected(Entity* e) override;

		void expand_to_selected() override;
	};

	struct cTreeLeafPrivate : cTreeLeaf
	{
		cTreePrivate* tree = nullptr;
		cReceiverPrivate* receiver = nullptr;

		void* mouse_listener = nullptr;

		void on_gain_receiver();
		void on_lost_receiver();
	};

	struct cTreeNodePrivate : cTreeNode
	{
		cTreePrivate* tree = nullptr;
		cReceiverPrivate* receiver = nullptr;
		cTextPrivate* arrow_text = nullptr;
		cReceiverPrivate* arrow_receiver = nullptr;
		cElementPrivate* items_element = nullptr;

		void* mouse_listener = nullptr;
		void* arrow_mouse_listener = nullptr;

		void toggle_collapse() override;

		void on_gain_receiver();
		void on_lost_receiver();
		void on_gain_arrow_receiver();
		void on_lost_arrow_receiver();
	};
}
