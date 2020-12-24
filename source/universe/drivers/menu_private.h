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
		EntityPrivate* arrow;

		MenuType type = MenuTop;
		EntityPrivate* items;
		EntityPrivate* root;
		bool ancestor_opened = false;
		bool opened = false;

		void on_load_finished() override;
		bool on_child_added(Entity* e) override;

		void open();
		void close();

		void mark_ancestor_opened(bool v);
	};

	struct dMenuItemPrivate : dMenuItem
	{
		cReceiverPrivate* receiver;

		void on_load_finished() override;
	};

	struct dMenuBarPrivate : dMenuBar
	{

	};
}
