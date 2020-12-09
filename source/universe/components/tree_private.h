#pragma once

#include "../entity_private.h"
#include <flame/universe/components/tree.h>

namespace flame
{
	struct cTextPrivate;
	struct cEventReceiverPrivate;

	struct cTreePrivate : cTree
	{
		EntityPrivate* selected = nullptr;

		cEventReceiverPrivate* event_receiver = nullptr;

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();

		Entity* get_selected() const override { return selected; }
		void set_selected(Entity* e) override;

		void expand_to_selected() override;
	};

	struct cTreeLeafPrivate : cTreeLeaf
	{
		cTreePrivate* tree = nullptr;
		cEventReceiverPrivate* event_receiver = nullptr;

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};

	struct cTreeNodePrivate : cTreeNode
	{
		cTreePrivate* tree = nullptr;
		cEventReceiverPrivate* event_receiver = nullptr;
		cTextPrivate* arrow_text = nullptr;
		cEventReceiverPrivate* arrow_event_receiver = nullptr;
		cElementPrivate* items_element = nullptr;

		void* mouse_listener = nullptr;
		void* arrow_mouse_listener = nullptr;

		void toggle_collapse() override;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
		void on_gain_arrow_event_receiver();
		void on_lost_arrow_event_receiver();
	};
}
