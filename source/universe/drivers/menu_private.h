#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/menu.h>

namespace flame
{
	struct cElementPrivate;
	struct cReceiverPrivate;
	struct cTextPrivate;

	struct dMenuPrivate : dMenu
	{
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		cTextPrivate* text;

		MenuType type = MenuTop;
		EntityPrivate* items;
		EntityPrivate* root;
		void* root_mouse_listener = nullptr;
		bool first = true;
		bool opened = false;

		void on_load_finished() override;
		bool on_child_added(Entity* e) override;

		void open();
		void close();
	};
}
