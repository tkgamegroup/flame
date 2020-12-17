#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/list.h>

namespace flame
{
	struct cReceiverPrivate;

	struct dListPrivate : dList
	{
		cReceiverPrivate* receiver;

		EntityPrivate* selected = nullptr;
		bool enable_deselect = true;

		void on_load_finished() override;
		bool on_child_added(Entity* _e) override;

		Entity* get_selected() const override { return selected; }
		void set_selected(Entity* e) override;
	};

	struct dListItemPrivate : dListItem
	{
		dListPrivate* list;
		cReceiverPrivate* receiver;

		void on_load_finished() override;
	};
}
