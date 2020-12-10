#pragma once

#include "../entity_private.h"
#include <flame/universe/components/menu.h>

namespace flame
{
	struct EntityPrivate;
	struct cElementPrivate;
	struct cReceiverPrivate;

	struct cMenuPrivate : cMenu
	{
		cElementPrivate* element = nullptr;
		cReceiverPrivate* receiver = nullptr;

		void* mouse_down_listener = nullptr;
		void* mouse_move_listener = nullptr;
		void* root_mouse_listener = nullptr;
		
		MenuType type = MenuTop;
		EntityPrivate* items = nullptr;
		EntityPrivate* root = nullptr;
		cReceiverPrivate* root_receiver = nullptr;
		bool opened = false;
		int frame = -1;

		MenuType get_type() const override { return type; }
		void set_type(MenuType t) override { type = t; }

		void open();
		void close();

		void on_gain_receiver();
		void on_lost_receiver();
	};
}
