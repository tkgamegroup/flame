#pragma once

#include "../entity_private.h"
#include <flame/universe/components/list.h>

namespace flame
{
	struct cReceiverPrivate;

	struct cListPrivate : cList
	{
		cReceiverPrivate* receiver;

		void* mouse_listener = nullptr;

		EntityPrivate* selected = nullptr;
		bool select_air_when_clicked = true;

		void on_gain_receiver();
		void on_lost_receiver();

		Entity* get_selected() const override { return selected; }
		void set_selected(Entity* e) override;
	};

	struct cListItemPrivate : cListItem
	{
		cListPrivate* list;
		cReceiverPrivate* receiver;

		void* mouse_listener = nullptr;

		void on_gain_receiver();
		void on_lost_receiver();
	};
}
