#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/menu.h>

namespace flame
{
	struct cElementPrivate;
	struct cReceiverPrivate;

	struct dMenuPrivate : dMenu
	{
		cElementPrivate* element;
		cReceiverPrivate* receiver;

		MenuType type = MenuTop;
		EntityPrivate* items;
		EntityPrivate* root;
		bool opened = false;

		void on_load_finished() override;
		bool on_child_added(Entity* e) override;

		void open();
		void close();
	};
}
