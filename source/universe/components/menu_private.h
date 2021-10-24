#pragma once

#include "menu.h"

namespace flame
{
	struct cMenuPrivate : cMenu
	{
		MenuType type = MenuTop;
		EntityPrivate* items;
		EntityPrivate* root;
		cReceiverPrivate* root_receiver;
		bool ancestor_opened = false;
		bool opened = false;
		
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		cTextPrivate* text;
		EntityPrivate* arrow;

		void on_entered_world() override;
		void on_load_finished() override;
		bool on_before_adding_child(EntityPtr e) override;
		
		void open();
		void close();
		
		void mark_ancestor_opened(bool v);
	};
}
